#pragma once

#include <kernel/stivale.h>

#include <lib/output.h>

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

class VesaBlk {
public:
    VesaBlk(uint32_t x, uint32_t y, uint32_t colour) : x(x), 
        y(y), colour(colour) { }

    VesaBlk() = default;

    void blkRedraw(uint32_t newX, uint32_t newY);

    void blkChangeColour(uint32_t newColour);

    void blkDraw();
private:
    uint32_t x, y, colour;

    uint32_t backgroundBuffer[64];
};

class VesaBlkGrp {
public:
    VesaBlkGrp(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour);

    ~VesaBlkGrp();

    void draw();

    void redraw(uint32_t newX, uint32_t newY);
private:    
    uint32_t x, y, xCnt, yCnt, colour;

    VesaBlk *blocks;
};

class VesaShape {
public:
    VesaShape(uint32_t x, uint32_t y, uint8_t *bitmap, uint32_t xCnt, uint32_t yCnt, uint32_t colour);

    void draw();

    void redraw(uint32_t newX, uint32_t newY);
private:
    uint32_t *background, x, y, xCnt, yCnt, colour;
    uint8_t *foreground; 
};

inline vesa vesa;

}
