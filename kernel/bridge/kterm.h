#pragma once

#include <lib/bmp.h>

#include <stdint.h>

namespace kernel {

class kterm {
public: 
    void setBackground(const char *imagePath, uint32_t colourOverride = 0x69694200);

    void setForeground(uint32_t fg);

    void print(const char *str, ...);

    void putchar(char c);
private:
    uint32_t foreground = 0xffffff, currentRow = 0, currentColumn = 0, backgroundOverride = 0x69694200;

    bmpImage_t background;

    void drawBackground(uint32_t x, uint32_t y);
};

inline kterm kterm;

}
