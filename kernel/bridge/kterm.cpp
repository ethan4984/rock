#include <kernel/bridge/commandHub.h>
#include <kernel/bridge/kterm.h>
#include <lib/stringUtils.h> 
#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <lib/output.h>
#include <lib/vesa.h>
#include <lib/bmp.h>
#include <stdarg.h>

namespace kernel {

command *kterm::commands;

void kterm::addBuffer(char c) {
    if(currentIndex >= maxSize) {
        buffer = (char*)kheap.krealloc(buffer, maxSize);
        maxSize *= 2;
    }

    buffer[currentIndex++] = c;
}

void kterm::drawBackground(uint32_t x, uint32_t y) {
    if(backgroundOverride != 0x69694200) {
        for(int i = 0; i < 8; i++) {
            for(int j = 0; j < 8; j++)
                vesa.setPixel(x + i, y + j, backgroundOverride);
        }
        return;
    } 

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++)
            vesa.setPixel(x + i, y + j, bmpGetPixel(x + i, y + j, background));
    }
} 

void kterm::putchar(uint8_t c) {
    switch(c) {
        case '\n':
            currentRow = 0;
            currentColumn += 8;

            commandHub();

            memset(buffer, 0, maxSize);
            currentIndex = 0;
            break;
        case '\b':
            if(currentColumn == 0) 
                break;

            if(currentIndex != 0 && input) {
                buffer[--currentIndex] = 0;
            }

            if(currentRow == 0) {
                drawBackground(currentRow, currentColumn);
                currentRow = vesa.width;
                currentColumn -= 8;
                break;
            } 

            currentRow -= 8;
            drawBackground(currentRow, currentColumn);

            break;
        default:
            vesa.renderChar(currentRow, currentColumn, foreground, c);
            
            if(input)
                addBuffer(c);

            currentRow += 8;
            if(currentRow == vesa.width) {
                currentRow = 0;
                currentColumn += 8;
                if(currentColumn == vesa.height)
                    currentColumn = 0;
            }
    }
}

void kterm::print(const char *str, ...) {
    va_list arg;
    va_start(arg, str);

    input = false;

    printArgs(str, arg, basePutchar);

    input = true;
}

void kterm::setForeground(uint32_t fg) {
    foreground = fg;
}

void kterm::setBackground(const char *filePath, uint32_t colourOverride) {
    if(colourOverride != 0x69694200) {
        backgroundOverride = colourOverride;
        return;
    }

    background = drawBMP(filePath);
}

void kterm::init() { 
    buffer = new char[256];
    maxSize = 256; 
    memset(buffer, 0, maxSize);
    commands = new command[50];
}

void basePutchar(uint8_t c) {
    kterm.putchar(c);
}

}
