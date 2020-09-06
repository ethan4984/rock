#include <kernel/bridge/kterm.h>
#include <lib/stringUtils.h> 
#include <lib/output.h>
#include <lib/vesa.h>
#include <lib/bmp.h>
#include <stdarg.h>

namespace kernel {

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

void kterm::putchar(char c) {
    switch(c) {
        case '\n':
            currentRow = 0;
            currentColumn += 8;
            break;
        case '\b':
            if(currentRow == 0) {
                if(currentColumn == 0) 
                    break;

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
    uint64_t hold = 0;
    char *string;
    char character;

    va_list arg;
    va_start(arg, str);

    for(uint64_t i = 0; i < strlen(str); i++) {
        if(str[i] != '%')
            putchar(str[i]);
        else {
            i++;
            switch(str[i]) {
                case 'd':
                    hold = va_arg(arg, long);
                    string = itob(hold, 10);
                    for(uint64_t i = 0; i < strlen(string); i++)
                        putchar(string[i]);
                    break;
                case 's':
                    string = va_arg(arg, char*);
                    for(uint64_t i = 0; i < strlen(string); i++)
                        putchar(string[i]);
                    break;
                case 'c':
                    character = va_arg(arg, int);
                    putchar(character);
                    break; 
                case 'x':
                    hold = va_arg(arg, uint64_t);
                    string = itob(hold, 16);
                    for(uint64_t i = 0; i < strlen(string); i++)
                        putchar(string[i]);
                    break;
                case 'a':
                    hold = va_arg(arg, uint64_t);
                    string = itob(hold, 16);
                    int offset_zeros = 16 - strlen(string);
                    for(int i = 0; i < offset_zeros; i++)
                        putchar('0');
                    for(uint64_t i = 0; i < strlen(string); i++)
                        putchar(string[i]);
                    break;
            }
        }
    }

    va_end(arg);
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

}
