#include "libc/stdlib.h"
#include "libc/string.h"

// Simple memory allocator for an operating system
// This is a very basic implementation

// Memory region structure for our simple allocator
typedef struct memory_block {
    size_t size;
    int free;
    struct memory_block* next;
} memory_block_t;

// Heap starts at this address (adjust as needed for your OS)
#define HEAP_START 0x100000
#define HEAP_SIZE  0x100000 // 1MB heap

// Static allocation for now - replace with proper memory management later
static char heap[HEAP_SIZE];
static memory_block_t* free_list = NULL;
static int heap_initialized = 0;

// Initialize the heap
void initialize_heap() {
    if (heap_initialized) return;
    
    // Create the first free block that spans the entire heap
    free_list = (memory_block_t*)heap;
    free_list->size = HEAP_SIZE - sizeof(memory_block_t);
    free_list->free = 1;
    free_list->next = NULL;
    
    heap_initialized = 1;
}

// Allocate memory
void* malloc(size_t size) {
    memory_block_t* current;
    memory_block_t* previous;
    memory_block_t* new_block;
    void* result;
    
    // Initialize heap if not already done
    if (!heap_initialized) {
        initialize_heap();
    }
    
    // Align size to multiple of 8 bytes
    if (size % 8 != 0) {
        size = ((size / 8) + 1) * 8;
    }
    
    // Minimum allocation size
    if (size < 16) {
        size = 16;
    }
    
    // Search for a free block
    current = free_list;
    previous = NULL;
    
    while (current) {
        if (current->free && current->size >= size) {
            // Found a block
            if (current->size >= size + sizeof(memory_block_t) + 16) {
                // Split the block if it's large enough
                new_block = (memory_block_t*)((char*)current + sizeof(memory_block_t) + size);
                new_block->size = current->size - size - sizeof(memory_block_t);
                new_block->free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->free = 0;
            result = (void*)((char*)current + sizeof(memory_block_t));
            return result;
        }
        
        previous = current;
        current = current->next;
    }
    
    // No suitable block found
    return NULL;
}

// Free allocated memory
void free(void* ptr) {
    memory_block_t* block;
    memory_block_t* current;
    memory_block_t* next_block;
    
    if (!ptr) return;
    
    // Get the block header
    block = (memory_block_t*)((char*)ptr - sizeof(memory_block_t));
    
    // Mark the block as free
    block->free = 1;
    
    // Merge with adjacent free blocks
    current = free_list;
    
    while (current) {
        if (current->free && (char*)current + sizeof(memory_block_t) + current->size == (char*)block) {
            // Merge with previous block
            current->size += sizeof(memory_block_t) + block->size;
            current->next = block->next;
            block = current;
        } else if (current == block && current->next && current->next->free) {
            // Merge with next block
            next_block = current->next;
            current->size += sizeof(memory_block_t) + next_block->size;
            current->next = next_block->next;
        }
        
        current = current->next;
    }
}

// Allocate zeroed memory
void* calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    void* ptr = malloc(total_size);
    
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

// Reallocate memory
void* realloc(void* ptr, size_t size) {
    memory_block_t* block;
    void* new_ptr;
    
    if (!ptr) return malloc(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    // Get the block header
    block = (memory_block_t*)((char*)ptr - sizeof(memory_block_t));
    
    if (block->size >= size) {
        // Current block is large enough
        return ptr;
    }
    
    // Allocate a new block
    new_ptr = malloc(size);
    if (!new_ptr) return NULL;
    
    // Copy the data
    memcpy(new_ptr, ptr, block->size);
    
    // Free the old block
    free(ptr);
    
    return new_ptr;
}

// String to integer conversion
int atoi(const char* nptr) {
    int result = 0;
    int sign = 1;
    
    // Skip whitespace
    while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n') {
        nptr++;
    }
    
    // Handle sign
    if (*nptr == '-') {
        sign = -1;
        nptr++;
    } else if (*nptr == '+') {
        nptr++;
    }
    
    // Convert digits
    while (*nptr >= '0' && *nptr <= '9') {
        result = result * 10 + (*nptr - '0');
        nptr++;
    }
    
    return sign * result;
}

// String to long conversion
long atol(const char* nptr) {
    long result = 0;
    int sign = 1;
    
    // Skip whitespace
    while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n') {
        nptr++;
    }
    
    // Handle sign
    if (*nptr == '-') {
        sign = -1;
        nptr++;
    } else if (*nptr == '+') {
        nptr++;
    }
    
    // Convert digits
    while (*nptr >= '0' && *nptr <= '9') {
        result = result * 10 + (*nptr - '0');
        nptr++;
    }
    
    return sign * result;
}

// Random number generation
static unsigned long next = 1;

int rand(void) {
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % (RAND_MAX + 1);
}

void srand(unsigned int seed) {
    next = seed;
} 