#include <font.h>
#include <stivale.h>
#include <mm/vmm.h>
#include <memutils.h>
#include <mm/pmm.h>

static uint32_t *framebuffer, fb_height, fb_width, fb_pitch, fb_bpp, *double_buffer, fb_size;

void set_pixel(uint32_t x, uint32_t y, uint32_t colour) {
    *(volatile uint32_t*)(((uint64_t)framebuffer + HIGH_VMA) + ((y * fb_pitch) + (x * fb_bpp / 8))) = colour;
    *(volatile uint32_t*)(((uint64_t)double_buffer) + ((y * fb_pitch) + (x * fb_bpp / 8))) = colour;
}

uint32_t get_pixel(uint32_t x, uint32_t y) {
    return *(volatile uint32_t*)(((uint64_t)double_buffer) + ((y * fb_pitch) + (x * fb_bpp / 8)));
}

void tty_init(struct stivale *stivale) {
    framebuffer = (uint32_t*)stivale->fb_addr;
    fb_height = stivale->fb_height;
    fb_width = stivale->fb_width;
    fb_bpp = stivale->fb_bpp;
    fb_pitch = stivale->fb_pitch;
    fb_size = fb_height * fb_pitch;
    double_buffer = (uint32_t*)(pmm_calloc(DIV_ROUNDUP(fb_height * fb_width * (fb_bpp / 8), 0x1000)) + HIGH_VMA); 
}
