#include <mm/vmm.h>
#include <mm/pmm.h>
#include <font.h>
#include <graphics.h>

uint32_t *framebuffer, fb_height, fb_width, fb_pitch, fb_bpp, *double_buffer, fb_size;

void init_graphics(stivale_t *stivale) {
    framebuffer = (uint32_t*)(stivale->fb_addr + HIGH_VMA);
    fb_height = stivale->fb_height;
    fb_width = stivale->fb_width;
    fb_bpp = stivale->fb_bpp;
    fb_pitch = stivale->fb_pitch;
    fb_size = fb_height * fb_pitch;

    double_buffer = (uint32_t*)(pmm_calloc(ROUNDUP(fb_height * fb_width * (fb_bpp / 8), 0x1000)) + HIGH_VMA); 
    clear_screen(0xffffff);
}

inline void set_pixel(uint32_t x, uint32_t y, uint32_t colour) {
    *(volatile uint32_t*)(((uint64_t)framebuffer) + ((y * fb_pitch) + (x * fb_bpp / 8))) = colour;
    *(volatile uint32_t*)(((uint64_t)double_buffer) + ((y * fb_pitch) + (x * fb_bpp / 8))) = colour;
}

inline uint32_t get_pixel(uint32_t x, uint32_t y) {
    return *(volatile uint32_t*)(((uint64_t)double_buffer) + ((y * fb_pitch) + (x * fb_bpp / 8)));
}

void scroll(uint32_t bg, uint32_t row_shift) {
    memcpy64((uint64_t*)framebuffer, (uint64_t*)((uint64_t)double_buffer + (fb_pitch * row_shift)), (fb_size - (fb_pitch * row_shift)) / 8);
    memcpy64((uint64_t*)double_buffer, (uint64_t*)((uint64_t)double_buffer + (fb_pitch * row_shift)), (fb_size - (fb_pitch * row_shift)) / 8);

    memset32((uint32_t*)((uint64_t)framebuffer + fb_size - (fb_pitch * row_shift)), bg, (fb_pitch * row_shift) / 4); 
    memset32((uint32_t*)((uint64_t)double_buffer + fb_size - (fb_pitch * row_shift)), bg, (fb_pitch * row_shift) / 4); 
}

void clear_screen(uint32_t bg) {
    memset32(framebuffer, bg, fb_size / 4);
    memset32(double_buffer, bg, fb_size / 4);
}

void render_char(uint32_t x, uint32_t y, uint32_t fg, char c) {
    uint16_t offset = ((uint8_t)c - 0x20) * 16;
    for(uint8_t i = 0, i_cnt = 8; i < 8 && i_cnt > 0; i++, i_cnt--) {
        for(uint8_t j = 0; j < 16; j++) {
            if((font_bitmap[offset + j] >> i) & 1)
                set_pixel(x + i_cnt, y + j, fg);
        }
    }
}

int draw_shape(shape_t *shape) {
    for(uint32_t y = 0; y < shape->height; y++) {
        for(uint32_t x = 0; x < shape->width; x++) { 
            if(shape->colour_buffer[y * shape->width + x] == UNUSED_PIXEL)
                continue; 
            shape->backbuffer[y * shape->width + x] = get_pixel(shape->x + x, shape->y + y);
            set_pixel(shape->x + x, shape->y + y, shape->colour_buffer[y * shape->width + x]);
        }
    }
    return 0;
}

int redraw_shape(shape_t *shape, uint32_t new_x, uint32_t new_y) {
    for(uint32_t x = 0; x < shape->width; x++) {
        for(uint32_t y = 0; y < shape->height; y++) {
            if(shape->colour_buffer[y * shape->width + x] == UNUSED_PIXEL)
                continue; 
            set_pixel(shape->x + x, shape->y + y, shape->backbuffer[y * shape->width + x]); 
        }
    }

    shape->x = new_x; 
    shape->y = new_y;

    return draw_shape(shape);
}

int draw_window(window_t *win) {
    for(uint32_t i = win->x; i < win->x + win->width; i++) {
        for(uint32_t j = win->y; j < win->y + win->height; j++) {
            set_pixel(i, j, win->background);
        }
    }
    
    for(uint32_t i = 0; i < win->shape_cnt; i++) {
        draw_shape(&win->shapes[i]);
    }

    return 0;
}
