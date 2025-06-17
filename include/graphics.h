#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"

// VGA Mode 13h constants
#define VGA_WIDTH           320
#define VGA_HEIGHT          200
#define VGA_MEMORY          0xA0000
#define VGA_BUFFER_SIZE     (VGA_WIDTH * VGA_HEIGHT)

// VGA registers
#define VGA_MISC_OUTPUT     0x3C2
#define VGA_SEQ_INDEX       0x3C4
#define VGA_SEQ_DATA        0x3C5
#define VGA_CRTC_INDEX      0x3D4
#define VGA_CRTC_DATA       0x3D5
#define VGA_GC_INDEX        0x3CE
#define VGA_GC_DATA         0x3CF
#define VGA_AC_INDEX        0x3C0
#define VGA_AC_WRITE        0x3C0
#define VGA_AC_READ         0x3C1
#define VGA_INPUT_STATUS    0x3DA

// Common colors for 256-color palette
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GRAY    7
#define COLOR_DARK_GRAY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW        14
#define COLOR_WHITE         15

// Point structure
typedef struct {
    int x, y;
} point_t;

// Rectangle structure
typedef struct {
    int x, y, width, height;
} rect_t;

// Graphics state
typedef struct {
    uint8_t* framebuffer;
    int current_mode;
    uint8_t foreground_color;
    uint8_t background_color;
} graphics_state_t;

// Function prototypes
// Mode switching
void graphics_init(void);
void graphics_set_mode_13h(void);
void graphics_set_text_mode(void);

// Basic drawing
void graphics_put_pixel(int x, int y, uint8_t color);
uint8_t graphics_get_pixel(int x, int y);
void graphics_clear_screen(uint8_t color);

// Drawing primitives
void graphics_draw_line(int x1, int y1, int x2, int y2, uint8_t color);
void graphics_draw_rect(int x, int y, int width, int height, uint8_t color);
void graphics_fill_rect(int x, int y, int width, int height, uint8_t color);
void graphics_draw_circle(int cx, int cy, int radius, uint8_t color);
void graphics_fill_circle(int cx, int cy, int radius, uint8_t color);

// Text rendering (simple bitmap font)
void graphics_draw_char(int x, int y, char c, uint8_t color);
void graphics_draw_string(int x, int y, const char* str, uint8_t color);

// Palette management
void graphics_set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void graphics_set_default_palette(void);

// State management
void graphics_set_foreground_color(uint8_t color);
void graphics_set_background_color(uint8_t color);
uint8_t graphics_get_foreground_color(void);
uint8_t graphics_get_background_color(void);

// Buffer operations
void graphics_copy_buffer(uint8_t* src, int src_x, int src_y, int src_width,
                         int dst_x, int dst_y, int width, int height);
void graphics_swap_buffers(void);

#endif // GRAPHICS_H 