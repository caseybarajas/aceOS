#ifndef CPU_H
#define CPU_H

#include "libc/stdint.h"

// Enable interrupts
void enable_interrupts();

// Get ESP register value
uint32_t get_esp();

#endif 