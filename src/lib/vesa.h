#pragma once

#include <kernel/stivale.h>

#include <lib/output.h>

#define VESA_BLOCK_SIZE 8

namespace vesa {

inline uint32_t width, height, bpp, framebuffer, pitch;

void init(stivaleInfo_t *stivale);

void renderChar(uint64_t x, uint64_t y, uint32_t fg, char c);

void setPixel(uint16_t x, uint16_t y, uint32_t colour);

uint32_t grabColour(uint16_t x, uint16_t y);

class blk {
public:
    blk(uint32_t x, uint32_t y, uint32_t colour) : x(x), 
        y(y), colour(colour) { }

    blk() = default;

    void blkRedraw(uint32_t newX, uint32_t newY);

    void blkChangeColour(uint32_t newColour);

    void blkDraw();
private:
    uint32_t x, y, colour;

    uint32_t backgroundBuffer[64];
};

class blkGrp {
public:
    blkGrp(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour);

    blkGrp() = default;

    void draw();

    void redraw(uint32_t newX, uint32_t newY);

    blk *blocks;
private:    
    uint32_t x, y, xCnt, yCnt, colour;
};

class shape {
public:
    shape(uint32_t x, uint32_t y, uint8_t *bitmap, uint32_t xCnt, uint32_t yCnt, uint32_t colour);

    void draw();

    void redraw(uint32_t newX, uint32_t newY);
private:
    uint32_t *background, x, y, xCnt, yCnt, colour;
    uint8_t *foreground; 
};

}
