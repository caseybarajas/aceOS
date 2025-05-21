// kernel.c
// a very minimal c kernel for caseyos.

#include <stdint.h>
#include "idt.h"
#include "isr.h"
#include "keyboard.h"

// Define constants for video memory and display attributes
#define REAL_MODE_VIDEO_MEM 0xB8000   // Standard VGA text mode address - this is correct for protected mode
// For real mode segmented addressing, we need to adjust how we access this
#define COLUMNS 80
#define ROWS 25
#define WHITE_ON_BLACK 0x07
#define BRIGHT_WHITE_ON_BLACK 0x0F

// Terminal state
static int cursor_col = 0;
static int cursor_row = 10;

// Function prototypes
void kernel_main(void);
void terminal_putchar(char c);
void update_cursor();

// Entry point - this will be called by the bootloader
void _start() {
    kernel_main();
    
    // In case kernel_main ever returns (it shouldn't)
    while(1) {}
}

// Fill the screen with a pattern to make it obvious if the kernel is running
void fill_screen_pattern() {
    // Access video memory - we're loaded at 0x10000, but in real mode with segmented addresses
    volatile unsigned char *video_memory = (volatile unsigned char*)0xB8000;
    char pattern = 'A';
    
    // Write a pattern of characters to make it very visible
    for (int i = 0; i < COLUMNS * ROWS * 2; i += 2) {
        video_memory[i] = pattern++;
        video_memory[i + 1] = WHITE_ON_BLACK;
        
        // Reset pattern after 'Z'
        if (pattern > 'Z') pattern = 'A';
    }
}

// Clear the screen to black
void clear_screen() {
    volatile unsigned char *video_memory = (volatile unsigned char*)0xB8000;
    
    for (int i = 0; i < COLUMNS * ROWS * 2; i += 2) {
        video_memory[i] = ' ';          // space character
        video_memory[i + 1] = WHITE_ON_BLACK;
    }
}

// Print a single character at a specific position
void k_print_char(char c, unsigned char attribute, int row, int col) {
    volatile unsigned char *video_memory = (volatile unsigned char*)0xB8000;
    int offset = (row * COLUMNS + col) * 2;
    
    if (offset < 0 || offset >= COLUMNS * ROWS * 2) {
        return; // Safety check to prevent out-of-bounds access
    }
    
    video_memory[offset] = c;
    video_memory[offset + 1] = attribute;
}

// Print a string at a specific position
void k_print_string(const char *str, unsigned char attribute, int row, int start_col) {
    int col = start_col;
    
    if (!str) return;
    
    while (*str) {
        if (row >= 0 && row < ROWS && col >= 0 && col < COLUMNS) {
            k_print_char(*str, attribute, row, col);
        }
        
        str++;
        col++;
        
        if (col >= COLUMNS) {
            row++;
            col = 0;
            
            if (row >= ROWS) {
                row = ROWS - 1;
            }
        }
    }
}

// Draw cursor at current position
void update_cursor() {
    // Draw cursor (simple implementation - just a visible underscore character)
    k_print_char('_', BRIGHT_WHITE_ON_BLACK, cursor_row, cursor_col);
}

// Process a character input and display it in the terminal
void terminal_putchar(char c) {
    // Remove current cursor
    k_print_char(' ', WHITE_ON_BLACK, cursor_row, cursor_col);
    
    // Handle special characters
    if (c == '\n') {
        // New line
        cursor_row++;
        cursor_col = 0;
        
        // Scroll if needed
        if (cursor_row >= ROWS) {
            cursor_row = ROWS - 1;
            // In a full implementation, we would scroll the screen here
        }
    } 
    else if (c == '\b') {
        // Backspace
        if (cursor_col > 0) {
            cursor_col--;
            k_print_char(' ', WHITE_ON_BLACK, cursor_row, cursor_col);
        }
        else if (cursor_row > 10) {
            // Go to end of previous line
            cursor_row--;
            cursor_col = COLUMNS - 1;
            k_print_char(' ', WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (c == '\t') {
        // Tab - move cursor to next tab stop (every 8 columns)
        cursor_col = (cursor_col + 8) & ~7;
        
        // Wrap if needed
        if (cursor_col >= COLUMNS) {
            cursor_col = 0;
            cursor_row++;
            
            // Scroll if needed
            if (cursor_row >= ROWS) {
                cursor_row = ROWS - 1;
                // In a full implementation, we would scroll the screen here
            }
        }
    }
    else {
        // Regular character
        k_print_char(c, WHITE_ON_BLACK, cursor_row, cursor_col);
        cursor_col++;
        
        // Wrap if needed
        if (cursor_col >= COLUMNS) {
            cursor_col = 0;
            cursor_row++;
            
            // Scroll if needed
            if (cursor_row >= ROWS) {
                cursor_row = ROWS - 1;
                // In a full implementation, we would scroll the screen here
            }
        }
    }
    
    // Update cursor position
    update_cursor();
}

// Entry point for the kernel - called from bootloader
void kernel_main() {
    // Clear screen
    clear_screen();
    
    k_print_string("*** Kernel Loaded Successfully! ***", WHITE_ON_BLACK, 0, 0);
    
    // Initialize interrupt system
    k_print_string("Initializing interrupt system...", WHITE_ON_BLACK, 1, 0);
    idt_init();     // Set up and load the IDT
    isr_init();     // Set up interrupt service routines
    k_print_string("Interrupt system initialized!", WHITE_ON_BLACK, 2, 0);
    
    // Initialize keyboard
    keyboard_init();
    k_print_string("Keyboard initialized", WHITE_ON_BLACK, 3, 0);
    
    // Enable interrupts
    asm volatile("sti");
    
    k_print_string("Interrupts enabled", WHITE_ON_BLACK, 4, 0);
    
    // Keyboard input demo message
    k_print_string("Type on the keyboard (starting on line 10):", WHITE_ON_BLACK, 9, 0);


    cursor_col = 0;
    cursor_row = 10;
    
    // Main kernel loop
    while (1) {
        // Check for keyboard input (non-blocking)
        if (!keyboard_buffer_empty()) {
            char c = keyboard_getchar();
            terminal_putchar(c);
        }
    }
}