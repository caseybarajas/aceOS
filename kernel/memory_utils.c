#include "../include/memory.h"
#include "../include/libc/string.h"

// Align address upward to the given alignment
uint32_t memory_align_up(uint32_t addr, uint32_t alignment) {
    if (alignment == 0) return addr;
    
    uint32_t remainder = addr % alignment;
    if (remainder == 0) {
        return addr;
    }
    
    return addr + (alignment - remainder);
}

// Align address downward to the given alignment
uint32_t memory_align_down(uint32_t addr, uint32_t alignment) {
    if (alignment == 0) return addr;
    
    return addr - (addr % alignment);
}

// Copy one page of memory
void memory_copy_page(uint32_t dest, uint32_t src) {
    // Ensure addresses are page-aligned
    dest = memory_align_down(dest, PAGE_SIZE);
    src = memory_align_down(src, PAGE_SIZE);
    
    // Copy 4KB page
    memcpy((void*)dest, (void*)src, PAGE_SIZE);
}

// Zero out a page of memory
void memory_zero_page(uint32_t addr) {
    // Ensure address is page-aligned
    addr = memory_align_down(addr, PAGE_SIZE);
    
    // Zero 4KB page
    memset((void*)addr, 0, PAGE_SIZE);
}

// Get SIZE_MAX for heap allocator
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

// External itoa declaration (defined in pmm.c)
extern void itoa(int value, char* str, int base); 