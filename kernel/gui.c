#include "../include/graphics.h"
#include "../include/process.h"
#include "../include/keyboard.h"
#include "../include/memory.h"
#include "../include/libc/string.h"
#include "../include/libc/stdlib.h"
#include "../include/serial.h"
#include "../include/utils.h"

// GUI constants
#define MAX_WINDOWS 16
#define WINDOW_TITLE_HEIGHT 20
#define WINDOW_BORDER_WIDTH 2
#define DESKTOP_COLOR COLOR_LIGHT_GRAY
#define WINDOW_BORDER_COLOR COLOR_DARK_GRAY
#define WINDOW_TITLE_COLOR COLOR_BLUE
#define WINDOW_TITLE_TEXT_COLOR COLOR_WHITE
#define WINDOW_CONTENT_COLOR COLOR_WHITE

// Window states
#define WINDOW_STATE_NORMAL   1
#define WINDOW_STATE_MINIMIZED 2
#define WINDOW_STATE_MAXIMIZED 3
#define WINDOW_STATE_CLOSED   4

// Window structure
typedef struct window {
    uint32_t id;
    char title[32];
    rect_t bounds;
    uint32_t state;
    uint8_t* content_buffer;
    struct window* next;
    process_t* owner_process;
    int visible;
    int z_order;
} window_t;

// GUI system state
typedef struct gui_state {
    window_t* windows[MAX_WINDOWS];
    window_t* active_window;
    int window_count;
    int next_window_id;
    int desktop_initialized;
    point_t mouse_pos;
    int mouse_buttons;
} gui_state_t;

static gui_state_t gui_state;

// Function prototypes
void gui_init(void);
void gui_main_loop(void);
void gui_draw_desktop(void);
void gui_draw_window(window_t* window);
void gui_draw_all_windows(void);
window_t* gui_create_window(const char* title, int x, int y, int width, int height);
void gui_destroy_window(window_t* window);
void gui_handle_keyboard_input(void);
void gui_handle_mouse_input(void);
void gui_update_screen(void);

// GUI process entry point
void gui_process_main(void) {
    serial_write_string("GUI Process: Starting GUI process...\n");
    
    // Initialize GUI subsystem
    gui_init();
    
    // Switch to graphics mode
    graphics_set_mode_13h();
    graphics_set_default_palette();
    
    // Draw initial desktop
    gui_draw_desktop();
    
    // Create a test window
    window_t* test_window = gui_create_window("aceOS Desktop", 50, 50, 200, 150);
    if (test_window) {
        // Draw some content in the test window
        graphics_fill_rect(test_window->bounds.x + WINDOW_BORDER_WIDTH, 
                          test_window->bounds.y + WINDOW_TITLE_HEIGHT, 
                          test_window->bounds.width - 2 * WINDOW_BORDER_WIDTH, 
                          test_window->bounds.height - WINDOW_TITLE_HEIGHT - WINDOW_BORDER_WIDTH,
                          WINDOW_CONTENT_COLOR);
        
        graphics_draw_string(test_window->bounds.x + 10, 
                           test_window->bounds.y + WINDOW_TITLE_HEIGHT + 10,
                           "Welcome to aceOS!", COLOR_BLACK);
        
        graphics_draw_string(test_window->bounds.x + 10, 
                           test_window->bounds.y + WINDOW_TITLE_HEIGHT + 25,
                           "GUI System Active", COLOR_BLACK);
    }
    
    serial_write_string("GUI Process: Desktop initialized\n");
    
    // Main GUI event loop
    gui_main_loop();
}

// Initialize GUI subsystem
void gui_init(void) {
    serial_write_string("GUI: Initializing GUI subsystem...\n");
    
    // Clear GUI state
    memset(&gui_state, 0, sizeof(gui_state_t));
    
    gui_state.next_window_id = 1;
    gui_state.mouse_pos.x = VGA_WIDTH / 2;
    gui_state.mouse_pos.y = VGA_HEIGHT / 2;
    gui_state.desktop_initialized = 1;
    
    serial_write_string("GUI: GUI subsystem initialized\n");
}

// Global flag to control GUI main loop
static int gui_running = 1;

// Main GUI event loop
void gui_main_loop(void) {
    gui_running = 1;
    
    while (gui_running) {
        // Handle keyboard input
        gui_handle_keyboard_input();
        
        // Handle mouse input (placeholder for now)
        gui_handle_mouse_input();
        
        // Update screen if needed
        gui_update_screen();
        
        // Yield to other processes
        scheduler_yield();
        
        // Brief pause to prevent consuming all CPU
        for (volatile int i = 0; i < 1000; i++);
    }
}

// Draw the desktop background
void gui_draw_desktop(void) {
    // Clear screen with desktop color
    graphics_clear_screen(DESKTOP_COLOR);
    
    // Draw desktop pattern (simple grid)
    for (int x = 0; x < VGA_WIDTH; x += 20) {
        graphics_draw_line(x, 0, x, VGA_HEIGHT - 1, COLOR_DARK_GRAY);
    }
    
    for (int y = 0; y < VGA_HEIGHT; y += 20) {
        graphics_draw_line(0, y, VGA_WIDTH - 1, y, COLOR_DARK_GRAY);
    }
    
    // Draw title bar at top
    graphics_fill_rect(0, 0, VGA_WIDTH, 20, COLOR_BLUE);
    graphics_draw_string(5, 5, "aceOS Desktop Environment", COLOR_WHITE);
    
    // Draw system info in corner
    graphics_draw_string(VGA_WIDTH - 100, 5, "GUI Active", COLOR_YELLOW);
}

// Draw a single window
void gui_draw_window(window_t* window) {
    if (!window || !window->visible) {
        return;
    }
    
    // Draw window border
    graphics_draw_rect(window->bounds.x, window->bounds.y, 
                      window->bounds.width, window->bounds.height, 
                      WINDOW_BORDER_COLOR);
    
    // Draw title bar
    graphics_fill_rect(window->bounds.x + 1, window->bounds.y + 1,
                      window->bounds.width - 2, WINDOW_TITLE_HEIGHT - 1,
                      WINDOW_TITLE_COLOR);
    
    // Draw title text
    graphics_draw_string(window->bounds.x + 5, window->bounds.y + 5,
                        window->title, WINDOW_TITLE_TEXT_COLOR);
    
    // Draw close button (simple X)
    int close_x = window->bounds.x + window->bounds.width - 15;
    int close_y = window->bounds.y + 5;
    graphics_draw_string(close_x, close_y, "X", COLOR_RED);
    
    // Draw window content area
    graphics_fill_rect(window->bounds.x + WINDOW_BORDER_WIDTH,
                      window->bounds.y + WINDOW_TITLE_HEIGHT,
                      window->bounds.width - 2 * WINDOW_BORDER_WIDTH,
                      window->bounds.height - WINDOW_TITLE_HEIGHT - WINDOW_BORDER_WIDTH,
                      WINDOW_CONTENT_COLOR);
}

// Draw all windows in z-order
void gui_draw_all_windows(void) {
    // Simple drawing - just draw all visible windows
    for (int i = 0; i < gui_state.window_count; i++) {
        if (gui_state.windows[i]) {
            gui_draw_window(gui_state.windows[i]);
        }
    }
}

// Create a new window
window_t* gui_create_window(const char* title, int x, int y, int width, int height) {
    if (gui_state.window_count >= MAX_WINDOWS) {
        serial_write_string("GUI: Maximum windows reached\n");
        return NULL;
    }
    
    // Allocate window structure
    window_t* window = (window_t*)malloc(sizeof(window_t));
    if (!window) {
        serial_write_string("GUI: Failed to allocate window memory\n");
        return NULL;
    }
    
    // Initialize window
    window->id = gui_state.next_window_id++;
    strncpy(window->title, title, 31);
    window->title[31] = '\0';
    window->bounds.x = x;
    window->bounds.y = y;
    window->bounds.width = width;
    window->bounds.height = height;
    window->state = WINDOW_STATE_NORMAL;
    window->visible = 1;
    window->z_order = gui_state.window_count;
    window->owner_process = process_get_current();
    window->next = NULL;
    
    // Add to window list
    gui_state.windows[gui_state.window_count] = window;
    gui_state.window_count++;
    
    // Set as active window
    gui_state.active_window = window;
    
    serial_write_string("GUI: Created window: ");
    serial_write_string(title);
    serial_write_string("\n");
    
    return window;
}

// Destroy a window
void gui_destroy_window(window_t* window) {
    if (!window) {
        return;
    }
    
    // Remove from window list
    for (int i = 0; i < gui_state.window_count; i++) {
        if (gui_state.windows[i] == window) {
            // Shift remaining windows
            for (int j = i; j < gui_state.window_count - 1; j++) {
                gui_state.windows[j] = gui_state.windows[j + 1];
            }
            gui_state.windows[gui_state.window_count - 1] = NULL;
            gui_state.window_count--;
            break;
        }
    }
    
    // Update active window
    if (gui_state.active_window == window) {
        gui_state.active_window = (gui_state.window_count > 0) ? 
                                  gui_state.windows[gui_state.window_count - 1] : NULL;
    }
    
    // Free window memory
    if (window->content_buffer) {
        free(window->content_buffer);
    }
    free(window);
    
    serial_write_string("GUI: Window destroyed\n");
}

// Handle keyboard input
void gui_handle_keyboard_input(void) {
    if (!keyboard_buffer_empty()) {
        char c = keyboard_getchar();
        
        // Debug: Show what key was received
        serial_write_string("GUI: Key received: ");
        char buffer[8];
        itoa((int)c, buffer, 10);
        serial_write_string(buffer);
        serial_write_string(" (0x");
        itoa((int)c, buffer, 16);
        serial_write_string(buffer);
        serial_write_string(")\n");
        
        // Handle special keys
        switch (c) {
            case 27: // ESC - exit GUI
                serial_write_string("GUI: ESC pressed, exiting to text mode\n");
                graphics_set_text_mode();
                gui_running = 0; // Exit the main loop
                break;
                
            case 'q':
            case 'Q':
                // Close active window
                if (gui_state.active_window) {
                    gui_destroy_window(gui_state.active_window);
                    gui_draw_desktop();
                    gui_draw_all_windows();
                }
                break;
                
            case 'n':
            case 'N':
                // Create new window
                {
                    static int window_counter = 1;
                    char title[32];
                    strcpy(title, "Window ");
                    itoa(window_counter++, title + 7, 10);
                    
                    int x = 30 + (window_counter * 20) % 100;
                    int y = 40 + (window_counter * 15) % 80;
                    
                    window_t* new_window = gui_create_window(title, x, y, 180, 120);
                    if (new_window) {
                        gui_draw_desktop();
                        gui_draw_all_windows();
                    }
                }
                break;
                
            default:
                // Forward to active window (placeholder)
                break;
        }
    }
}

// Handle mouse input (placeholder)
void gui_handle_mouse_input(void) {
    // Mouse support would be implemented here
    // For now, this is just a placeholder
}

// Update screen
void gui_update_screen(void) {
    // For now, we don't have double buffering, so this is a placeholder
    // In a more advanced implementation, this would swap buffers
} 