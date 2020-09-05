#include <kernel/mm/virtualPageManager.h>
#include <kernel/drivers/font.h>
#include <kernel/drivers/vesa.h>
#include <kernel/stivale.h>

namespace kernel {

uint16_t height, width, pitch, bpp;
uint32_t framebuffer;

void setPixel(uint16_t x, uint16_t y, uint32_t colour) {
    *(volatile uint32_t*)(((uint64_t)framebuffer + HIGH_VMA) + ((y * pitch) + (x * bpp / 8))) = colour; 
}

uint32_t grabColour(uint16_t x, uint16_t y) {
    return *(volatile uint32_t*)(((uint64_t)framebuffer + HIGH_VMA) + ((y * pitch) + (x * bpp / 8)));
}

void initVESA(stivaleInfo_t *stivale) {
    framebuffer = stivale->framebufferAddr; 
    height = stivale->framebufferHeight;
    width = stivale->framebufferWidth;
    pitch = stivale->framebufferPitch;
    bpp = stivale->framebufferBpp;
}

void renderChar(uint64_t x, uint64_t y, uint32_t fg, uint32_t bg, char c) {
    for(uint8_t i = 0; i < 8; i++) {
        for(uint8_t j = 0; j < 8; j++) {
            if((font[(uint8_t)c][i] >> j) & 1) {
                setPixel(j + x, y + i, fg);
            }
            else {
                setPixel(j + x, y + i, bg);
            }
        }
    }
}

}
