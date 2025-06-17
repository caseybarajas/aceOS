#include "../include/memory.h"
#include "../include/libc/string.h"
#include "../include/serial.h"

// Current page directory
static page_directory_t* current_page_directory = NULL;
static page_directory_t kernel_page_directory;

// Assembly functions for paging (we'll need to implement these)
extern void vmm_load_page_directory(uint32_t page_dir_physical);
extern void vmm_enable_paging_asm(void);
extern void vmm_flush_tlb(void);

// Get page directory index from virtual address
#define GET_PD_INDEX(addr) ((addr) >> 22)

// Get page table index from virtual address  
#define GET_PT_INDEX(addr) (((addr) >> 12) & 0x3FF)

// Get page-aligned address
#define PAGE_ALIGN(addr) ((addr) & ~(PAGE_SIZE - 1))

// Initialize virtual memory manager
void vmm_init(void) {
    serial_write_string("Initializing Virtual Memory Manager...\n");
    
    // Clear kernel page directory
    memset(&kernel_page_directory, 0, sizeof(page_directory_t));
    
    // Identity map first 4MB (kernel space)
    // This ensures kernel code can continue running after paging is enabled
    for (uint32_t addr = 0; addr < 0x400000; addr += PAGE_SIZE) {
        vmm_map_page(&kernel_page_directory, addr, addr, 
                     PAGE_PRESENT | PAGE_WRITABLE);
    }
    
    // Map kernel virtual space to physical space
    for (uint32_t addr = 0; addr < 0x400000; addr += PAGE_SIZE) {
        vmm_map_page(&kernel_page_directory, KERNEL_VIRTUAL_BASE + addr, addr,
                     PAGE_PRESENT | PAGE_WRITABLE);
    }
    
    current_page_directory = &kernel_page_directory;
    
    serial_write_string("VMM: Kernel page directory created\n");
}

// Create a new page directory
page_directory_t* vmm_create_page_directory(void) {
    // Allocate physical frame for page directory
    uint32_t page_dir_phys = pmm_alloc_frame();
    if (!page_dir_phys) {
        return NULL;
    }
    
    page_directory_t* page_dir = (page_directory_t*)page_dir_phys;
    
    // Clear the page directory
    memset(page_dir, 0, sizeof(page_directory_t));
    
    // Copy kernel mappings from current directory
    for (int i = GET_PD_INDEX(KERNEL_VIRTUAL_BASE); i < PAGE_DIRECTORY_SIZE; i++) {
        page_dir->entries[i] = kernel_page_directory.entries[i];
    }
    
    return page_dir;
}

// Switch to a different page directory
void vmm_switch_page_directory(page_directory_t* dir) {
    current_page_directory = dir;
    uint32_t dir_phys = (uint32_t)dir;
    vmm_load_page_directory(dir_phys);
}

// Map a virtual page to a physical page
void vmm_map_page(page_directory_t* dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    // Align addresses to page boundaries
    virtual_addr = PAGE_ALIGN(virtual_addr);
    physical_addr = PAGE_ALIGN(physical_addr);
    
    uint32_t pd_index = GET_PD_INDEX(virtual_addr);
    uint32_t pt_index = GET_PT_INDEX(virtual_addr);
    
    // Check if page table exists
    if (!(dir->entries[pd_index].present)) {
        // Allocate new page table
        uint32_t page_table_phys = pmm_alloc_frame();
        if (!page_table_phys) {
            return; // Out of memory
        }
        
        // Clear the page table
        page_table_t* page_table = (page_table_t*)page_table_phys;
        memset(page_table, 0, sizeof(page_table_t));
        
        // Set page directory entry
        dir->entries[pd_index].present = 1;
        dir->entries[pd_index].writable = 1;
        dir->entries[pd_index].user = (flags & PAGE_USER) ? 1 : 0;
        dir->entries[pd_index].address = page_table_phys >> 12;
    }
    
    // Get page table
    page_table_t* page_table = (page_table_t*)(dir->entries[pd_index].address << 12);
    
    // Set page table entry
    page_table->entries[pt_index].present = (flags & PAGE_PRESENT) ? 1 : 0;
    page_table->entries[pt_index].writable = (flags & PAGE_WRITABLE) ? 1 : 0;
    page_table->entries[pt_index].user = (flags & PAGE_USER) ? 1 : 0;
    page_table->entries[pt_index].address = physical_addr >> 12;
    
    // Flush TLB for this page
    vmm_flush_tlb();
}

// Unmap a virtual page
void vmm_unmap_page(page_directory_t* dir, uint32_t virtual_addr) {
    virtual_addr = PAGE_ALIGN(virtual_addr);
    
    uint32_t pd_index = GET_PD_INDEX(virtual_addr);
    uint32_t pt_index = GET_PT_INDEX(virtual_addr);
    
    // Check if page table exists
    if (!(dir->entries[pd_index].present)) {
        return; // Page not mapped
    }
    
    // Get page table
    page_table_t* page_table = (page_table_t*)(dir->entries[pd_index].address << 12);
    
    // Get physical address before unmapping
    uint32_t physical_addr = page_table->entries[pt_index].address << 12;
    
    // Clear page table entry
    memset(&page_table->entries[pt_index], 0, sizeof(page_table_entry_t));
    
    // Free the physical frame
    if (physical_addr) {
        pmm_free_frame(physical_addr);
    }
    
    // Flush TLB for this page
    vmm_flush_tlb();
}

// Get physical address for a virtual address
uint32_t vmm_get_physical_address(page_directory_t* dir, uint32_t virtual_addr) {
    uint32_t pd_index = GET_PD_INDEX(virtual_addr);
    uint32_t pt_index = GET_PT_INDEX(virtual_addr);
    uint32_t offset = virtual_addr & 0xFFF;
    
    // Check if page table exists
    if (!(dir->entries[pd_index].present)) {
        return 0; // Page not mapped
    }
    
    // Get page table
    page_table_t* page_table = (page_table_t*)(dir->entries[pd_index].address << 12);
    
    // Check if page is mapped
    if (!(page_table->entries[pt_index].present)) {
        return 0; // Page not mapped
    }
    
    return (page_table->entries[pt_index].address << 12) | offset;
}

// Enable paging
void vmm_enable_paging(void) {
    serial_write_string("VMM: Enabling paging...\n");
    
    // Load kernel page directory
    vmm_switch_page_directory(&kernel_page_directory);
    
    // Enable paging in assembly
    vmm_enable_paging_asm();
    
    serial_write_string("VMM: Paging enabled successfully\n");
} 