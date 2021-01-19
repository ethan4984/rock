#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <stivale.h>

#define UNUSED_PIXEL 0x69696969

typedef struct {
    uint32_t *colour_buffer;
    uint32_t *backbuffer;
    uint32_t x;
    uint32_t y;
    uint32_t height;
    uint32_t width;
} shape_t;

typedef struct {
    shape_t *shapes;
    uint32_t shape_cnt;
    uint32_t x;
    uint32_t y;
    uint32_t height;
    uint32_t width;
    uint32_t background;
} window_t;

extern uint32_t *framebuffer, fb_bpp, fb_height, fb_width, fb_pitch;

void init_graphics(stivale_t *stivale);

void set_pixel(uint32_t x, uint32_t y, uint32_t colour);

uint32_t get_pixel(uint32_t x, uint32_t y);

void render_char(uint32_t x, uint32_t y, uint32_t fg, char c);

void scroll(uint32_t bg, uint32_t row_shift);

void clear_screen(uint32_t bg);

int draw_shape(shape_t *shape); 

int redraw_shape(shape_t *shape, uint32_t new_x, uint32_t new_y);

int draw_window(window_t *win);

int redraw_window(window_t *win, uint32_t new_x, uint32_t new_y);

int destroy_window(window_t *win);

#endif
