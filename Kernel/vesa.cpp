#include <shitio.h>
#include <alloc.h>
#include <vesa.h>
#include <stivale.h>
#include <memory.h>

using namespace standardout;

uint16_t framebuffer_height, framebuffer_width, framebuffer_pitch, framebuffer_bpp;
uint32_t *framebuffer;

void init_graphics(stivale_info_t *boot_info)
{
    framebuffer = (uint32_t*)boot_info->framebuffer_addr;
    framebuffer_height = boot_info->framebuffer_height;
    framebuffer_width = boot_info->framebuffer_width;
    framebuffer_pitch = boot_info->framebuffer_pitch;
    framebuffer_bpp = boot_info->framebuffer_bpp;

    t_print("VBE VESA init:\n\n");
    t_print("Framebuffer address: %x\n",  framebuffer);
    t_print("Framebuffer Height: %d\n",  framebuffer_height);
    t_print("Framebuffer Width: %d\n",  framebuffer_width);
    t_print("Framebuffer pitch: %d\n",  framebuffer_pitch);
    t_print("Framebuffer BPP: %d\n\n",  framebuffer_bpp);
}

void show_vesa_state()
{
    k_print("VBE VESA init:\n\n");
    k_print("\tFramebuffer address: %x\n",  framebuffer);
    k_print("\tFramebuffer Height: %d\n",  framebuffer_height);
    k_print("\tFramebuffer Width: %d\n",  framebuffer_width);
    k_print("\tFramebuffer pitch: %d\n",  framebuffer_pitch);
    k_print("\tFramebuffer BPP: %d\n\n",  framebuffer_bpp);
}

void set_pixel(uint16_t x, uint16_t y, uint32_t colour)
{
    *(volatile uint32_t*)((uint64_t)framebuffer + ((y * framebuffer_pitch) + (x * framebuffer_bpp / 8))) = colour; // bpp / 8 = bytes per pixel
}

uint32_t grab_colour(uint16_t x, uint16_t y)
{
    return *(volatile uint32_t*)((uint64_t)framebuffer + ((y * framebuffer_pitch) + (x * framebuffer_bpp / 8)));
}

volatile uint32_t *grab_framebuffer_addr(uint16_t x, uint16_t y)
{
    return (volatile uint32_t*)((uint64_t)framebuffer + ((y * framebuffer_pitch) + (x * framebuffer_bpp / 8)));
}

void draw_hline(uint16_t x, uint16_t x1, uint16_t y, uint32_t colour)
{
    for(int i = x; i < x1 + 1; i++)
        set_pixel(i, y, colour);
}

void draw_vline(uint16_t y, uint16_t y1, uint16_t x, uint32_t colour)
{
    for(int i = y; i < y1 + 1; i++)
        set_pixel(x, i, colour);
}

void draw_rechtangle(vec2 vec, uint32_t colour)
{
    for(int i = vec[0][1]; i < vec[3][1] + 1; i++)
        draw_hline(vec[0][0], vec[3][0], i, colour);
}

window::window(vec2 vec, uint32_t colour, uint32_t border_colour)
{
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 2; j++)
            window_borders[i][j] = vec[i][j];

    draw_rechtangle(vec, colour);
}

void window::fill_window(uint32_t colour, uint32_t border_colour)
{
    draw_rechtangle(window_borders, colour);
}

widget::widget(uint32_t vert[][2], uint32_t rows, uint32_t colour_t, uint32_t border_colour_t)
{
    background_colour = (uint32_t*)malloc(sizeof(uint32_t*));
    colour = colour_t;
    border_colour = border_colour_t;
    draw_widget(vert, rows);
}

void widget::draw_widget(uint32_t vect[][2], uint32_t rows)
{
    for(int i = vect[0][1]; i < vect[3][1] + 1; i++) {
        for(int j = vect[0][0]; j < vect[3][0] + 1; j++) {
            set_pixel(j, i, colour);
        }
    }
}

void widget::move_right(uint32_t vect[][2], uint32_t rows)
{
    for(uint32_t i = 0; i < rows; i++) {
        vect[i][0] += 1;
        set_pixel(vect[i][0] - 1, vect[i][1], background_colour[i]);
    }

    draw_widget(vect, rows);
}

void widget::move_left(uint32_t vect[][2], uint32_t rows)
{
    for(uint32_t i = 0; i < rows; i++) {
        vect[i][0] -= 1;
        set_pixel(vect[i][0] + 1, vect[i][1], background_colour[i]);
    }

    draw_widget(vect, rows);
}

void widget::move_down(uint32_t vect[][2], uint32_t rows)
{
    for(uint32_t i = 0; i < rows; i++) {
        vect[i][1] += 1;
        set_pixel(vect[i][0], vect[i][1] - 1, background_colour[i]);
    }

    draw_widget(vect, rows);
}

void widget::move_up(uint32_t  vect[][2], uint32_t rows)
{
    for(uint32_t i = 0; i < rows; i++) {
        vect[i][1] -= 1;
        set_pixel(vect[i][0], vect[i][1] + 1, background_colour[i]);
    }

    draw_widget(vect, rows);
}

void widget::move(uint32_t vect[][2], uint32_t rows, uint64_t x, uint64_t y)
{

    for(uint32_t i = 0; i < rows; i++) {
        if(vect[i][0] >= y)
            vect[i][0] -= vect[3][0] - y;
        else
            vect[i][0] += y - vect[3][0];

        if(vect[i][1] >= x)
            vect[i][1] += vect[3][1] - x;
        else
            vect[i][1] -= x - vect[3][1];
    }
    draw_widget(vect, 4);
}

void render_char(uint64_t x, uint64_t y, uint32_t fg, uint32_t bg, char c) {
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            if ((font[(uint8_t)c][i] >> j) & 1) {
                uint64_t offset = ((i + y) * framebuffer_pitch) + ((j + x) * 4);
                *(uint32_t*)((uint64_t)framebuffer + offset) = fg ;
            }
            else {
                uint64_t offset = ((i + y) * framebuffer_pitch) + ((j + x) * 4);
                *(uint32_t *)((uint64_t)framebuffer + offset) = bg;
            }
        }
    }
}
