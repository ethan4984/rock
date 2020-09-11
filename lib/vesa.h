#pragma once

#include <kernel/stivale.h>

#define VESA_BLOCK_SIZE 8

namespace kernel {

class vesa {
public:
    void initVESA(stivaleInfo_t *stivale);

    void renderChar(uint64_t x, uint64_t y, uint32_t fg, char c);

    void setPixel(uint16_t x, uint16_t y, uint32_t colour);

    uint32_t grabColour(uint16_t x, uint16_t y);

    uint32_t width, height, bpp, framebuffer, pitch;
};

struct VesaBlk {
    uint32_t x, y, colour;
    uint32_t backgroundBuffer[VESA_BLOCK_SIZE^2];
} __attribute__((packed));

void blkRedraw(uint32_t newX, uint32_t newY, VesaBlk *blk);

void blkChangeColour(uint32_t newColour, VesaBlk *blk);

void blkDraw(VesaBlk *blk);
    
class VesaBlkGrp {
public:
    VesaBlkGrp(uint32_t x, uint32_t y, uint32_t blkCntX, uint32_t blkCntY, uint32_t colours);

    VesaBlkGrp() = default;

    ~VesaBlkGrp();

    void draw();

    void redraw(uint32_t newX, uint32_t newY);
private:
    VesaBlk *blocks;

    uint32_t x, y, blkCntX, blkCntY, colour;
};

inline vesa vesa;

}
