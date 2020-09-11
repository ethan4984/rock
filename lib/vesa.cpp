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

void blkDraw(VesaBlk *blk) {
    uint32_t cnt = 0;
    for(uint32_t i = blk->x; i < blk->x + VESA_BLOCK_SIZE; i++) {
        for(uint32_t j = blk->y; j < blk->y + VESA_BLOCK_SIZE; j++) {
            blk->backgroundBuffer[cnt++] = vesa.grabColour(i, j);
            vesa.setPixel(i, j, blk->colour); 
        }
    }
}

void blkRedraw(uint32_t newX, uint32_t newY, VesaBlk *blk) {
    kprintDS("[KDEBUG]", "%d %d", blk->x, blk->y);
    uint32_t cnt = 0;
    for(uint32_t i = blk->x; i < blk->x + VESA_BLOCK_SIZE; i++) {
        for(uint32_t j = blk->y; j < blk->y + VESA_BLOCK_SIZE; j++) {
            vesa.setPixel(i, j, blk->backgroundBuffer[cnt++]); 
        }
    }

    blk->x = newY; blk->y = newY;
    blkDraw(blk);
}

void changeColour(uint32_t newColour, VesaBlk *blk) {
    blk->colour = newColour; 
    blkDraw(blk);
}

VesaBlkGrp::VesaBlkGrp(uint32_t x, uint32_t y, uint32_t blkCntX, uint32_t blkCntY, uint32_t colour) : x(x), 
    y(y), blkCntX(blkCntX), blkCntY(blkCntY), colour(colour) {
    blocks = new VesaBlk[blkCntX * blkCntY]; 
    uint32_t cnt = 0;
    for(int i = 0; i < blkCntY; i++) {
        for(int j = 0; j < blkCntX; j++, cnt++) {
            blocks[cnt] = { j * VESA_BLOCK_SIZE + x, i * VESA_BLOCK_SIZE + y, colour };
            blkDraw(&blocks[cnt]);
        }
    } 
}

VesaBlkGrp::~VesaBlkGrp() {
    delete blocks;
}

void VesaBlkGrp::draw() { 
    for(int i = 0; i < blkCntX * blkCntY; i++) {
        blkDraw(&blocks[i]);
    }
}

void VesaBlkGrp::redraw(uint32_t newX, uint32_t newY) {
    x = newX;
    y = newY;
    uint32_t cnt = 0;
    for(int i = 0; i < blkCntY; i++) {
        for(int j = 0; j < blkCntX; j++) {
            VesaBlk *lel = &blocks[cnt++];
            blkRedraw(j * 8 + x, i * 8 + y, lel);
        }
    } 
}

}
