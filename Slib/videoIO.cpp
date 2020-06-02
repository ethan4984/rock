#include <Kernel/drivers/vesa.h>

#include <videoIO.h>
#include <stdarg.h>
#include <stringUtils.h>
#include <ports.h>
#include <stddef.h>

uint64_t terminalRow, terminalColumn;
uint32_t terminalBG, terminalFG, height, width;
uint32_t *referenceColumn;

void initalizeVESA(uint32_t fg, uint32_t bg, uint64_t x, uint64_t y) 
{
    terminalRow = 0;
    terminalColumn = 0;

    terminalBG = bg;
    terminalFG = fg;

    height = y;
    width = x;

    referenceColumn = (uint32_t*)0x10000;
}

bool endOfScreenV()
{
    return (terminalColumn >= height) ? true : false;
}

bool endOfScreenH(size_t offset)
{
    return (terminalRow + offset >= width) ? true : false;
}

namespace out
{
    void cPrint(const char *str, ...) /* ouputs to COM1 */
    {
        uint64_t hold = 0;
        char *string;
        char character;

        va_list arg;
        va_start(arg, str);

        for(uint64_t i = 0; i < strlen(str); i++) {
            if(str[i] != '%')
                serial_write(str[i]);
            else {
                i++;
                switch(str[i]) {
                    case 'd':
                        hold = va_arg(arg, long);
                        string = itob(hold, 10);
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                    case 's':
                        string = va_arg(arg, char*);
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                    case 'c':
                        character = va_arg(arg, int);
                        serial_write(character);
                        break; 
                    case 'x':
                        hold = va_arg(arg, uint64_t);
                        string = itob(hold, 16);
                        serial_write('0');
                        serial_write('x');
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                    case 'a':
                        hold = va_arg(arg, uint64_t);
                        string = itob(hold, 16);
                        serial_write('0');
                        serial_write('x');
                        int offset_zeros = 16 - strlen(string);
                        for(int i = 0; i < offset_zeros; i++)
                            serial_write('0');
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                }
            }
        }
        va_end(arg);
        serial_write('\n');
    }

    void putchar(char c)
    {
        static bool isBack = false;

        if(endOfScreenV()) {
            vesaScroll(8, terminalBG);
            terminalColumn -= 8;
        }

        switch(c) {
            case '\n':
                referenceColumn[terminalRow] = terminalColumn;
                terminalRow = 0;
                terminalColumn += 8;
                break;
            case '\t':
                for(int i = 0; i < 4; i++)
                    putchar(' ');
                break;
            case '\b':
                isBack = true;
                if(terminalRow == 0) {
                    if(terminalColumn == 0)
                        break;
                    terminalColumn -= 8;
                    terminalRow = width;
                    terminalRow -= 8;
                    putchar(' ');
                    terminalRow -= 8;
                    break;
                }
                terminalRow -= 8;
                putchar(' ');
                terminalRow -= 8;
                isBack = false;
                break;
            default:
                renderChar(terminalRow, terminalColumn, terminalFG, terminalBG, c);
                terminalRow += 8;
                if(terminalRow == width && !(isBack)) {
                    terminalRow = 0;
                    terminalColumn += 8;
                    if(terminalColumn == height)
                        terminalColumn = 0;
                }
                break;
        }
    }

    void kPrint(const char *str, ...)
    {
        char *string;
        uint64_t number;

        va_list args;
        va_start(args, str);

        for(uint64_t i = 0; i < strlen(str); i++) {
            if(str[i] != '%')
                putchar(str[i]);
            else {
                switch(str[++i]) {
                    case 'd':
                        number = va_arg(args, long);
                        string = itob(number, 10);
                        for(size_t i = 0; i < strlen(string); i++)
                            putchar(string[i]);
                        break;
                    case 's':
                        string = va_arg(args, char *);
                        for(size_t i = 0; i < strlen(string); i++)
                            putchar(string[i]);
                        break;
                    case 'x':
                        number = va_arg(args, uint64_t);
                        string = itob(number, 16);
                        putchar('0');
                        putchar('x');
                        for(size_t i = 0; i < strlen(string); i++)
                            putchar(string[i]);
                        break;
                    case 'a':
                        number = va_arg(args, uint64_t);
                        string = itob(number, 16);
                        int offset_zeros = 16 - strlen(string);
                        for(int i = 0; i < offset_zeros; i++)
                            putchar('0');
                        for(size_t i = 0; i < strlen(string); i++)
                            putchar(string[i]);
                        break;
                }
            }
        }
        va_end(args);
    }
}
