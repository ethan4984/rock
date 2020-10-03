#pragma once

#include <lib/font.h>

class character {
public:
    character(char c, uint32_t x, uint32_t y, uint32_t colour);

    character() = default;

    void render();

    void unrender();
private:
    uint32_t backgroundBuffer[64];

    char c;

    uint32_t x, y, colour;
};
   
class textBox {
public:
    textBox(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour);

    ~textBox();

    void printf(const char *str, ...);

    void putchar(uint8_t c);

    void deleteAll();
private:
    uint32_t x, y, xCnt, yCnt, colour, totalChars = 0;

    character *characters;
};
