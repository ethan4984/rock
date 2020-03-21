#include <port.h>
#include <shitio.h>
#include <memory.h>

#include <stdarg.h>

/* prototypes */

void update_cursor(size_t terminal_row, size_t terminal_column);

inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg);

inline uint16_t vga_entry(unsigned char uc, uint8_t color);

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

/* globals */

const size_t VGA_WIDTH = 80;

static const size_t VGA_HEIGHT = 25;

uint8_t current_color = 0;

size_t terminal_row;

size_t terminal_column;

uint8_t terminal_color;

uint16_t *terminal_buffer;

size_t y;

size_t x;

int current_y = 0;

namespace standardout
{
    void initalize(uint8_t bg, uint8_t fg)
    {
        clear_screen();
        terminal_row = 0;
        terminal_column = 0;
        terminal_color = vga_entry_color(fg, bg);
        terminal_buffer = VGA_MEMORY;

        current_color = bg;

        for(y = 0; y < VGA_HEIGHT; y++) {
            for(x = 0; x < VGA_WIDTH; x++) {
                const size_t index = y * VGA_WIDTH + x;
                terminal_buffer[index] = vga_entry(' ', terminal_color);
            }
        }
    }

    bool end_of_terminal()
    {
        return (terminal_row >= 24) ? true : false;
    }

    bool end_of_screen(size_t offset)
    {
        return (terminal_column + offset >= 45) ? true : false;
    }

    void change_text_color(uint8_t color)
    {
        terminal_color = vga_entry_color(color, current_color);
    }

    bool terminal_setcolor(uint8_t bg, uint8_t fg)
    {
        if(bg <= 15 && bg > 0) {
            if(fg <= 15 && fg > 0) {
                terminal_buffer = VGA_MEMORY;
                terminal_color = vga_entry_color(fg, bg);
                return true;
            }
        }
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
        switch(c) {
            case '\n':
                reference_column[terminal_row] = terminal_column;
                terminal_row++;
                terminal_column = 0;
                update_cursor(terminal_row, terminal_column);
                break;
            case '\t':
                for(int i = 0; i < 4; i++)
                    putchar(' ');
                break;
            case '\b':
                if(terminal_column == 0) {
                    if(terminal_row != 0) {
                        terminal_row--;
                        terminal_column = reference_column[terminal_row];
                        update_cursor(terminal_row, terminal_column);
                    }
                    break;
                }
                terminal_column--;
                terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
                update_cursor(terminal_row, terminal_column);
                break;
            default:
                terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
                if(++terminal_column == VGA_WIDTH) {
                    terminal_column = 0;
                    if (++terminal_row == VGA_HEIGHT)
                        terminal_row = 0;
                }
                update_cursor(terminal_row, terminal_column);
                break;
        }
        if(end_of_terminal())
            clear_promnt();
    }

    void k_print(const char str[256],...)
    {
        uint64_t hold = 0;
        char *string;

        va_list arg;
        va_start(arg, str);

        int length = strlen(str);

        for(int i = 0; i < length; i++) {
            if(str[i] != '%')
                putchar(str[i]);
            else {
                i++;
                switch(str[i]) {
                    case 'd':
                        hold = va_arg(arg, long);
                        string = convert(hold, 10);
                        for(size_t i = 0; i < strlen(string); i++)
                            putchar(string[i]);
                        break;
                    case 's':
                        string = va_arg(arg, char *);
                        for(size_t i = 0; i < strlen(string); i++)
                            putchar(string[i]);
                        break;
                    case 'x':
                        hold = va_arg(arg, uint64_t);
                        string = convert(hold, 16);
                        putchar('0');
                        putchar('x');
                        for(size_t i = 0; i < utoa(hold, string, 16); i++)
                            putchar(string[i]);
                        break;
                    case 'a':
                        hold = va_arg(arg, uint64_t);
                        string = convert(hold, 16);
                        putchar('0');
                        putchar('x');
                        int offset_zeros = 16 - strlen(string);
                        for(int i = 0; i < offset_zeros; i++)
                            putchar('0');
                        for(size_t i = 0; i < strlen(string); i++)
                            putchar(string[i]);
                        break;
                }
                va_end(arg);
            }
        }
    }

    void s_print(uint8_t color, size_t x, size_t y, const char str[256],...)
    {
        unsigned int hold = 0;
        char *string;

        va_list arg;
        va_start(arg, str);

        int length = strlen(str);

        for(int i = 0; i < length; i++) {
            if(str[i] != '%')
                special_char(str[i], x++, y, color);
            else {
                i++;
                switch(str[i]) {
                    case 'd':
                        hold = va_arg(arg, int);
                        string = convert(hold, 10);
                        for(size_t i = 0; i < strlen(string); i++)
                            special_char(string[i], x++, y, color);
                        break;
                    case 's':
                        string = va_arg(arg, char *);
                        for(size_t i = 0; i < strlen(string); i++)
                            special_char(string[i], x++, y, color);
                        break;
                    case 'x':
                        hold = va_arg(arg, unsigned int);
                        string = convert(hold, 16);
                        special_char('0', x++, y, color);
                        special_char('x', x++, y, color);
                        for(size_t i = 0; i < strlen(string); i++)
                            special_char(string[i], x++, y, color);
                        break;
                    case 'a':
                        hold = va_arg(arg, uint64_t);
                        string = convert(hold, 16);
                        special_char('0', x++, y, color);
                        special_char('x', x++, y, color);
                        int offset_zeros = 16 - strlen(string);
                        for(uint64_t i = 0; i < offset_zeros; i++)
                            special_char('0', x++, y, color);
                        for(size_t i = 0; i < strlen(string); i++)
                            special_char(string[i], x++, y, color);
                        break;
                }
                va_end(arg);
            }
        }
        current_y = y;
    }

    void t_print(const char str[256],...)
    {
        uint64_t hold = 0;
        char *string;

        va_list arg;
        va_start(arg, str);

        int length = strlen(str);

        for(int i = 0; i < length; i++) {
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
                        string = va_arg(arg, char *);
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
                        putchar('0');
                        putchar('x');
                        int offset_zeros = 16 - strlen(string);
                        for(size_t i = 0; i < offset_zeros; i++)
                            serial_write('0');
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                }
                va_end(arg);
            }
        }
        serial_write('\n');
    }

    void clear_screen()
    {
        terminal_column = 0;
        terminal_row = 0;
        for(y = 0; y < VGA_HEIGHT; y++) {
            for(x = 0; x < VGA_WIDTH; x++) {
                const size_t index = y * VGA_WIDTH + x;
                terminal_buffer[index]=vga_entry(' ', terminal_color);
            }
        }
    }

    void clear_promnt()
    {
        terminal_column = 0;
        terminal_row = 0;
        for(y = 0; y < 25; y++) {
            for(x = 0; x < 48; x++) {
                const size_t index = y * 80 + x;
                terminal_buffer[index] = vga_entry(' ', terminal_color);
            }
        }
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
        const size_t index = y * VGA_WIDTH + x;
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
    static char hold[]= "0123456789ABCDEF";
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

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

int strcmp(const char *a, const char *b)
{
    while(*a && *a == *b) {
        a++;
        b++;
    }
    return (int)(unsigned char)(*a) - (int)(unsigned char)(*b);
}

char *strcpy(char *dest, const char *src)
{
    if(dest == NULL)
    return NULL;

    char *new_dest = dest;

    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }

    *dest = '\0';

    return new_dest;
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
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}
