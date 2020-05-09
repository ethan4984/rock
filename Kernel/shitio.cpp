#include <port.h>
#include <shitio.h>
#include <memory.h>
#include <vesa.h>

#include <stdarg.h>

/* prototypes */

void update_cursor(size_t terminal_row, size_t terminal_column);

inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg);

inline uint16_t vga_entry(unsigned char uc, uint8_t color);

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

/* globals */

uint32_t WIDTH;

uint32_t HEIGHT;

uint8_t current_color = 0;

size_t terminal_row;

size_t terminal_column;

uint32_t terminal_bg;

uint32_t terminal_fg;

uint16_t *terminal_buffer;

size_t y;

size_t x;

int current_y = 0;

namespace standardout
{
    void initalize(uint32_t fg, uint32_t bg, uint64_t x, uint64_t y)
    {
        terminal_row = 0;
        terminal_column = 0;

        terminal_bg = bg;
        terminal_fg = fg;

        WIDTH = x;
        HEIGHT = y;

        clear_promnt();
    }

    bool end_of_terminal()
    {
        return (terminal_column >= 768) ? true : false;
    }

    bool end_of_screen(size_t offset)
    {
        return (terminal_row + offset >= 1024) ? true : false;
    }

    void change_text_color(uint8_t color)
    {
    }

    bool terminal_setcolor(uint8_t bg, uint8_t fg)
    {
        return false;
    }

    void disable_cursor()
    {
        outb(0x3d4, 0x0a);
        outb(0x3d5, 0x20);
    }

    void enable_cursor()
    {
        outb(0x3d4, 0x0a);
        outb(0x3d5, (inb(0x3d5) & 0xc0) | terminal_row);
        outb(0xd4, 0x0b);
        outb(0x3d5, (inb(0x3d5) & 0xe0) | terminal_column);
    }

    uint32_t *reference_column;

    void putchar(char c)
    {
        static bool is_back = false, bruh = false;

        if(end_of_terminal()) {
            vesa_scroll(8, terminal_bg);
            terminal_column -= 8;
        }

        switch(c) {
            case '\n':
                reference_column[terminal_row] = terminal_column;
                terminal_row = 0;
                terminal_column += 8;
                break;
            case '\t':
                for(int i = 0; i < 4; i++)
                    putchar(' ');
                break;
            case '\b':
                is_back = true;
                if(terminal_row == 0) {
                    if(terminal_column == 0)
                        break;
                    terminal_column -= 8;
                    terminal_row = WIDTH;
                    terminal_row -= 8;
                    putchar(' ');
                    terminal_row -= 8;
                    break;
                }
                terminal_row -= 8;
                putchar(' ');
                terminal_row -= 8;
                is_back = false;
                break;
            default:
                render_char(terminal_row, terminal_column, terminal_fg, terminal_bg, c);
                terminal_row += 8;
                if(terminal_row == WIDTH && !(is_back)) {
                    terminal_row = 0;
                    terminal_column += 8;
                    if(terminal_column == HEIGHT)
                        terminal_column = 0;
                }
                break;
        }
    }

    void t_print(const char str[256], ...)
    {
        uint64_t hold = 0;
        char *string;

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
                        string = convert(hold, 10);
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                    case 's':
                        string = va_arg(arg, char*);
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                    case 'x':
                        hold = va_arg(arg, uint64_t);
                        string = convert(hold, 16);
                        serial_write('0');
                        serial_write('x');
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                    case 'a':
                        hold = va_arg(arg, uint64_t);
                        string = convert(hold, 16);
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

    void k_print(const char *str, ...)
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
                        string = convert(number, 10);
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
                        string = convert(number, 16);
                        putchar('0');
                        putchar('x');
                        for(size_t i = 0; i < strlen(string); i++)
                            putchar(string[i]);
                        break;
                    case 'a':
                        number = va_arg(args, uint64_t);
                        string = convert(number, 16);
                        putchar('0');
                        putchar('x');
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

    void clear_screen()
    {
        terminal_column = 0;
        terminal_row = 0;
        for(y = 0; y < HEIGHT; y++) {
            for(x = 0; x < WIDTH; x++) {
            }
        }
    }

    void clear_promnt()
    {
        terminal_column = 0;
        terminal_row = 0;
        set_screen(terminal_bg);
    }

    int count_digits(int num) {
        int count =0;
        while(num > 0) {
            count++;
            num /= 10;
        }
        return count;
    }

    void special_char(char c, size_t x, size_t y, uint8_t fg, uint8_t bg)
    {
        const size_t index = y * WIDTH + x;
        if(bg == 17)
            terminal_buffer[index] = vga_entry(c, vga_entry_color(fg, current_color));
        else
            terminal_buffer[index] = vga_entry(c, vga_entry_color(fg, bg));
    }

    void special_num(int num, int size, size_t x, size_t y, uint8_t fg, uint8_t bg)
    {
        static int last_size = 0;

        for(int i = 0; i < last_size; i++)
            special_char(' ', i, y, current_color);

        char *cnum = 0;

        int tmp_size = size;

        while(num > 0) {
            cnum[tmp_size - 1] = (num % 10) + '0';
            num /= 10;
            tmp_size--;
        }

        if(bg == 17) {
            for(int i = 0; i < size; i++)
                special_char(cnum[i], x++, y, fg, current_color);
            last_size = x;
            return;
        }

        for(int i = 0; i < size; i++)
            special_char(cnum[i], x++, y, fg, bg);

        last_size = x;
    }

    int grab_current_y()
    {
        return current_y++;
    }
}

char *convert(uint64_t num, int base)
{
    static char hold[] = "0123456789ABCDEF";
    static char buffer[50];
    char *str;

    str = &buffer[49];
    *str = '\0';

    do {
        *--str = hold[num%base];
        num /= base;
    } while(num != 0);

    return str;
}

void update_cursor(size_t terminal_row, size_t terminal_column)
{
    unsigned short position = terminal_row * 80 + terminal_column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char )((position >> 8) & 0xFF));
}

inline uint8_t vga_entry_color(uint8_t foreground, uint8_t background)
{
    return foreground | background << 4;
}

inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}
