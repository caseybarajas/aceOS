#include "pic.h"
#include "io.h"

// Initialize the PICs by remapping them
void pic_init() {
    // Save current masks
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);
    
    // Start initialization sequence
    outb(PIC1_COMMAND, ICW1_INIT);
    outb(PIC2_COMMAND, ICW1_INIT);
    
    // Set vector offsets: 
    // Master PIC starts at 32 (0x20), Slave PIC starts at 40 (0x28)
    outb(PIC1_DATA, 0x20);    // ICW2: Master PIC vector offset
    outb(PIC2_DATA, 0x28);    // ICW2: Slave PIC vector offset
    
    // Tell PICs how they're wired together
    outb(PIC1_DATA, 4);       // ICW3: Master has slave on IRQ2 (bit 2)
    outb(PIC2_DATA, 2);       // ICW3: Slave's cascade identity (cascade input is 2)
    
    // Set mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    
    // Restore saved masks
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

// Send End-of-Interrupt to PIC
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {  // If from slave PIC
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
}

// Set an IRQ mask (disable the IRQ)
void irq_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = inb(port) | (1 << irq);
    outb(port, value);
}

// Clear an IRQ mask (enable the IRQ)
void irq_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = inb(port) & ~(1 << irq);
    outb(port, value);
} 