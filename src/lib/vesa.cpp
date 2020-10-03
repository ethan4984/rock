#include <kernel/mm/virtualPageManager.h>
#include <kernel/mm/kHeap.h>
#include <kernel/stivale.h>
#include <lib/output.h>
#include <lib/font.h>
#include <lib/vesa.h>

namespace vesa {

void setPixel(uint16_t x, uint16_t y, uint32_t colour) {
    *(volatile uint32_t*)(((uint64_t)framebuffer + HIGH_VMA) + ((y * pitch) + (x * bpp / 8))) = colour; 
}

uint32_t grabColour(uint16_t x, uint16_t y) {
    return *(volatile uint32_t*)(((uint64_t)framebuffer + HIGH_VMA) + ((y * pitch) + (x * bpp / 8)));
}

void init(stivaleInfo_t *stivale) {
    framebuffer = stivale->framebufferAddr; 
    height = stivale->framebufferHeight;
    width = stivale->framebufferWidth;
    pitch = stivale->framebufferPitch;
    bpp = stivale->framebufferBpp;
}

void renderChar(uint64_t x, uint64_t y, uint32_t fg, char c) {
    for(uint8_t i = 0; i < 8; i++) {
        for(uint8_t j = 0; j < 8; j++) {
            if((font[(uint8_t)c][i] >> j) & 1) {
                setPixel(j + x, y + i, fg);
            }
        }
    }
}

void blk::blkDraw() {
    uint32_t cnt = 0;
    for(uint32_t i = y; i < y + VESA_BLOCK_SIZE; i++) {
        for(uint32_t j = x; j < x + VESA_BLOCK_SIZE; j++) {
            backgroundBuffer[cnt++] = grabColour(j, i);
            setPixel(j, i, colour); 
        }
    }
}

void blk::blkRedraw(uint32_t newX, uint32_t newY) {
    uint32_t cnt = 0;
    for(uint32_t i = y; i < y + VESA_BLOCK_SIZE; i++) {
        for(uint32_t j = x; j < x + VESA_BLOCK_SIZE; j++) {
            setPixel(j, i, backgroundBuffer[cnt++]); 
        }
    }

    x = newX; y = newY;
    blkDraw();
}

void blk::blkChangeColour(uint32_t newColour) {
    colour = newColour; 
    blkDraw();
}

blkGrp::blkGrp(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour) : x(x),
    y(y), xCnt(xCnt), yCnt(yCnt), colour(colour) {
    uint32_t cnt = 0;

    blocks = new blk[xCnt * yCnt];
    for(uint32_t i = 0; i < yCnt; i++) {
        for(uint32_t j = 0; j < xCnt; j++, cnt++) {
            blocks[cnt] = blk(j * 8 + x, i * 8 + y, colour);
        }
    }

    draw();
}

void blkGrp::draw() {
    for(uint32_t j = 0; j < xCnt * yCnt; j++) {
        blocks[j].blkDraw();
    } 
}

void blkGrp::redraw(uint32_t newX, uint32_t newY) {
    x = newX; y = newY;
    uint32_t cnt = 0; 
    for(uint32_t i = 0; i < yCnt; i++) {
        for(uint32_t j = 0; j < xCnt; j++, cnt++) {
            blocks[cnt].blkRedraw(j * 8 + x, i * 8 + y);
        }
    }
}

shape::shape(uint32_t x, uint32_t y, uint8_t *foreground, uint32_t xCnt, uint32_t yCnt, uint32_t colour) : x(x), y(y), xCnt(xCnt), yCnt(yCnt), colour(colour), foreground(foreground) {
    background = new uint32_t[xCnt * yCnt];
    draw();
}

void shape::draw() {
    uint32_t cnt = 0;

    for(uint32_t i = y; i < y + yCnt; i++) {
        for(uint32_t j = x; j < x + xCnt; j++, cnt++) {
            if(foreground[cnt] == 1) {
                background[cnt] = grabColour(j, i);
                setPixel(j, i, colour); 
            } else if(foreground[cnt] == 2) {
                background[cnt] = grabColour(j, i);
                setPixel(j, i, 0); 
            }
        }
    }
}

void shape::redraw(uint32_t newX, uint32_t newY) {
    uint32_t cnt = 0;
    for(uint32_t i = y; i < y + yCnt; i++) { 
        for(uint32_t j = x; j < x + xCnt; j++, cnt++) { 
            if(foreground[cnt] != 0) {
                setPixel(j, i, background[cnt]);
            }
        }
    }
    x = newX; y = newY;
    draw();
}

}
