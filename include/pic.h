#ifndef PIC_H
#define PIC_H

#include <stdint.h>
#include "io.h"

// PIC ports
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// PIC initialization commands
#define ICW1_INIT    0x11    // Initialize the PIC and enable ICW4
#define ICW4_8086    0x01    // 8086/88 mode

// Function prototypes
void pic_init();
void pic_send_eoi(uint8_t irq);
void irq_set_mask(uint8_t irq);
void irq_clear_mask(uint8_t irq);

#endif 