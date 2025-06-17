#ifndef MEMORY_H
#define MEMORY_H

#include "libc/stdint.h"
#include "libc/stddef.h"

// Memory layout constants
#define KERNEL_VIRTUAL_BASE     0xC0000000  // 3GB - kernel virtual address
#define KERNEL_PHYSICAL_BASE    0x00100000  // 1MB - where kernel is loaded
#define USER_VIRTUAL_BASE       0x40000000  // 1GB - user space starts here
#define PAGE_SIZE               4096        // 4KB pages
#define PAGE_DIRECTORY_SIZE     1024
#define PAGE_TABLE_SIZE         1024

// Page flags
#define PAGE_PRESENT            0x001
#define PAGE_WRITABLE           0x002
#define PAGE_USER               0x004
#define PAGE_WRITE_THROUGH      0x008
#define PAGE_CACHE_DISABLED     0x010
#define PAGE_ACCESSED           0x020
#define PAGE_DIRTY              0x040
#define PAGE_SIZE_FLAG          0x080
#define PAGE_GLOBAL             0x100

// Memory regions
#define MEMORY_REGION_AVAILABLE 1
#define MEMORY_REGION_RESERVED  2
#define MEMORY_REGION_RECLAIMABLE 3
#define MEMORY_REGION_NVS       4

// Physical memory manager
typedef struct {
    uint32_t* bitmap;
    uint32_t bitmap_size;
    uint32_t total_frames;
    uint32_t free_frames;
    uint32_t first_free_frame;
} physical_memory_manager_t;

// Virtual memory structures
typedef struct page_directory_entry {
    uint32_t present    : 1;
    uint32_t writable   : 1;
    uint32_t user       : 1;
    uint32_t reserved   : 2;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t size       : 1;
    uint32_t global     : 1;
    uint32_t available  : 3;
    uint32_t address    : 20;
} __attribute__((packed)) page_directory_entry_t;

typedef struct page_table_entry {
    uint32_t present    : 1;
    uint32_t writable   : 1;
    uint32_t user       : 1;
    uint32_t reserved   : 2;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t reserved2  : 1;
    uint32_t global     : 1;
    uint32_t available  : 3;
    uint32_t address    : 20;
} __attribute__((packed)) page_table_entry_t;

typedef struct page_directory {
    page_directory_entry_t entries[PAGE_DIRECTORY_SIZE];
} __attribute__((aligned(PAGE_SIZE))) page_directory_t;

typedef struct page_table {
    page_table_entry_t entries[PAGE_TABLE_SIZE];
} __attribute__((aligned(PAGE_SIZE))) page_table_t;

// Enhanced heap allocator
typedef struct heap_block {
    size_t size;
    int free;
    uint32_t magic;
    struct heap_block* next;
    struct heap_block* prev;
} heap_block_t;

typedef struct heap_manager {
    heap_block_t* first_block;
    void* heap_start;
    void* heap_end;
    size_t total_size;
    size_t free_size;
    uint32_t blocks_allocated;
    uint32_t blocks_free;
} heap_manager_t;

// Function prototypes
// Physical memory management
void pmm_init(void);
uint32_t pmm_alloc_frame(void);
void pmm_free_frame(uint32_t frame);
uint32_t pmm_get_free_frames(void);
void pmm_mark_frame_used(uint32_t frame);

// Virtual memory management
void vmm_init(void);
page_directory_t* vmm_create_page_directory(void);
void vmm_switch_page_directory(page_directory_t* dir);
void vmm_map_page(page_directory_t* dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
void vmm_unmap_page(page_directory_t* dir, uint32_t virtual_addr);
uint32_t vmm_get_physical_address(page_directory_t* dir, uint32_t virtual_addr);
void vmm_enable_paging(void);

// Enhanced heap management
void heap_init(void* start, size_t size);
void* heap_malloc(size_t size);
void* heap_calloc(size_t nmemb, size_t size);
void* heap_realloc(void* ptr, size_t size);
void heap_free(void* ptr);
void heap_print_stats(void);
int heap_validate(void);

// Memory utilities
void memory_copy_page(uint32_t dest, uint32_t src);
void memory_zero_page(uint32_t addr);
uint32_t memory_align_up(uint32_t addr, uint32_t alignment);
uint32_t memory_align_down(uint32_t addr, uint32_t alignment);

#endif /* MEMORY_H */ 