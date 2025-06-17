#include <stdint.h>
#include "io.h"
#include "serial.h"
#include "isr.h"
#include "pic.h"

// Serial port I/O ports
#define SERIAL_COM1_PORT 0x3F8  // COM1 port base address

// Serial Port Registers (offsets from base address)
#define DATA_REG        0       // Data register (read/write)
#define INT_ENABLE_REG  1       // Interrupt enable
#define FIFO_CTRL_REG   2       // FIFO control
#define LINE_CTRL_REG   3       // Line control
#define MODEM_CTRL_REG  4       // Modem control
#define LINE_STATUS_REG 5       // Line status
#define MODEM_STATUS_REG 6      // Modem status
#define SCRATCH_REG     7       // Scratch register

// Line status bits
#define LSR_DR      0x01        // Data Ready
#define LSR_THRE    0x20        // Transmitter Holding Register Empty

// Buffer for receive data
#define SERIAL_BUFFER_SIZE 256
static char serial_buffer[SERIAL_BUFFER_SIZE];
static uint32_t buffer_head = 0;
static uint32_t buffer_tail = 0;
static uint8_t serial_initialized = 0;

// Initialize the serial port
void serial_init() {
    // Disable interrupts
    outb(SERIAL_COM1_PORT + INT_ENABLE_REG, 0x00);
    
    // Set baud rate to 38400 (115200/3)
    // First set DLAB (Divisor Latch Access Bit) to 1
    outb(SERIAL_COM1_PORT + LINE_CTRL_REG, 0x80);
    
    // Set divisor to 3 (38400 baud)
    // Low byte
    outb(SERIAL_COM1_PORT + DATA_REG, 0x03);
    // High byte
    outb(SERIAL_COM1_PORT + INT_ENABLE_REG, 0x00);
    
    // 8 bits, no parity, one stop bit (8N1)
    outb(SERIAL_COM1_PORT + LINE_CTRL_REG, 0x03);
    
    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_COM1_PORT + FIFO_CTRL_REG, 0xC7);
    
    // Set RTS/DSR, enable interrupts
    outb(SERIAL_COM1_PORT + MODEM_CTRL_REG, 0x0B);
    
    // Setup the receive interrupt to happen when data is available
    outb(SERIAL_COM1_PORT + INT_ENABLE_REG, 0x01);
    
    // Register the serial interrupt handler (IRQ4 -> INT 36)
    register_interrupt_handler(36, serial_interrupt_handler);
    
    // Enable COM1 interrupt (IRQ4)
    irq_clear_mask(4);
    
    // Setup buffer
    buffer_head = 0;
    buffer_tail = 0;
    
    serial_initialized = 1;
    
    // Send initial message to verify serial port is working
    serial_write_string("Serial port initialized - aceOS debugging enabled\r\n");
}

// Check if serial port transmit buffer is empty
static uint8_t serial_transmit_empty() {
    return inb(SERIAL_COM1_PORT + LINE_STATUS_REG) & LSR_THRE;
}

// Write a single byte to the serial port
void serial_write(char c) {
    // Wait until the transmitter is empty
    while (!serial_transmit_empty()) {
        // Busy wait
    }
    
    // Send the character
    outb(SERIAL_COM1_PORT + DATA_REG, c);
}

// Write a string to the serial port
void serial_write_string(const char* str) {
    if (!serial_initialized) return;
    
    while (*str) {
        serial_write(*str);
        str++;
    }
}

// Write a hexadecimal value to the serial port
void serial_write_hex(uint32_t value) {
    if (!serial_initialized) return;
    
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[11]; // 0x + 8 digits + null
    
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[10] = '\0';
    
    for (int i = 9; i >= 2; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    serial_write_string(buffer);
}

// Write a decimal value to the serial port
void serial_write_dec(uint32_t value) {
    if (!serial_initialized) return;
    
    char buffer[12]; // Max 10 digits for 32-bit + null
    char* p = buffer + sizeof(buffer) - 1;
    *p = '\0';
    
    do {
        *(--p) = '0' + (value % 10);
        value /= 10;
    } while (value > 0);
    
    serial_write_string(p);
}

// Read a byte from the serial port (non-blocking)
uint8_t serial_read() {
    // Check if data is available
    if (inb(SERIAL_COM1_PORT + LINE_STATUS_REG) & LSR_DR) {
        return inb(SERIAL_COM1_PORT + DATA_REG);
    }
    return 0;
}

// Push a character into the serial buffer
void serial_buffer_push(char c) {
    // Calculate the next head position
    uint32_t next_head = (buffer_head + 1) % SERIAL_BUFFER_SIZE;
    
    // Check if buffer would be full
    if (next_head != buffer_tail) {
        // Buffer not full, add character
        serial_buffer[buffer_head] = c;
        buffer_head = next_head;
    }
    // If buffer is full, character is dropped
}

// Pop a character from the serial buffer
char serial_buffer_pop() {
    // Check if buffer is empty
    if (buffer_head == buffer_tail) {
        return 0; // Buffer empty
    }
    
    // Get the character at the tail
    char c = serial_buffer[buffer_tail];
    
    // Move tail to next position
    buffer_tail = (buffer_tail + 1) % SERIAL_BUFFER_SIZE;
    
    return c;
}

// Check if the serial buffer is empty
uint8_t serial_buffer_empty() {
    return buffer_head == buffer_tail;
}

// Serial interrupt handler
void serial_interrupt_handler(registers_t regs) {
    // Read the byte from the serial port
    uint8_t data = inb(SERIAL_COM1_PORT + DATA_REG);
    
    // Add to buffer
    serial_buffer_push((char)data);
    
    // Echo back what was received
    serial_write((char)data);
}

// Debug functions
void debug_print(const char* message) {
    serial_write_string(message);
}

void debug_println(const char* message) {
    serial_write_string(message);
    serial_write_string("\r\n");
}

void debug_printf(const char* format, ...) {
    // This would be a more complex implementation
    // For now, just write the format string
    serial_write_string(format);
    serial_write_string("\r\n");
}

// Functions for system state debugging
void debug_dump_registers(registers_t regs) {
    serial_write_string("Register Dump:\r\n");
    serial_write_string("EAX: 0x"); serial_write_hex(regs.eax); serial_write_string("\r\n");
    serial_write_string("EBX: 0x"); serial_write_hex(regs.ebx); serial_write_string("\r\n");
    serial_write_string("ECX: 0x"); serial_write_hex(regs.ecx); serial_write_string("\r\n");
    serial_write_string("EDX: 0x"); serial_write_hex(regs.edx); serial_write_string("\r\n");
    serial_write_string("ESP: 0x"); serial_write_hex(regs.esp); serial_write_string("\r\n");
    serial_write_string("EBP: 0x"); serial_write_hex(regs.ebp); serial_write_string("\r\n");
    serial_write_string("ESI: 0x"); serial_write_hex(regs.esi); serial_write_string("\r\n");
    serial_write_string("EDI: 0x"); serial_write_hex(regs.edi); serial_write_string("\r\n");
    serial_write_string("EIP: 0x"); serial_write_hex(regs.eip); serial_write_string("\r\n");
}

void debug_dump_stack(uint32_t* stack_ptr, uint32_t count) {
    serial_write_string("Stack Dump:\r\n");
    
    for (uint32_t i = 0; i < count; i++) {
        serial_write_string("[0x");
        serial_write_hex((uint32_t)&stack_ptr[i]);
        serial_write_string("]: 0x");
        serial_write_hex(stack_ptr[i]);
        serial_write_string("\r\n");
    }
} 