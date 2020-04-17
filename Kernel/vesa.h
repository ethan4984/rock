#pragma once

#include <stivale.h>
#include <stdint.h>

typedef uint16_t pair[2]; /* cords for a pixel on a screen : [0] = x, [1] = y */

typedef uint16_t vec2[4][2]; /* cords for a rechtangle */

void init_graphics(stivale_info_t *boot_info);

void set_pixel(uint16_t x, uint16_t y, uint32_t colour);

void draw_hline(uint16_t x, uint16_t x1, uint16_t y, uint32_t colour);

void draw_vline(uint16_t y, uint16_t y1, uint16_t x, uint32_t colour);

void draw_rechtangle(vec2 vertices, uint32_t colour);

class window
{
    public:
        window(vec2 vec, uint32_t colour, uint32_t border_colour);

        void fill_window(uint32_t colour, uint32_t border_colour);
    private:
        vec2 window_borders;
};

class widget
{
    public:
        widget(uint32_t vect[][2], uint32_t rows, uint32_t colour, uint32_t border_colour);

        void draw_widget(uint32_t vect[][2], uint32_t rows);

        void move_right(uint32_t vect[][2], uint32_t rows);

        void move_left(uint32_t vect[][2], uint32_t rows);

        void move_up(uint32_t vect[][2], uint32_t rows);

        void move_down(uint32_t vect[][2], uint32_t rows);
    private:
        uint64_t number_of_pixels = 0;

        uint32_t colour, border_colour;

        uint32_t *background_colour;
};
