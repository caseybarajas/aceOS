#include "../include/memory.h"
#include "../include/libc/string.h"
#include "../include/serial.h"
#include "../include/utils.h"

// Physical memory manager instance
static physical_memory_manager_t pmm;

// Memory starts after kernel (2MB for safety)
#define MEMORY_START 0x200000
#define MEMORY_SIZE  0x1E00000  // 30MB available (total 32MB assumed)

// Initialize physical memory manager
void pmm_init(void) {
    serial_write_string("Initializing Physical Memory Manager...\n");
    
    // Calculate number of frames
    pmm.total_frames = MEMORY_SIZE / PAGE_SIZE;
    pmm.free_frames = pmm.total_frames;
    pmm.first_free_frame = 0;
    
    // Calculate bitmap size (1 bit per frame)
    pmm.bitmap_size = (pmm.total_frames + 31) / 32; // Round up to 32-bit words
    
    // Place bitmap at start of available memory
    pmm.bitmap = (uint32_t*)MEMORY_START;
    
    // Clear bitmap (all frames free initially)
    memset(pmm.bitmap, 0, pmm.bitmap_size * sizeof(uint32_t));
    
    // Mark frames used by bitmap itself
    uint32_t bitmap_frames = (pmm.bitmap_size * sizeof(uint32_t) + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = 0; i < bitmap_frames; i++) {
        pmm_mark_frame_used(i);
    }
    
    serial_write_string("PMM: Initialized with ");
    char buffer[32];
    itoa(pmm.total_frames, buffer, 10);
    serial_write_string(buffer);
    serial_write_string(" frames (");
    itoa(pmm.total_frames * PAGE_SIZE / 1024 / 1024, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("MB)\n");
}

// Allocate a physical frame
uint32_t pmm_alloc_frame(void) {
    if (pmm.free_frames == 0) {
        return 0; // Out of memory
    }
    
    // Start searching from first_free_frame for efficiency
    for (uint32_t i = pmm.first_free_frame; i < pmm.total_frames; i++) {
        uint32_t word_index = i / 32;
        uint32_t bit_index = i % 32;
        
        if (!(pmm.bitmap[word_index] & (1 << bit_index))) {
            // Found free frame
            pmm.bitmap[word_index] |= (1 << bit_index);
            pmm.free_frames--;
            
            // Update first_free_frame hint
            if (i == pmm.first_free_frame) {
                pmm.first_free_frame++;
            }
            
            // Return physical address
            return MEMORY_START + (i * PAGE_SIZE);
        }
    }
    
    // Fallback: search from beginning
    for (uint32_t i = 0; i < pmm.first_free_frame; i++) {
        uint32_t word_index = i / 32;
        uint32_t bit_index = i % 32;
        
        if (!(pmm.bitmap[word_index] & (1 << bit_index))) {
            // Found free frame
            pmm.bitmap[word_index] |= (1 << bit_index);
            pmm.free_frames--;
            
            // Return physical address
            return MEMORY_START + (i * PAGE_SIZE);
        }
    }
    
    return 0; // Out of memory
}

// Free a physical frame
void pmm_free_frame(uint32_t frame_addr) {
    if (frame_addr < MEMORY_START) {
        return; // Invalid address
    }
    
    uint32_t frame_index = (frame_addr - MEMORY_START) / PAGE_SIZE;
    
    if (frame_index >= pmm.total_frames) {
        return; // Invalid frame
    }
    
    uint32_t word_index = frame_index / 32;
    uint32_t bit_index = frame_index % 32;
    
    if (pmm.bitmap[word_index] & (1 << bit_index)) {
        // Frame was allocated, now free it
        pmm.bitmap[word_index] &= ~(1 << bit_index);
        pmm.free_frames++;
        
        // Update first_free_frame hint if this frame is earlier
        if (frame_index < pmm.first_free_frame) {
            pmm.first_free_frame = frame_index;
        }
    }
}

// Mark a frame as used
void pmm_mark_frame_used(uint32_t frame_index) {
    if (frame_index >= pmm.total_frames) {
        return;
    }
    
    uint32_t word_index = frame_index / 32;
    uint32_t bit_index = frame_index % 32;
    
    if (!(pmm.bitmap[word_index] & (1 << bit_index))) {
        // Frame was free, now mark as used
        pmm.bitmap[word_index] |= (1 << bit_index);
        pmm.free_frames--;
    }
}

// Get number of free frames
uint32_t pmm_get_free_frames(void) {
    return pmm.free_frames;
}

// Simple itoa implementation for debugging
void itoa(int value, char* str, int base) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    int tmp_value;
    
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
        ptr1++;
    }
    
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while (value);
    
    *ptr-- = '\0';
    
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
} 