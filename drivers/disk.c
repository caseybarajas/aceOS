#include "../include/disk.h"
#include "../include/io.h"
#include "../include/serial.h"
#include "../include/libc/string.h"
#include "../include/timer.h"
#include "../include/utils.h"

// Maximum number of drives supported
#define MAX_DRIVES 4

// Global disk information
static disk_info_t drives[MAX_DRIVES];
static int drives_detected = 0;

// Initialize disk subsystem
void disk_init(void) {
    serial_write_string("Initializing disk subsystem...\n");
    
    // Clear drive information
    memset(drives, 0, sizeof(drives));
    
    // Detect available drives
    drives_detected = disk_detect_drives();
    
    char buffer[16];
    serial_write_string("Disk subsystem initialized, ");
    itoa(drives_detected, buffer, 10);
    serial_write_string(buffer);
    serial_write_string(" drives detected\n");
}

// Detect available disk drives
int disk_detect_drives(void) {
    int count = 0;
    
    // Check primary ATA drives
    for (int i = 0; i < 2; i++) {
        if (ata_identify_drive(i, &drives[count])) {
            drives[count].drive_number = i;
            drives[count].disk_type = DISK_TYPE_ATA;
            drives[count].present = 1;
            count++;
        }
    }
    
    // Check secondary ATA drives
    for (int i = 2; i < 4; i++) {
        if (ata_identify_drive(i, &drives[count])) {
            drives[count].drive_number = i;
            drives[count].disk_type = DISK_TYPE_ATA;
            drives[count].present = 1;
            count++;
        }
    }
    
    // TODO: Add floppy disk detection
    
    return count;
}

// Get disk information for a specific drive
disk_info_t* disk_get_info(uint8_t drive) {
    if (drive >= MAX_DRIVES) {
        return NULL;
    }
    
    for (int i = 0; i < drives_detected; i++) {
        if (drives[i].drive_number == drive && drives[i].present) {
            return &drives[i];
        }
    }
    
    return NULL;
}

// Generic disk read function
int disk_read_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer) {
    disk_info_t* info = disk_get_info(drive);
    
    if (!info) {
        serial_write_string("ERROR: Invalid drive number\n");
        return -1;
    }
    
    switch (info->disk_type) {
        case DISK_TYPE_ATA:
            return ata_read_sectors(drive, lba, count, buffer);
        case DISK_TYPE_FLOPPY:
            return floppy_read_sectors(drive, lba, count, buffer);
        default:
            serial_write_string("ERROR: Unknown disk type\n");
            return -1;
    }
}

// Generic disk write function
int disk_write_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer) {
    disk_info_t* info = disk_get_info(drive);
    
    if (!info) {
        serial_write_string("ERROR: Invalid drive number\n");
        return -1;
    }
    
    switch (info->disk_type) {
        case DISK_TYPE_ATA:
            return ata_write_sectors(drive, lba, count, buffer);
        case DISK_TYPE_FLOPPY:
            return floppy_write_sectors(drive, lba, count, buffer);
        default:
            serial_write_string("ERROR: Unknown disk type\n");
            return -1;
    }
}

// ATA/IDE functions
int ata_identify_drive(uint8_t drive, disk_info_t* info) {
    uint16_t base = (drive < 2) ? ATA_PRIMARY_BASE : ATA_SECONDARY_BASE;
    uint8_t drive_select = (drive % 2) ? 0xB0 : 0xA0;
    
    // Select drive
    outb(base + ATA_REG_DRIVE_HEAD, drive_select);
    
    // Small delay
    for (int i = 0; i < 4; i++) {
        inb(base + ATA_REG_STATUS);
    }
    
    // Send IDENTIFY command
    outb(base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    
    // Check if drive exists
    uint8_t status = inb(base + ATA_REG_STATUS);
    if (status == 0) {
        return 0; // No drive
    }
    
    // Wait for BSY to clear with timeout
    {
        uint32_t timeout = 1000000; // arbitrary loop count timeout
        while ((inb(base + ATA_REG_STATUS) & ATA_STATUS_BSY) && --timeout) {}
        if (timeout == 0) {
            serial_write_string("ATA IDENTIFY timeout waiting for BSY clear\n");
            return 0;
        }
    }
    
    // Check for errors
    status = inb(base + ATA_REG_STATUS);
    if (status & ATA_STATUS_ERR) {
        return 0; // Error occurred
    }
    
    // Wait for DRQ with timeout
    {
        uint32_t timeout = 1000000;
        while (!(inb(base + ATA_REG_STATUS) & ATA_STATUS_DRQ) && --timeout) {}
        if (timeout == 0) {
            serial_write_string("ATA IDENTIFY timeout waiting for DRQ\n");
            return 0;
        }
    }
    
    // Read identification data
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(base + ATA_REG_DATA);
    }
    
    // Extract information from identify data
    if (info) {
        // Model string (words 27-46)
        for (int i = 0; i < 20; i++) {
            uint16_t word = identify_data[27 + i];
            info->model[i * 2] = (word >> 8) & 0xFF;
            info->model[i * 2 + 1] = word & 0xFF;
        }
        info->model[40] = '\0';
        
        // Serial number (words 10-19)
        for (int i = 0; i < 10; i++) {
            uint16_t word = identify_data[10 + i];
            info->serial[i * 2] = (word >> 8) & 0xFF;
            info->serial[i * 2 + 1] = word & 0xFF;
        }
        info->serial[20] = '\0';
        
        // Total sectors (words 60-61 for 28-bit LBA)
        info->total_sectors = (uint32_t)identify_data[60] | 
                             ((uint32_t)identify_data[61] << 16);
        
        info->sector_size = 512; // Standard sector size
        
        // Geometry (approximated for modern drives)
        info->geometry.sector_size = 512;
        info->geometry.total_sectors = info->total_sectors;
        info->geometry.sectors_per_track = 63;
        info->geometry.heads = 16;
        info->geometry.cylinders = info->total_sectors / (63 * 16);
    }
    
    return 1; // Success
}

int ata_read_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer) {
    uint16_t base = (drive < 2) ? ATA_PRIMARY_BASE : ATA_SECONDARY_BASE;
    uint8_t drive_select = ((drive % 2) ? 0xF0 : 0xE0) | ((lba >> 24) & 0x0F);
    
    // Wait for drive to be ready with timeout
    {
        uint32_t timeout = 1000000;
        while (!(inb(base + ATA_REG_STATUS) & ATA_STATUS_RDY) && --timeout) {}
        if (timeout == 0) {
            serial_write_string("ATA READ timeout waiting for RDY\n");
            return -1;
        }
    }
    
    // Set up LBA and sector count
    outb(base + ATA_REG_DRIVE_HEAD, drive_select);
    outb(base + ATA_REG_SECTOR_COUNT, count);
    outb(base + ATA_REG_LBA_LOW, lba & 0xFF);
    outb(base + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);
    outb(base + ATA_REG_LBA_HIGH, (lba >> 16) & 0xFF);
    
    // Send read command
    outb(base + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);
    
    uint16_t* buf = (uint16_t*)buffer;
    
    for (int sector = 0; sector < count; sector++) {
        // Wait for data with timeout
        {
            uint32_t timeout = 1000000;
            while (!(inb(base + ATA_REG_STATUS) & ATA_STATUS_DRQ) && --timeout) {}
            if (timeout == 0) {
                serial_write_string("ATA READ timeout waiting for DRQ\n");
                return -1;
            }
        }
        
        // Read sector data
        for (int i = 0; i < 256; i++) {
            buf[sector * 256 + i] = inw(base + ATA_REG_DATA);
        }
    }
    
    return 0; // Success
}

int ata_write_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer) {
    uint16_t base = (drive < 2) ? ATA_PRIMARY_BASE : ATA_SECONDARY_BASE;
    uint8_t drive_select = ((drive % 2) ? 0xF0 : 0xE0) | ((lba >> 24) & 0x0F);
    
    // Wait for drive to be ready with timeout
    {
        uint32_t timeout = 1000000;
        while (!(inb(base + ATA_REG_STATUS) & ATA_STATUS_RDY) && --timeout) {}
        if (timeout == 0) {
            serial_write_string("ATA WRITE timeout waiting for RDY\n");
            return -1;
        }
    }
    
    // Set up LBA and sector count
    outb(base + ATA_REG_DRIVE_HEAD, drive_select);
    outb(base + ATA_REG_SECTOR_COUNT, count);
    outb(base + ATA_REG_LBA_LOW, lba & 0xFF);
    outb(base + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);
    outb(base + ATA_REG_LBA_HIGH, (lba >> 16) & 0xFF);
    
    // Send write command
    outb(base + ATA_REG_COMMAND, ATA_CMD_WRITE_SECTORS);
    
    uint16_t* buf = (uint16_t*)buffer;
    
    for (int sector = 0; sector < count; sector++) {
        // Wait for data request with timeout
        {
            uint32_t timeout = 1000000;
            while (!(inb(base + ATA_REG_STATUS) & ATA_STATUS_DRQ) && --timeout) {}
            if (timeout == 0) {
                serial_write_string("ATA WRITE timeout waiting for DRQ\n");
                return -1;
            }
        }
        
        // Write sector data
        for (int i = 0; i < 256; i++) {
            outw(base + ATA_REG_DATA, buf[sector * 256 + i]);
        }
    }
    
    // Wait for write completion with timeout
    {
        uint32_t timeout = 1000000;
        while ((inb(base + ATA_REG_STATUS) & (ATA_STATUS_BSY)) && --timeout) {}
        if (timeout == 0) {
            serial_write_string("ATA WRITE timeout waiting for completion\n");
            return -1;
        }
    }
    
    return 0; // Success
}

void ata_wait_busy(uint16_t base) {
    while (inb(base + ATA_REG_STATUS) & ATA_STATUS_BSY) {
        // Wait for BSY bit to clear
    }
}

void ata_wait_ready(uint16_t base) {
    while (!(inb(base + ATA_REG_STATUS) & ATA_STATUS_RDY)) {
        // Wait for RDY bit to set
    }
}

// Floppy disk functions (basic implementation)
int floppy_read_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer) {
    // TODO: Implement floppy disk reading
    serial_write_string("ERROR: Floppy disk read not implemented\n");
    return -1;
}

int floppy_write_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer) {
    // TODO: Implement floppy disk writing
    serial_write_string("ERROR: Floppy disk write not implemented\n");
    return -1;
}

void floppy_motor_on(uint8_t drive) {
    // TODO: Implement floppy motor control
}

void floppy_motor_off(uint8_t drive) {
    // TODO: Implement floppy motor control
}

// Utility functions
void disk_print_info(uint8_t drive) {
    disk_info_t* info = disk_get_info(drive);
    
    if (!info) {
        serial_write_string("Drive not found\n");
        return;
    }
    
    char buffer[32];
    
    serial_write_string("\n=== DISK INFORMATION ===\n");
    serial_write_string("Drive: ");
    itoa(info->drive_number, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    serial_write_string("Type: ");
    switch (info->disk_type) {
        case DISK_TYPE_ATA:
            serial_write_string("ATA/IDE");
            break;
        case DISK_TYPE_FLOPPY:
            serial_write_string("Floppy");
            break;
        default:
            serial_write_string("Unknown");
            break;
    }
    serial_write_string("\n");
    
    serial_write_string("Model: ");
    serial_write_string(info->model);
    serial_write_string("\n");
    
    serial_write_string("Total sectors: ");
    itoa(info->total_sectors, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    serial_write_string("Capacity: ");
    itoa(info->total_sectors * info->sector_size / 1024 / 1024, buffer, 10);
    serial_write_string(buffer);
    serial_write_string(" MB\n");
    
    serial_write_string("========================\n");
}

void disk_print_all_drives(void) {
    serial_write_string("\n=== ALL DETECTED DRIVES ===\n");
    
    for (int i = 0; i < drives_detected; i++) {
        if (drives[i].present) {
            disk_print_info(drives[i].drive_number);
        }
    }
    
    serial_write_string("===========================\n");
}

int disk_test_read_write(uint8_t drive) {
    disk_info_t* info = disk_get_info(drive);
    
    if (!info) {
        serial_write_string("ERROR: Drive not found for testing\n");
        return -1;
    }
    
    // Allocate test buffer
    uint8_t test_buffer[512];
    uint8_t verify_buffer[512];
    
    // Fill test buffer with pattern
    for (int i = 0; i < 512; i++) {
        test_buffer[i] = (i % 256);
    }
    
    serial_write_string("Testing disk I/O on drive ");
    char buffer[16];
    itoa(drive, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("...\n");
    
    // Test write (be careful with LBA selection!)
    uint32_t test_lba = 1000; // Use a safe LBA
    
    if (disk_write_sectors(drive, test_lba, 1, test_buffer) != 0) {
        serial_write_string("ERROR: Write test failed\n");
        return -1;
    }
    
    // Test read
    if (disk_read_sectors(drive, test_lba, 1, verify_buffer) != 0) {
        serial_write_string("ERROR: Read test failed\n");
        return -1;
    }
    
    // Verify data
    for (int i = 0; i < 512; i++) {
        if (test_buffer[i] != verify_buffer[i]) {
            serial_write_string("ERROR: Data verification failed\n");
            return -1;
        }
    }
    
    serial_write_string("Disk I/O test passed!\n");
    return 0;
}

// CHS/LBA conversion utilities
uint32_t chs_to_lba(uint16_t cylinder, uint8_t head, uint8_t sector, disk_geometry_t* geom) {
    return (cylinder * geom->heads + head) * geom->sectors_per_track + (sector - 1);
}

void lba_to_chs(uint32_t lba, uint16_t* cylinder, uint8_t* head, uint8_t* sector, disk_geometry_t* geom) {
    *sector = (lba % geom->sectors_per_track) + 1;
    *head = (lba / geom->sectors_per_track) % geom->heads;
    *cylinder = lba / (geom->sectors_per_track * geom->heads);
}

// External functions
extern void itoa(int value, char* str, int base); 