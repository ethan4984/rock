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

    void updatePath(const char *currentPath);
public:
    char *buffer;

    uint32_t currentIndex;

    uint32_t maxSize;
private:
    uint32_t foreground = 0xffffff, currentRow = 0, currentColumn = 0, backgroundOverride = 0x69694200, cursorColour = 0xffc0cb;

    bool input = true;

    bmpImage_t background;

    static command *commands;

    static uint32_t commandCnt;

    char *path;
private:
    void drawBackground(uint32_t x, uint32_t y);

    void updateCursor(); 

    void addBuffer(char c);

    void commandHub();
};

inline kterm kterm;

}
