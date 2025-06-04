// kernel.c
// a very minimal c kernel for aceos.

#include "libc/stdint.h"
#include "idt.h"
#include "isr.h"
#include "keyboard.h"
#include "serial.h"  // Include the serial driver
#include "libc/libc.h"
#include "fs.h"      // Include our new filesystem

// Define constants for video memory and display attributes
#define REAL_MODE_VIDEO_MEM 0xB8000   // Standard VGA text mode address - this is correct for protected mode
// For real mode segmented addressing, we need to adjust how we access this
#define COLUMNS 80
#define ROWS 25
#define WHITE_ON_BLACK 0x07
#define BRIGHT_WHITE_ON_BLACK 0x0F

// Shell constants
#define MAX_COMMAND_LENGTH 72
#define COMMAND_HISTORY_SIZE 10
#define SHELL_PROMPT "aceOS> "

// Terminal state
static int cursor_col = 0;
static int cursor_row = 10;

// Shell state
static char current_command[MAX_COMMAND_LENGTH];
static int command_length = 0;
static char command_history[COMMAND_HISTORY_SIZE][MAX_COMMAND_LENGTH];
static int history_count = 0;
static int history_position = -1;

// Function prototypes
void kernel_main(void);
void terminal_putchar(char c);
void update_cursor();
void print_shell_prompt();
void process_command(const char* command);
void execute_command(const char* command);
void clear_command_buffer();
void shell_handle_input(char c);

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

// Print the shell prompt
void print_shell_prompt() {
    cursor_row++;
    k_print_string(SHELL_PROMPT, WHITE_ON_BLACK, cursor_row, 0);
    cursor_col = sizeof(SHELL_PROMPT) - 1;  // Move cursor after prompt
    update_cursor();
}

// Process and execute the command
void process_command(const char* command) {
    serial_write_string("Processing command: ");
    serial_write_string(command);

    // Add to history if not empty
    if (command_length > 0) {
        // Copy command to history
        int i;
        for (i = 0; i < command_length && i < MAX_COMMAND_LENGTH - 1; i++) {
            command_history[history_count][i] = command[i];
        }
        command_history[history_count][i] = '\0';
        
        history_count = (history_count + 1) % COMMAND_HISTORY_SIZE;
        history_position = -1;  // Reset history position
        
        // Execute the command
        execute_command(command);
    }
    
    // Start a new line for the next prompt
    cursor_row++;
    cursor_col = 0;
    
    // Show the prompt again
    print_shell_prompt();
}

// Execute shell commands
void execute_command(const char* command) {
    if (strcmp(command, "help") == 0) {
        cursor_row++;
        cursor_col = 0;
        k_print_string("Available commands:", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("help     - Show this help", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("clear    - Clear the screen", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("version  - Show OS version", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("echo     - Echo text to screen", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("debug    - Send test message to serial debug port", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("pwd      - Show current directory", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("cd       - Change directory", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("ls       - List files in directory", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("mkdir    - Create a directory", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("touch    - Create an empty file", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("cat      - Display file contents", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("cp       - Copy file", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("mv       - Move/rename file", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("rm       - Remove file or directory", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("write    - Write content to a file", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("find     - Find files by name pattern", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("tree     - Show directory tree structure", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("stat     - Show file information", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        cursor_row++;
        cursor_col = 2;
        k_print_string("fsinfo   - Display filesystem information", WHITE_ON_BLACK, cursor_row, cursor_col);
    }
    else if (strcmp(command, "clear") == 0) {
        clear_screen();
        cursor_row = -2; // forces the prompt to be at the top of the screen
        cursor_col = 0;
    }
    else if (strcmp(command, "version") == 0) {
        cursor_row++;
        cursor_col = 0;
        k_print_string("aceOS v0.1", WHITE_ON_BLACK, cursor_row, cursor_col);
    }
    else if (strncmp(command, "echo ", 5) == 0) {
        cursor_row++;
        cursor_col = 0;
        k_print_string(command + 5, WHITE_ON_BLACK, cursor_row, cursor_col);
    }
    else if (strcmp(command, "debug") == 0) {
        cursor_row++;
        cursor_col = 0;
        k_print_string("Sending debug information to serial port...", WHITE_ON_BLACK, cursor_row, cursor_col);
        
        // Send debug info to serial port
        debug_println("===== DEBUG INFO =====");
        debug_print("OS Version: ");
        debug_println("aceOS v0.1");
        debug_print("Memory at 0x10000: Kernel loaded (");
        serial_write_dec(60);
        debug_println(" sectors)");
        
        // Stack pointer info
        debug_println("Stack pointer is in kernel space");
        
        // Show memory usage (simplified)
        debug_println("Memory Map:");
        debug_println("0x00000000 - 0x000003FF: Real Mode IVT");
        debug_println("0x00000400 - 0x000004FF: BIOS Data Area");
        debug_println("0x00007C00 - 0x00007DFF: Our Bootloader");
        debug_println("0x00010000 - 0x0001FFFF: Our Kernel");
        debug_println("=====================");
    }
    else if (strcmp(command, "ls") == 0 || strncmp(command, "ls ", 3) == 0) {
        cursor_row++;
        cursor_col = 0;
        
        // Get path parameter if any, otherwise use current directory
        const char* path;
        if (strlen(command) > 3) {
            path = command + 3;
            // Skip leading spaces
            while (*path == ' ' && *path != '\0') {
                path++;
            }
            // If path is empty after trimming spaces, use current directory
            if (*path == '\0') {
                path = fs_get_current_dir();
            }
        } else {
            path = fs_get_current_dir();
        }
        
        // Buffer to hold directory listing
        char buffer[1024];
        int result = fs_list_dir(path, buffer, sizeof(buffer));
        
        // Debug output
        debug_print("ls result: ");
        serial_write_dec(result);
        debug_print(", buffer: ");
        debug_println(buffer);
        
        if (result >= 0) {
            // Successfully listed directory, display results
            k_print_string(buffer, WHITE_ON_BLACK, cursor_row, cursor_col);
            
            // If buffer was empty, show message
            if (result == 0) {
                k_print_string("(directory is empty)", WHITE_ON_BLACK, cursor_row, cursor_col);
            }
        } else {
            // Error occurred
            k_print_string("Error: Could not list directory", WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (strncmp(command, "mkdir ", 6) == 0) {
        cursor_row++;
        cursor_col = 0;
        
        // Get the directory path
        const char* path = command + 6;
        
        // Create the directory
        int result = fs_mkdir(path);
        
        if (result == 0) {
            k_print_string("Directory created successfully", WHITE_ON_BLACK, cursor_row, cursor_col);
        } else {
            k_print_string("Error: Could not create directory", WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (strncmp(command, "touch ", 6) == 0) {
        cursor_row++;
        cursor_col = 0;
        
        // Get the file path
        const char* path = command + 6;
        
        // Create an empty file
        int result = fs_create(path, 0);
        
        if (result == 0) {
            k_print_string("File created successfully", WHITE_ON_BLACK, cursor_row, cursor_col);
        } else {
            k_print_string("Error: Could not create file", WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (strncmp(command, "cat ", 4) == 0) {
        cursor_row++;
        cursor_col = 0;
        
        // Get the file path
        const char* path = command + 4;
        
        // Buffer to hold file content
        char buffer[1024];
        int bytes_read = fs_read(path, buffer, sizeof(buffer) - 1);
        
        if (bytes_read >= 0) {
            // Null terminate the buffer
            buffer[bytes_read] = '\0';
            
            // Display file content
            k_print_string("File content:", WHITE_ON_BLACK, cursor_row, cursor_col);
            cursor_row++;
            
            if (bytes_read > 0) {
                k_print_string(buffer, WHITE_ON_BLACK, cursor_row, cursor_col);
            } else {
                k_print_string("(empty file)", WHITE_ON_BLACK, cursor_row, cursor_col);
            }
        } else {
            k_print_string("Error: Could not read file", WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (strncmp(command, "rm ", 3) == 0) {
        cursor_row++;
        cursor_col = 0;
        
        // Get the path
        const char* path = command + 3;
        
        // Delete the file or directory
        int result = fs_delete(path);
        
        if (result == 0) {
            k_print_string("File or directory deleted successfully", WHITE_ON_BLACK, cursor_row, cursor_col);
        } else {
            k_print_string("Error: Could not delete file or directory", WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (strncmp(command, "write ", 6) == 0) {
        // Command format: write filepath content
        cursor_row++;
        cursor_col = 0;
        
        // Extract filepath
        char filepath[FS_MAX_PATH_LEN] = {0};
        const char* content_start = NULL;
        int filepath_len = 0;
        
        // Skip "write " prefix
        const char* cmd_ptr = command + 6;
        
        // Skip any spaces
        while (*cmd_ptr == ' ' && *cmd_ptr != '\0') {
            cmd_ptr++;
        }
        
        // Extract filepath (until next space)
        while (*cmd_ptr != ' ' && *cmd_ptr != '\0' && filepath_len < FS_MAX_PATH_LEN - 1) {
            filepath[filepath_len++] = *cmd_ptr++;
        }
        
        // Skip any spaces after filepath
        while (*cmd_ptr == ' ' && *cmd_ptr != '\0') {
            cmd_ptr++;
        }
        
        // Set content pointer
        content_start = cmd_ptr;
        
        if (filepath_len > 0 && *content_start != '\0') {
            // Write content to the file
            int result = fs_write(filepath, content_start, strlen(content_start));
            
            if (result == 0) {
                k_print_string("File written successfully", WHITE_ON_BLACK, cursor_row, cursor_col);
            } else {
                k_print_string("Error: Could not write to file", WHITE_ON_BLACK, cursor_row, cursor_col);
            }
        } else {
            k_print_string("Usage: write filepath content", WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (strcmp(command, "fsinfo") == 0) {
        cursor_row++;
        cursor_col = 0;
        
        // Print filesystem info to serial port (it's more useful there)
        fs_print_stats();
        
        k_print_string("Filesystem information printed to serial port", WHITE_ON_BLACK, cursor_row, cursor_col);
    }
    else if (strcmp(command, "pwd") == 0) {
        cursor_row++;
        cursor_col = 0;
        
        char* current_dir = fs_get_current_dir();
        k_print_string("Current directory: ", WHITE_ON_BLACK, cursor_row, cursor_col);
        cursor_col = 19;
        k_print_string(current_dir, WHITE_ON_BLACK, cursor_row, cursor_col);
    }
    else if (strncmp(command, "cd ", 3) == 0) {
        cursor_row++;
        cursor_col = 0;
        
        const char* path = command + 3;
        
        // Debug output
        debug_print("cd attempting to change to: ");
        debug_println(path);
        
        int result = fs_change_dir(path);
        
        if (result == 0) {
            k_print_string("Directory changed successfully", WHITE_ON_BLACK, cursor_row, cursor_col);
        } else {
            k_print_string("Error: Could not change directory", WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (strcmp(command, "cd") == 0) {
        cursor_row++;
        cursor_col = 0;
        k_print_string("Usage: cd <directory>", WHITE_ON_BLACK, cursor_row, cursor_col);
    }
    else if (strncmp(command, "cp ", 3) == 0) {
        cursor_row++;
        cursor_col = 0;
        
        // Parse source and destination paths
        char src_path[FS_MAX_PATH_LEN] = {0};
        char dest_path[FS_MAX_PATH_LEN] = {0};
        
        const char* cmd_ptr = command + 3;
        int src_len = 0;
        
        // Skip spaces
        while (*cmd_ptr == ' ' && *cmd_ptr != '\0') {
            cmd_ptr++;
        }
        
        // Extract source path
        while (*cmd_ptr != ' ' && *cmd_ptr != '\0' && src_len < FS_MAX_PATH_LEN - 1) {
            src_path[src_len++] = *cmd_ptr++;
        }
        
        // Skip spaces
        while (*cmd_ptr == ' ' && *cmd_ptr != '\0') {
            cmd_ptr++;
        }
        
        // Extract destination path
        strncpy(dest_path, cmd_ptr, FS_MAX_PATH_LEN);
        
        if (src_len > 0 && strlen(dest_path) > 0) {
            int result = fs_copy(src_path, dest_path);
            
            if (result == 0) {
                k_print_string("File copied successfully", WHITE_ON_BLACK, cursor_row, cursor_col);
            } else {
                k_print_string("Error: Could not copy file", WHITE_ON_BLACK, cursor_row, cursor_col);
            }
        } else {
            k_print_string("Usage: cp <source> <destination>", WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (strncmp(command, "mv ", 3) == 0) {
        cursor_row++;
        cursor_col = 0;
        
        // Parse source and destination paths (same as cp)
        char src_path[FS_MAX_PATH_LEN] = {0};
        char dest_path[FS_MAX_PATH_LEN] = {0};
        
        const char* cmd_ptr = command + 3;
        int src_len = 0;
        
        // Skip spaces
        while (*cmd_ptr == ' ' && *cmd_ptr != '\0') {
            cmd_ptr++;
        }
        
        // Extract source path
        while (*cmd_ptr != ' ' && *cmd_ptr != '\0' && src_len < FS_MAX_PATH_LEN - 1) {
            src_path[src_len++] = *cmd_ptr++;
        }
        
        // Skip spaces
        while (*cmd_ptr == ' ' && *cmd_ptr != '\0') {
            cmd_ptr++;
        }
        
        // Extract destination path
        strncpy(dest_path, cmd_ptr, FS_MAX_PATH_LEN);
        
        if (src_len > 0 && strlen(dest_path) > 0) {
            int result = fs_move(src_path, dest_path);
            
            if (result == 0) {
                k_print_string("File moved successfully", WHITE_ON_BLACK, cursor_row, cursor_col);
            } else {
                k_print_string("Error: Could not move file", WHITE_ON_BLACK, cursor_row, cursor_col);
            }
        } else {
            k_print_string("Usage: mv <source> <destination>", WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (strncmp(command, "find ", 5) == 0) {
        cursor_row++;
        cursor_col = 0;
        
        const char* pattern = command + 5;
        char results[1024];
        
        int found = fs_find_by_name(pattern, results, sizeof(results));
        
        if (found >= 0) {
            k_print_string("Search results:", WHITE_ON_BLACK, cursor_row, cursor_col);
            cursor_row++;
            k_print_string(results, WHITE_ON_BLACK, cursor_row, cursor_col);
        } else {
            k_print_string("Error: Search failed", WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (strcmp(command, "tree") == 0 || strncmp(command, "tree ", 5) == 0) {
        cursor_row++;
        cursor_col = 0;
        
        // Get path parameter if any
        const char* path = "/";
        if (strlen(command) > 5) {
            path = command + 5;
        }
        
        char buffer[2048] = {0};
        fs_tree(path, buffer, sizeof(buffer), 0);
        
        k_print_string("Directory tree:", WHITE_ON_BLACK, cursor_row, cursor_col);
        cursor_row++;
        k_print_string(buffer, WHITE_ON_BLACK, cursor_row, cursor_col);
    }
    else if (strncmp(command, "stat ", 5) == 0) {
        cursor_row++;
        cursor_col = 0;
        
        const char* path = command + 5;
        fs_entry_t info;
        
        int result = fs_stat(path, &info);
        
        if (result == 0) {
            k_print_string("File information:", WHITE_ON_BLACK, cursor_row, cursor_col);
            cursor_row++;
            
            k_print_string("  Name: ", WHITE_ON_BLACK, cursor_row, cursor_col);
            cursor_col = 8;
            k_print_string(info.name, WHITE_ON_BLACK, cursor_row, cursor_col);
            cursor_row++;
            cursor_col = 0;
            
            k_print_string("  Type: ", WHITE_ON_BLACK, cursor_row, cursor_col);
            cursor_col = 8;
            k_print_string((info.type == FS_TYPE_DIRECTORY) ? "Directory" : "File", WHITE_ON_BLACK, cursor_row, cursor_col);
            cursor_row++;
            cursor_col = 0;
            
            k_print_string("  Size: ", WHITE_ON_BLACK, cursor_row, cursor_col);
            // Note: We need a way to print numbers, for now just show "N bytes"
            cursor_col = 8;
            k_print_string("Size info sent to serial", WHITE_ON_BLACK, cursor_row, cursor_col);
            
            // Send detailed info to serial
            debug_print("File size: ");
            serial_write_dec(info.size);
            debug_println(" bytes");
        } else {
            k_print_string("Error: Could not get file information", WHITE_ON_BLACK, cursor_row, cursor_col);
        }
    }
    else if (command_length > 0) {
        cursor_row++;
        cursor_col = 0;
        k_print_string("Unknown command: ", WHITE_ON_BLACK, cursor_row, cursor_col);
        cursor_col = 17;
        k_print_string(command, WHITE_ON_BLACK, cursor_row, cursor_col);
    }
}

// Clear the command buffer
void clear_command_buffer() {
    for (int i = 0; i < MAX_COMMAND_LENGTH; i++) {
        current_command[i] = '\0';
    }
    command_length = 0;
}

// Process a character input and display it in the terminal
void terminal_putchar(char c) {
    // Remove current cursor
    k_print_char(' ', WHITE_ON_BLACK, cursor_row, cursor_col);
    
    // Handle special characters
    if (c == '\n') {
        // New line - process command
        current_command[command_length] = '\0';
        process_command(current_command);
        clear_command_buffer();
        return;
    } 
    else if (c == '\b') {
        // Backspace - remove character from buffer
        if (cursor_col > sizeof(SHELL_PROMPT) - 1) {
            cursor_col--;
            k_print_char(' ', WHITE_ON_BLACK, cursor_row, cursor_col);
            command_length--;
            current_command[command_length] = '\0';
        }
    }
    else if (c == '\t') {
        // Tab - for command completion in future
    }
    else {
        // Regular character - add to buffer if there's room
        if (command_length < MAX_COMMAND_LENGTH - 1 && cursor_col < COLUMNS - 1) {
            current_command[command_length++] = c;
            k_print_char(c, WHITE_ON_BLACK, cursor_row, cursor_col);
            cursor_col++;
        }
    }
    
    // Update cursor position
    update_cursor();
}

// Handle shell input including control keys and history
void shell_handle_input(char c) {
    terminal_putchar(c);
}

// Main kernel function
void kernel_main() {
    // Clear screen
    clear_screen();
    
    // Initialize serial port
    serial_init();
    serial_write_string("Serial port initialized");

    k_print_string("*** Kernel Loaded Successfully! ***", WHITE_ON_BLACK, 0, 0);
    serial_write_string("Kernel loaded successfully!");
    
    // Initialize interrupt system
    serial_write_string("Initializing interrupt system...");
    idt_init();     // Set up and load the IDT
    serial_write_string("Initializing service routines...");
    isr_init();     // Set up interrupt service routines
    
    // Initialize keyboard
    keyboard_init();
    serial_write_string("Keyboard initialized");

    // Enable interrupts (using inline assembly)
#ifdef __GNUC__
    __asm__ __volatile__("sti");
#else
    // Alternative method for non-GCC compilers
    #if defined(_MSC_VER)
    __asm { sti }
    #else
    // Just a placeholder, not actually enabling interrupts
    #endif
#endif
    serial_write_string("Interrupts enabled");

    // Initialize C standard library
    libc_init();
    
    // Initialize filesystem
    fs_init();
    serial_write_string("Filesystem initialized");

    // Keyboard input demo message
    k_print_string("aceOS Shell v0.1", WHITE_ON_BLACK, 0, 0);
    k_print_string("Type 'help' for available commands", WHITE_ON_BLACK, 1, 0);

    // Initialize shell
    cursor_row = 3;
    cursor_col = 0;
    clear_command_buffer();
    print_shell_prompt();
    
    // Main kernel loop
    while (1) {
        // Check for keyboard input (non-blocking)
        if (!keyboard_buffer_empty()) {
            char c = keyboard_getchar();
            shell_handle_input(c);
        }
    }
}