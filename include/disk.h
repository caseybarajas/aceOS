#ifndef DISK_H
#define DISK_H

#include "libc/stdint.h"

// Disk types
#define DISK_TYPE_FLOPPY    1
#define DISK_TYPE_ATA       2
#define DISK_TYPE_UNKNOWN   0

// ATA/IDE ports
#define ATA_PRIMARY_BASE     0x1F0
#define ATA_SECONDARY_BASE   0x170
#define ATA_PRIMARY_CTRL     0x3F6
#define ATA_SECONDARY_CTRL   0x376

// ATA registers
#define ATA_REG_DATA         0x00
#define ATA_REG_ERROR        0x01
#define ATA_REG_FEATURES     0x01
#define ATA_REG_SECTOR_COUNT 0x02
#define ATA_REG_LBA_LOW      0x03
#define ATA_REG_LBA_MID      0x04
#define ATA_REG_LBA_HIGH     0x05
#define ATA_REG_DRIVE_HEAD   0x06
#define ATA_REG_STATUS       0x07
#define ATA_REG_COMMAND      0x07

// ATA status bits
#define ATA_STATUS_BSY       0x80  // Busy
#define ATA_STATUS_RDY       0x40  // Ready
#define ATA_STATUS_DWF       0x20  // Drive write fault
#define ATA_STATUS_DSC       0x10  // Drive seek complete
#define ATA_STATUS_DRQ       0x08  // Data request ready
#define ATA_STATUS_CORR      0x04  // Corrected data
#define ATA_STATUS_IDX       0x02  // Index
#define ATA_STATUS_ERR       0x01  // Error

// ATA commands
#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY      0xEC

// Floppy disk constants
#define FLOPPY_SECTORS_PER_TRACK  18
#define FLOPPY_HEADS             2
#define FLOPPY_TRACKS           80
#define FLOPPY_SECTOR_SIZE      512

// Disk geometry
typedef struct {
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectors_per_track;
    uint32_t sector_size;
    uint32_t total_sectors;
} disk_geometry_t;

// Disk information
typedef struct {
    uint8_t drive_number;
    uint8_t disk_type;
    char model[41];
    char serial[21];
    uint32_t total_sectors;
    uint32_t sector_size;
    disk_geometry_t geometry;
    int present;
} disk_info_t;

// Disk I/O request
typedef struct {
    uint8_t drive;
    uint32_t lba;
    uint16_t sector_count;
    void* buffer;
    int write;  // 0 = read, 1 = write
} disk_request_t;

// Function prototypes
// Disk initialization and detection
void disk_init(void);
int disk_detect_drives(void);
disk_info_t* disk_get_info(uint8_t drive);

// Low-level I/O
int disk_read_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer);
int disk_write_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer);

// ATA/IDE specific functions
int ata_identify_drive(uint8_t drive, disk_info_t* info);
int ata_read_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer);
int ata_write_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer);
void ata_wait_busy(uint16_t base);
void ata_wait_ready(uint16_t base);

// Floppy disk specific functions
int floppy_read_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer);
int floppy_write_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buffer);
void floppy_motor_on(uint8_t drive);
void floppy_motor_off(uint8_t drive);

// Utility functions
void disk_print_info(uint8_t drive);
void disk_print_all_drives(void);
int disk_test_read_write(uint8_t drive);

// CHS to LBA conversion
uint32_t chs_to_lba(uint16_t cylinder, uint8_t head, uint8_t sector, disk_geometry_t* geom);
void lba_to_chs(uint32_t lba, uint16_t* cylinder, uint8_t* head, uint8_t* sector, disk_geometry_t* geom);

#endif /* DISK_H */ 