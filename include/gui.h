#ifndef GUI_H
#define GUI_H

#include "graphics.h"
#include "process.h"
#include "types.h"

// GUI constants
#define MAX_WINDOWS 16
#define WINDOW_TITLE_HEIGHT 20
#define WINDOW_BORDER_WIDTH 2

// Window states
#define WINDOW_STATE_NORMAL   1
#define WINDOW_STATE_MINIMIZED 2
#define WINDOW_STATE_MAXIMIZED 3
#define WINDOW_STATE_CLOSED   4

// Forward declarations
typedef struct window window_t;
typedef struct gui_state gui_state_t;

// Window structure
struct window {
    uint32_t id;
    char title[32];
    rect_t bounds;
    uint32_t state;
    uint8_t* content_buffer;
    struct window* next;
    process_t* owner_process;
    int visible;
    int z_order;
};

// GUI system state
struct gui_state {
    window_t* windows[MAX_WINDOWS];
    window_t* active_window;
    int window_count;
    int next_window_id;
    int desktop_initialized;
    point_t mouse_pos;
    int mouse_buttons;
};

// Function prototypes
// GUI process entry point
void gui_process_main(void);

// Core GUI functions
void gui_init(void);
void gui_main_loop(void);
void gui_draw_desktop(void);
void gui_draw_window(window_t* window);
void gui_draw_all_windows(void);
void gui_update_screen(void);

// Window management
window_t* gui_create_window(const char* title, int x, int y, int width, int height);
void gui_destroy_window(window_t* window);

// Input handling
void gui_handle_keyboard_input(void);
void gui_handle_mouse_input(void);

#endif // GUI_H 