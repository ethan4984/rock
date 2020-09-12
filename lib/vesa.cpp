#include <kernel/mm/virtualPageManager.h>
#include <kernel/mm/kHeap.h>
#include <kernel/stivale.h>
#include <lib/output.h>
#include <lib/font.h>
#include <lib/vesa.h>

namespace kernel {

void vesa::setPixel(uint16_t x, uint16_t y, uint32_t colour) {
    *(volatile uint32_t*)(((uint64_t)framebuffer + HIGH_VMA) + ((y * pitch) + (x * bpp / 8))) = colour; 
}

uint32_t vesa::grabColour(uint16_t x, uint16_t y) {
    return *(volatile uint32_t*)(((uint64_t)framebuffer + HIGH_VMA) + ((y * pitch) + (x * bpp / 8)));
}

void vesa::initVESA(stivaleInfo_t *stivale) {
    framebuffer = stivale->framebufferAddr; 
    height = stivale->framebufferHeight;
    width = stivale->framebufferWidth;
    pitch = stivale->framebufferPitch;
    bpp = stivale->framebufferBpp;
}

void vesa::renderChar(uint64_t x, uint64_t y, uint32_t fg, char c) {
    for(uint8_t i = 0; i < 8; i++) {
        for(uint8_t j = 0; j < 8; j++) {
            if((font[(uint8_t)c][i] >> j) & 1) {
                setPixel(j + x, y + i, fg);
            }
        }
    }
}

void VesaBlk::blkDraw() {
    uint32_t cnt = 0;
    kprintDS("[KDEBUG]", "%d %d", x, y);
    for(uint32_t i = x; i < x + VESA_BLOCK_SIZE; i++) {
        for(uint32_t j = y; j < y + VESA_BLOCK_SIZE; j++) {
            backgroundBuffer[cnt++] = vesa.grabColour(i, j);
            vesa.setPixel(i, j, colour); 
        }
    }
}

void VesaBlk::blkRedraw(uint32_t newX, uint32_t newY) {
    kprintDS("[KDEBUG]", "%d %d", newX, newY);

    uint32_t cnt = 0;
    for(uint32_t i = x; i < x + VESA_BLOCK_SIZE; i++) {
        for(uint32_t j = y; j < y + VESA_BLOCK_SIZE; j++) {
            vesa.setPixel(i, j, backgroundBuffer[cnt++]); 
        }
    }

    x = newY; y = newY;
    blkDraw();
}

void VesaBlk::blkChangeColour(uint32_t newColour) {
    colour = newColour; 
    blkDraw();
}

VesaBlk blocks[9];

VesaBlkGrp::VesaBlkGrp(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour) : x(x),
    y(y), xCnt(xCnt), yCnt(yCnt), colour(colour) {
    uint32_t cnt = 0;
    for(int i = 0; i < yCnt; i++) {
        for(int j = 0; j < xCnt; j++, cnt++) {
            blocks[cnt] = VesaBlk(j * 8 + x, i * 8 + y, colour);
        }
    }
    draw();
}

void VesaBlkGrp::draw() {
    uint32_t cnt = 0;
    for(int j = 0; j < xCnt * yCnt; j++, cnt++) {
        blocks[cnt].blkDraw();
    } 
}

void VesaBlkGrp::redraw(uint32_t newX, uint32_t newY) {
    x = newX; y = newY;
    uint32_t cnt = 0; 
    for(int i = 0; i < yCnt; i++) {
        for(int j = 0; j < xCnt; j++, cnt++) {
            blocks[cnt].blkRedraw(j * 8 + x, i * 8 + y);
        }
    }
}

}
