#include "keyboard.h"
#include "io.h"
#include "pic.h"

// Keyboard buffer size
#define KEYBOARD_BUFFER_SIZE 256

// Global keyboard buffer variables
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static uint32_t buffer_head = 0;
static uint32_t buffer_tail = 0;

// Key state
static uint8_t shift_pressed = 0;
static uint8_t caps_lock_on = 0;

// Scancode to ASCII mapping for US keyboard (lowercase)
static const char scancode_ascii_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-',
    0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Scancode to ASCII mapping for US keyboard (uppercase)
static const char scancode_ascii_map_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-',
    0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Function to initialize the keyboard
void keyboard_init() {
    // Register our keyboard handler
    register_interrupt_handler(33, keyboard_handler); // IRQ1
    
    // Clear any existing keyboard buffer data
    keyboard_buffer_init();
    
    // Enable keyboard IRQ
    irq_clear_mask(1);
}

// Keyboard interrupt handler
void keyboard_handler(registers_t regs) {
    uint8_t scancode;
    
    // Read the scancode from the keyboard
    scancode = inb(KEYBOARD_DATA_PORT);
    
    // Check if this is a key release (bit 7 set)
    if (scancode & 0x80) {
        // Key release
        scancode &= 0x7F; // Clear bit 7 to get the actual key
        
        // Handle key releases
        if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT) {
            shift_pressed = 0;
        }
    } else {
        // Key press
        if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT) {
            shift_pressed = 1;
        } else if (scancode == KEY_CAPSLOCK) {
            caps_lock_on = !caps_lock_on;
        } else {
            // Convert scancode to ASCII
            char character = 0;
            
            // Handle letter case (uppercase or lowercase)
            if ((shift_pressed && !caps_lock_on) || (!shift_pressed && caps_lock_on)) {
                // Uppercase for letters
                if (scancode_ascii_map[scancode] >= 'a' && scancode_ascii_map[scancode] <= 'z') {
                    character = scancode_ascii_map[scancode] - 32; // Convert to uppercase
                } else {
                    // For non-letters, use shift mapping
                    character = scancode_ascii_map_shift[scancode];
                }
            } else if (shift_pressed && caps_lock_on) {
                // If both Shift and Caps Lock are active, letters are lowercase
                if (scancode_ascii_map_shift[scancode] >= 'A' && scancode_ascii_map_shift[scancode] <= 'Z') {
                    character = scancode_ascii_map_shift[scancode] + 32; // Convert to lowercase
                } else {
                    // For non-letters, use shift mapping
                    character = scancode_ascii_map_shift[scancode];
                }
            } else {
                // Normal case, no shift or caps lock for letters
                character = scancode_ascii_map[scancode];
            }
            
            // If we got a valid character, add it to the buffer
            if (character) {
                keyboard_buffer_push(character);
            }
        }
    }
}

// Initialize the keyboard buffer
void keyboard_buffer_init() {
    buffer_head = 0;
    buffer_tail = 0;
}

// Push a character into the keyboard buffer
void keyboard_buffer_push(char c) {
    // Calculate the next head position
    uint32_t next_head = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    
    // Check if buffer would be full
    if (next_head != buffer_tail) {
        // Buffer not full, add character
        keyboard_buffer[buffer_head] = c;
        buffer_head = next_head;
    }
    // If buffer is full, character is dropped
}

// Pop a character from the keyboard buffer
char keyboard_buffer_pop() {
    // Check if buffer is empty
    if (buffer_head == buffer_tail) {
        return 0; // Buffer empty
    }
    
    // Get the character at the tail
    char c = keyboard_buffer[buffer_tail];
    
    // Move tail to next position
    buffer_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    
    return c;
}

// Check if the keyboard buffer is empty
uint8_t keyboard_buffer_empty() {
    return buffer_head == buffer_tail;
}

// Get a character from the keyboard (blocking)
char keyboard_getchar() {
    char c;
    
    // Wait until a key is available
    while (keyboard_buffer_empty()) {
        // This is a busy wait, which is not ideal but simple
        // A more advanced system would use a semaphore or similar synchronization
    }
    
    // Get the next character
    c = keyboard_buffer_pop();
    
    return c;
} 