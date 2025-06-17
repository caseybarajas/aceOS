#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"
#include "isr.h"

// Serial port initialization
void serial_init();

// Basic I/O functions
void serial_write(char c);
void serial_write_string(const char* str);
void serial_write_hex(uint32_t value);
void serial_write_dec(uint32_t value);
uint8_t serial_read();

// Buffer functions
void serial_buffer_push(char c);
char serial_buffer_pop();
uint8_t serial_buffer_empty();

// Interrupt handler
void serial_interrupt_handler(registers_t regs);

// Debug functions
void debug_print(const char* message);
void debug_println(const char* message);
void debug_printf(const char* format, ...);
void debug_dump_registers(registers_t regs);
void debug_dump_stack(uint32_t* stack_ptr, uint32_t count);

#endif /* SERIAL_H */ 