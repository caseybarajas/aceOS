#include "../include/libc/stdint.h"

// Enable interrupts
void enable_interrupts() {
    __asm__ volatile("sti");
}

// Get ESP register value
uint32_t get_esp() {
    uint32_t esp;
    __asm__ volatile("mov %%esp, %0" : "=r" (esp));
    return esp;
} 