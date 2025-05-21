#include "idt.h"

// Define the IDT and its descriptor
idt_gate_t idt[256];
idt_descriptor_t idt_desc;

// External assembly function to load the IDT
extern void idt_load(uint32_t);

// Set an entry in the IDT
void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector, uint8_t type_attr) {
    idt[num].offset_low = (handler & 0xFFFF);
    idt[num].selector = selector;
    idt[num].reserved = 0;
    idt[num].type_attr = type_attr;
    idt[num].offset_high = ((handler >> 16) & 0xFFFF);
}

// Initialize and load the IDT
void idt_init(void) {
    // Set up the IDT descriptor
    idt_desc.limit = sizeof(idt_gate_t) * 256 - 1;
    idt_desc.base = (uint32_t)&idt;

    // Clear the IDT initially
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Load the IDT
    idt_load((uint32_t)&idt_desc);
} 