// kernel.c
// a very minimal c kernel for caseyos.

// Define constants for video memory and display attributes
#define REAL_MODE_VIDEO_MEM 0xB8000   // Standard VGA text mode address - this is correct for protected mode
// For real mode segmented addressing, we need to adjust how we access this
#define COLUMNS 80
#define ROWS 25
#define WHITE_ON_BLACK 0x07

// Function prototypes
void kernel_main(void);

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

// Entry point for the kernel - called from bootloader
void kernel_main() {
    // First try filling with pattern to ensure memory is working
    fill_screen_pattern();
    
    // Visible delay loop
    for (volatile int i = 0; i < 1000000; i++);
    
    // Clear screen
    clear_screen();
    
    // Print messages in highly visible area of screen
    const char *message = "*** CaseyOS Kernel Loaded Successfully! ***";
    k_print_string(message, WHITE_ON_BLACK, 0, 0);
    
    const char *info = "This is a simple OS kernel";
    k_print_string(info, WHITE_ON_BLACK, 2, 0);
    
    // Keep the kernel running
    while (1) {
        // Just loop (CPU will eventually be halted in real hardware)
        for (volatile int i = 0; i < 10000000; i++);
        
        // Blink something to show we're alive
        static char blinker = '|';
        if (blinker == '|') blinker = '-';
        else if (blinker == '-') blinker = '\\';
        else if (blinker == '\\') blinker = '/';
        else blinker = '|';
        
        k_print_char(blinker, 0x0F, 5, 0); // Bright white blinker
    }
}
