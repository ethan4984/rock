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
    for(uint32_t i = y; i < y + VESA_BLOCK_SIZE; i++) {
        for(uint32_t j = x; j < x + VESA_BLOCK_SIZE; j++) {
            backgroundBuffer[cnt++] = vesa.grabColour(j, i);
            vesa.setPixel(j, i, colour); 
        }
    }
}

void VesaBlk::blkRedraw(uint32_t newX, uint32_t newY) {
    uint32_t cnt = 0;
    for(uint32_t i = y; i < y + VESA_BLOCK_SIZE; i++) {
        for(uint32_t j = x; j < x + VESA_BLOCK_SIZE; j++) {
            vesa.setPixel(j, i, backgroundBuffer[cnt++]); 
        }
    }

    x = newX; y = newY;
    blkDraw();
}

void VesaBlk::blkChangeColour(uint32_t newColour) {
    colour = newColour; 
    blkDraw();
}

VesaBlkGrp::VesaBlkGrp(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour) : x(x),
    y(y), xCnt(xCnt), yCnt(yCnt), colour(colour) {
    uint32_t cnt = 0;

    blocks = new VesaBlk[xCnt * yCnt];
    for(uint32_t i = 0; i < yCnt; i++) {
        for(uint32_t j = 0; j < xCnt; j++, cnt++) {
            blocks[cnt] = VesaBlk(j * 8 + x, i * 8 + y, colour);
        }
    }

    draw();
}

/*VesaBlkGrp::~VesaBlkGrp() {
    kprintDS("[KDEBUG]", "%x", (uint64_t)blocks);
    delete blocks;
}*/

void VesaBlkGrp::draw() {
    for(uint32_t j = 0; j < xCnt * yCnt; j++) {
        blocks[j].blkDraw();
    } 
}

void VesaBlkGrp::redraw(uint32_t newX, uint32_t newY) {
    x = newX; y = newY;
    uint32_t cnt = 0; 
    for(uint32_t i = 0; i < yCnt; i++) {
        for(uint32_t j = 0; j < xCnt; j++, cnt++) {
            blocks[cnt].blkRedraw(j * 8 + x, i * 8 + y);
        }
    }
}

VesaShape::VesaShape(uint32_t x, uint32_t y, uint8_t *foreground, uint32_t xCnt, uint32_t yCnt, uint32_t colour) : x(x), y(y), xCnt(xCnt), yCnt(yCnt), colour(colour), foreground(foreground) {
    background = new uint32_t[xCnt * yCnt];
    draw();
}

void VesaShape::draw() {
    uint32_t cnt = 0;

    for(uint32_t i = y; i < y + yCnt; i++) {
        for(uint32_t j = x; j < x + xCnt; j++, cnt++) {
            if(foreground[cnt] == 1) {
                background[cnt] = vesa.grabColour(j, i);
                vesa.setPixel(j, i, colour); 
            } else if(foreground[cnt] == 2) {
                background[cnt] = vesa.grabColour(j, i);
                vesa.setPixel(j, i, 0); 
            }
        }
    }
}

void VesaShape::redraw(uint32_t newX, uint32_t newY) {
    uint32_t cnt = 0;
    for(uint32_t i = y; i < y + xCnt; i++) { 
        for(uint32_t j = x; j < x + yCnt; j++, cnt++) { 
            if(foreground[cnt] != 0) {
                vesa.setPixel(j, i, background[cnt]);
            }
        }
    }
    x = newX; y = newY;
    draw();
}

}
