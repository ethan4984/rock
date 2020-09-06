#pragma once

#include <kernel/stivale.h>

namespace kernel {

class vesa {
public:
    void initVESA(stivaleInfo_t *stivale);

    void renderChar(uint64_t x, uint64_t y, uint32_t fg, char c);

    void setPixel(uint16_t x, uint16_t y, uint32_t colour);

    uint32_t grabColour(uint16_t x, uint16_t y);

    uint32_t width, height, bpp, framebuffer, pitch;
};

inline vesa vesa;

}
