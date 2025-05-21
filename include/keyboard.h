#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "isr.h"

// Keyboard ports
#define KEYBOARD_DATA_PORT      0x60
#define KEYBOARD_STATUS_PORT    0x64
#define KEYBOARD_COMMAND_PORT   0x64

// Keyboard commands
#define KEYBOARD_CMD_LED        0xED
#define KEYBOARD_CMD_ECHO       0xEE
#define KEYBOARD_CMD_SCANCODE   0xF0
#define KEYBOARD_CMD_IDENTIFY   0xF2
#define KEYBOARD_CMD_RATE       0xF3
#define KEYBOARD_CMD_ENABLE     0xF4
#define KEYBOARD_CMD_RESET      0xFF

// Special keys
#define KEY_ESCAPE      0x01
#define KEY_BACKSPACE   0x0E
#define KEY_TAB         0x0F
#define KEY_ENTER       0x1C
#define KEY_LCTRL       0x1D
#define KEY_LSHIFT      0x2A
#define KEY_RSHIFT      0x36
#define KEY_LALT        0x38
#define KEY_CAPSLOCK    0x3A
#define KEY_F1          0x3B
#define KEY_F2          0x3C
#define KEY_F3          0x3D
#define KEY_F4          0x3E
#define KEY_F5          0x3F
#define KEY_F6          0x40
#define KEY_F7          0x41
#define KEY_F8          0x42
#define KEY_F9          0x43
#define KEY_F10         0x44
#define KEY_F11         0x57
#define KEY_F12         0x58
#define KEY_NUMLOCK     0x45
#define KEY_SCROLLLOCK  0x46

// Function prototypes
void keyboard_init();
void keyboard_handler(registers_t regs);
char keyboard_getchar();

// Buffer functions
void keyboard_buffer_init();
void keyboard_buffer_push(char c);
char keyboard_buffer_pop();
uint8_t keyboard_buffer_empty();

#endif 