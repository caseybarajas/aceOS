#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// IDT gate types
#define IDT_GATE_TASK      0x5
#define IDT_GATE_INT16     0x6
#define IDT_GATE_TRAP16    0x7
#define IDT_GATE_INT32     0xE
#define IDT_GATE_TRAP32    0xF

// Descriptor privilege levels
#define DPL_KERNEL         0x0
#define DPL_USER           0x3

// IDT gate descriptor structure
typedef struct {
    uint16_t offset_low;    // Lower 16 bits of handler address
    uint16_t selector;      // Code segment selector in GDT
    uint8_t reserved;       // Always 0
    uint8_t type_attr;      // Type and attributes
    uint16_t offset_high;   // Higher 16 bits of handler address
} __attribute__((packed)) idt_gate_t;

// IDT descriptor structure
typedef struct {
    uint16_t limit;         // Size of IDT - 1
    uint32_t base;          // Base address of IDT
} __attribute__((packed)) idt_descriptor_t;

// Function declarations
void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector, uint8_t type_attr);
void idt_init(void);

// Defined in idt.c
extern idt_gate_t idt[256];
extern idt_descriptor_t idt_desc;

#endif 