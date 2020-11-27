#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <stivale.h>

extern uint32_t *framebuffer, fb_bpp, fb_height, fb_width, fb_pitch;

void init_graphics(stivale_t *stivale);

void set_pixel(uint32_t x, uint32_t y, uint32_t colour);

uint32_t get_pixel(uint32_t x, uint32_t y);

void render_char(uint32_t x, uint32_t y, uint32_t fg, char c);

void scroll(uint32_t bg, uint32_t row_shift);

void clear_screen(uint32_t bg);

#endif
