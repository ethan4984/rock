#include "port.h"
#include "shitio.h"

#include <stdarg.h>

const size_t VGA_WIDTH = 80;

static const size_t VGA_HEIGHT = 25;

size_t terminal_row;

size_t terminal_column;

uint8_t terminal_color;

uint16_t *terminal_buffer;

size_t y;

size_t x;

enum vga_color {
       	VGA_COLOR_BLACK = 0,
    	VGA_COLOR_BLUE = 1,
        VGA_COLOR_GREEN = 2,
        VGA_COLOR_CYAN = 3,
        VGA_COLOR_RED = 4,
        VGA_COLOR_MAGENTA = 5,
        VGA_COLOR_BROWN = 6,
        VGA_COLOR_LIGHT_GREY = 7,
        VGA_COLOR_DARK_GREY = 8,
        VGA_COLOR_LIGHT_BLUE = 9,
        VGA_COLOR_LIGHT_GREEN = 10,
        VGA_COLOR_LIGHT_CYAN = 11,
        VGA_COLOR_LIGHT_RED = 12,
        VGA_COLOR_LIGHT_MAGENTA = 13,
        VGA_COLOR_LIGHT_BROWN = 14,
        VGA_COLOR_WHITE = 15,
};

size_t strlen(const char *str) {
    size_t len = 0;
	while (str[len])
		len++;
	return len;
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { ++a; ++b; }
    return (int)(unsigned char)(*a) - (int)(unsigned char)(*b);
}

void update_cursor(size_t terminal_row, size_t terminal_column) {
     unsigned short position = terminal_row * 80 + terminal_column;
     outb(0x3D4, 0x0F);
     outb(0x3D5, (unsigned char)(position & 0xFF));
     outb(0x3D4, 0x0E);
     outb(0x3D5, (unsigned char )((position >> 8) & 0xFF));
}

inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

bool end_of_terminal() {
	if(x == VGA_HEIGHT - 1)
		return true;
	return false;
}

void initalize() {
    terminal_row = 0;
	terminal_column = 0;
	terminal_color=vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer=VGA_MEMORY;
	for(y = 0; y < VGA_HEIGHT; y++) {
		for(x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index]=vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

uint32_t *reference_column;

void putchar(char c) {
    switch(c) {
        case '\n':
		reference_column[terminal_row] = terminal_column;
            terminal_row++;
            terminal_column = 0;
	    update_cursor(terminal_row, terminal_column);
            break;
        case '\t':
            while(terminal_column % 4)
                terminal_putentryat(' ', terminal_color, terminal_column++, terminal_row);
	    update_cursor(terminal_row, terminal_column);
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
}

char *convert(unsigned int num, int base)
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

	return(str);
}

void k_print(char str[256],...) {
    unsigned int hold = 0;
    char *string;

    va_list arg;
	va_start(arg, str);

    for(size_t i = 0; i < strlen(str); i++) {
        if(str[i] != '%')
        	putchar(str[i]);
        else {
            i++;
            switch(str[i]) {
                case 'd':
                    hold = va_arg(arg, int);
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
                    hold = va_arg(arg, unsigned int);
                    string = convert(hold, 16);
                    putchar('0');
                    putchar('x');
                    for(size_t i = 0; i < strlen(string); i++)
                        putchar(string[i]);
                    break;
            }
            va_end(arg);
        }
    }
}

void t_print(char str[256],...) {
    unsigned int hold = 0;
    char *string;

    va_list arg;
	va_start(arg, str);

    for(size_t i = 0; i < strlen(str); i++) {
        if(str[i] != '%')
        	serial_write(str[i]);
        else {
            i++;
            switch(str[i]) {
                case 'd':
                    hold = va_arg(arg, int);
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
                    hold = va_arg(arg, unsigned int);
                    string = convert(hold, 16);
                    serial_write('0');
                    serial_write('x');
                    for(size_t i = 0; i < strlen(string); i++)
                        serial_write(string[i]);
                    break;
            }
            va_end(arg);
        }
    }
    serial_write('\n');
}

void clear_screen() {
	terminal_column = 0;
	terminal_row = 0;
	for(y = 0; y < VGA_HEIGHT; y++) {
		for(x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index]=vga_entry(' ', terminal_color);
       		 }
	}
}














