#include <lib/memoryUtils.h>
#include <lib/stringUtils.h>
#include <lib/gui/text.h>
#include <lib/vesa.h>

namespace kernel {

character::character(char c, uint32_t x, uint32_t y, uint32_t colour) : c(c),
    x(x), y(y), colour(colour) {

}

void character::render() {
    uint32_t cnt = 0;
    for(uint32_t i = y; i < y + 8; i++) {
        for(uint32_t j = x; j < x + 8; j++, cnt++) {
            backgroundBuffer[cnt] = vesa.grabColour(j, i);
        }
    }

    vesa.renderChar(x, y, colour, c);
}

void character::unrender() {
    uint32_t cnt = 0;
    for(uint32_t i = y; i < y + 8; i++) {
        for(uint32_t j = x; j < x + 8; j++, cnt++) {
            vesa.setPixel(j, i, backgroundBuffer[cnt]);
        }
    }
}

textBox::textBox(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour) : x(x),
    y(y), xCnt(xCnt), yCnt(yCnt), colour(colour) {
    characters = new character[50];
}

textBox::~textBox() {
    delete characters;
}

void textBox::deleteAll() {
    uint32_t oldTotal = totalChars;
    for(int i = 0; i < oldTotal; i++) {
        characters[totalChars--].unrender();
        x -= 8;
    }
}

void textBox::putchar(uint8_t c) {
    switch(c) {
        case '\t':
            for(int i = 0; i < 4; i++)
                putchar(' ');
            break; 
        case '\n':
            x = 0;
            if(y != yCnt) 
                y += 8;
            break;
        case '\b':
            if(totalChars == 0)
                break;
            characters[totalChars--].unrender();
            x -= 8;
            break;
        default:
            characters[totalChars] = character(c, x, y, colour);
            characters[totalChars].render();

            x += 8;
            if(x == xCnt) { 
                x = 0;
                y += 8;
                if(y == yCnt)
                    y = 0;
            }

            totalChars++;
    }
}

void textBox::printf(const char *str, ...) {
    va_list arg;
    va_start(arg, str);

    uint64_t hold = 0;
    char *string;
    char character;
	
    for(uint64_t i = 0; i < strlen(str); i++) {
        if(str[i] != '%')
            putchar(str[i]); 
        else {
            switch(str[++i]) {
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
                    int offsetZeros = 16 - strlen(string);
                    for(int i = 0; i < offsetZeros; i++)
                        putchar('0');
                    for(uint64_t i = 0; i < strlen(string); i++)
                        putchar(string[i]);
                    break;
            }
        }
    }
    va_end(arg);
} 

}
