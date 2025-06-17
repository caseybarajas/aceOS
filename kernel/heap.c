#include "../include/memory.h"
#include "../include/libc/string.h"
#include "../include/serial.h"
#include "../include/utils.h"

// Heap magic numbers for corruption detection
#define HEAP_MAGIC_ALLOCATED 0xABCDEF00
#define HEAP_MAGIC_FREE      0x12345678
#define HEAP_MAGIC_FOOTER    0x87654321

// Minimum allocation size (must be multiple of 8 for alignment)
#define MIN_ALLOC_SIZE 32

static heap_manager_t heap_manager;
static int heap_initialized = 0;

// Initialize the heap
void heap_init(void* start, size_t size) {
    serial_write_string("Initializing enhanced heap manager...\n");
    
    // Ensure start address is page-aligned
    uint32_t heap_start = memory_align_up((uint32_t)start, 16);
    size_t aligned_size = size - (heap_start - (uint32_t)start);
    
    heap_manager.heap_start = (void*)heap_start;
    heap_manager.heap_end = (void*)(heap_start + aligned_size);
    heap_manager.total_size = aligned_size;
    heap_manager.free_size = aligned_size - sizeof(heap_block_t);
    heap_manager.blocks_allocated = 0;
    heap_manager.blocks_free = 1;
    
    // Create the first free block
    heap_block_t* first_block = (heap_block_t*)heap_start;
    first_block->size = aligned_size - sizeof(heap_block_t);
    first_block->free = 1;
    first_block->magic = HEAP_MAGIC_FREE;
    first_block->next = NULL;
    first_block->prev = NULL;
    
    heap_manager.first_block = first_block;
    heap_initialized = 1;
    
    char buffer[32];
    serial_write_string("Heap initialized: ");
    itoa(aligned_size / 1024, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("KB available\n");
}

// Find best-fit free block
static heap_block_t* find_best_fit(size_t size) {
    heap_block_t* current = heap_manager.first_block;
    heap_block_t* best = NULL;
    size_t best_size = SIZE_MAX;
    
    while (current) {
        if (current->free && current->size >= size && current->size < best_size) {
            best = current;
            best_size = current->size;
            
            // Exact fit - perfect!
            if (current->size == size) {
                break;
            }
        }
        current = current->next;
    }
    
    return best;
}

// Split a block if it's large enough
static void split_block(heap_block_t* block, size_t size) {
    size_t remaining_size = block->size - size - sizeof(heap_block_t);
    
    // Only split if remaining size is worth it
    if (remaining_size >= MIN_ALLOC_SIZE) {
        heap_block_t* new_block = (heap_block_t*)((char*)block + sizeof(heap_block_t) + size);
        
        new_block->size = remaining_size;
        new_block->free = 1;
        new_block->magic = HEAP_MAGIC_FREE;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next) {
            block->next->prev = new_block;
        }
        
        block->next = new_block;
        block->size = size;
        
        heap_manager.blocks_free++;
    }
}

// Merge adjacent free blocks
static void merge_free_blocks(heap_block_t* block) {
    // Merge with next block if free
    if (block->next && block->next->free) {
        heap_block_t* next = block->next;
        
        block->size += sizeof(heap_block_t) + next->size;
        block->next = next->next;
        
        if (next->next) {
            next->next->prev = block;
        }
        
        heap_manager.blocks_free--;
    }
    
    // Merge with previous block if free
    if (block->prev && block->prev->free) {
        heap_block_t* prev = block->prev;
        
        prev->size += sizeof(heap_block_t) + block->size;
        prev->next = block->next;
        
        if (block->next) {
            block->next->prev = prev;
        }
        
        heap_manager.blocks_free--;
    }
}

// Enhanced malloc with alignment and validation
void* heap_malloc(size_t size) {
    if (!heap_initialized) {
        return NULL;
    }
    
    if (size == 0) {
        return NULL;
    }
    
    // Align size to 8-byte boundary
    size = memory_align_up(size, 8);
    
    // Ensure minimum allocation size
    if (size < MIN_ALLOC_SIZE) {
        size = MIN_ALLOC_SIZE;
    }
    
    // Find best-fit block
    heap_block_t* block = find_best_fit(size);
    if (!block) {
        return NULL; // Out of memory
    }
    
    // Split block if necessary
    split_block(block, size);
    
    // Mark block as allocated
    block->free = 0;
    block->magic = HEAP_MAGIC_ALLOCATED;
    
    // Update statistics
    heap_manager.blocks_allocated++;
    heap_manager.blocks_free--;
    heap_manager.free_size -= (size + sizeof(heap_block_t));
    
    // Return pointer to user data
    return (void*)((char*)block + sizeof(heap_block_t));
}

// Enhanced free with validation
void heap_free(void* ptr) {
    if (!ptr || !heap_initialized) {
        return;
    }
    
    // Get block header
    heap_block_t* block = (heap_block_t*)((char*)ptr - sizeof(heap_block_t));
    
    // Validate magic number
    if (block->magic != HEAP_MAGIC_ALLOCATED) {
        serial_write_string("HEAP ERROR: Invalid magic number in free()\n");
        return;
    }
    
    // Mark block as free
    block->free = 1;
    block->magic = HEAP_MAGIC_FREE;
    
    // Update statistics
    heap_manager.blocks_allocated--;
    heap_manager.blocks_free++;
    heap_manager.free_size += (block->size + sizeof(heap_block_t));
    
    // Merge with adjacent free blocks
    merge_free_blocks(block);
}

// Enhanced calloc
void* heap_calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    
    // Check for overflow
    if (nmemb != 0 && total_size / nmemb != size) {
        return NULL;
    }
    
    void* ptr = heap_malloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

// Enhanced realloc
void* heap_realloc(void* ptr, size_t size) {
    if (!ptr) {
        return heap_malloc(size);
    }
    
    if (size == 0) {
        heap_free(ptr);
        return NULL;
    }
    
    // Get current block
    heap_block_t* block = (heap_block_t*)((char*)ptr - sizeof(heap_block_t));
    
    // Validate magic number
    if (block->magic != HEAP_MAGIC_ALLOCATED) {
        return NULL;
    }
    
    // Align new size
    size = memory_align_up(size, 8);
    if (size < MIN_ALLOC_SIZE) {
        size = MIN_ALLOC_SIZE;
    }
    
    // If new size fits in current block, just return
    if (size <= block->size) {
        return ptr;
    }
    
    // Allocate new block
    void* new_ptr = heap_malloc(size);
    if (!new_ptr) {
        return NULL;
    }
    
    // Copy data
    memcpy(new_ptr, ptr, block->size);
    
    // Free old block
    heap_free(ptr);
    
    return new_ptr;
}

// Print heap statistics
void heap_print_stats(void) {
    char buffer[32];
    
    serial_write_string("\n=== HEAP STATISTICS ===\n");
    
    serial_write_string("Total size: ");
    itoa(heap_manager.total_size, buffer, 10);
    serial_write_string(buffer);
    serial_write_string(" bytes\n");
    
    serial_write_string("Free size: ");
    itoa(heap_manager.free_size, buffer, 10);
    serial_write_string(buffer);
    serial_write_string(" bytes\n");
    
    serial_write_string("Used size: ");
    itoa(heap_manager.total_size - heap_manager.free_size, buffer, 10);
    serial_write_string(buffer);
    serial_write_string(" bytes\n");
    
    serial_write_string("Allocated blocks: ");
    itoa(heap_manager.blocks_allocated, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    serial_write_string("Free blocks: ");
    itoa(heap_manager.blocks_free, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    // Calculate fragmentation
    uint32_t usage_percent = ((heap_manager.total_size - heap_manager.free_size) * 100) / heap_manager.total_size;
    serial_write_string("Usage: ");
    itoa(usage_percent, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("%\n");
    
    serial_write_string("======================\n");
}

// Validate heap integrity
int heap_validate(void) {
    if (!heap_initialized) {
        return 0;
    }
    
    heap_block_t* current = heap_manager.first_block;
    int block_count = 0;
    int errors = 0;
    
    while (current) {
        block_count++;
        
        // Check magic number
        uint32_t expected_magic = current->free ? HEAP_MAGIC_FREE : HEAP_MAGIC_ALLOCATED;
        if (current->magic != expected_magic) {
            serial_write_string("HEAP ERROR: Invalid magic in block ");
            char buffer[16];
            itoa(block_count, buffer, 10);
            serial_write_string(buffer);
            serial_write_string("\n");
            errors++;
        }
        
        // Check bounds
        if ((char*)current < (char*)heap_manager.heap_start || 
            (char*)current >= (char*)heap_manager.heap_end) {
            serial_write_string("HEAP ERROR: Block out of bounds\n");
            errors++;
        }
        
        current = current->next;
        
        // Prevent infinite loops
        if (block_count > 10000) {
            serial_write_string("HEAP ERROR: Possible infinite loop\n");
            errors++;
            break;
        }
    }
    
    if (errors == 0) {
        serial_write_string("Heap validation passed\n");
    }
    
    return errors == 0;
} 