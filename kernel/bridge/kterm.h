#pragma once

#include <kernel/bridge/commandHub.h>

#include <lib/memoryUtils.h>
#include <lib/bmp.h>

#include <stdint.h>

namespace kernel {

void basePutchar(uint8_t c);

class kterm {
public: 
    void init();

    void setBackground(const char *imagePath, uint32_t colourOverride = 0x69694200);

    void setForeground(uint32_t fg);

    void print(const char *str, ...);

    void putchar(uint8_t c);

    void addCommand(command newCommand);
public:
    char *buffer;

    uint32_t currentIndex;

    uint32_t maxSize;
private:
    uint32_t foreground = 0xffffff, currentRow = 0, currentColumn = 0, backgroundOverride = 0x69694200;

    bool input = true;

    bmpImage_t background;

    static command *commands;

    uint32_t commandCnt = 0;
private:
    void drawBackground(uint32_t x, uint32_t y);

    void addBuffer(char c);

    void commandHub();
};

inline kterm kterm;

}
