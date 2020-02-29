#include "port.h"
#include "shitio.h"

#include <stdarg.h>

/* prototypes */

char *convert(unsigned int num, int base);

void update_cursor(size_t terminal_row, size_t terminal_column);

inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg);

inline uint16_t vga_entry(unsigned char uc, uint8_t color);

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

/* globals */

const size_t VGA_WIDTH = 80;

static const size_t VGA_HEIGHT = 25;

size_t terminal_row;

size_t terminal_column;

uint8_t terminal_color;

uint16_t *terminal_buffer;

size_t y;

size_t x;

namespace standardout {
	void initalize(uint8_t bg, uint8_t fg) {
		clear_screen();
		terminal_row = 0;
		terminal_column = 0;
		terminal_color = vga_entry_color(fg, bg);
		terminal_buffer = VGA_MEMORY;

		for(y = 0; y < VGA_HEIGHT; y++) {
			for(x = 0; x < VGA_WIDTH; x++) {
				const size_t index = y * VGA_WIDTH + x;
				terminal_buffer[index] = vga_entry(' ', terminal_color);
			}
		}
	}

	bool end_of_terminal() {
		return (terminal_row >= 24) ? true : false;
	}

	bool terminal_setcolor(uint8_t bg, uint8_t fg) {
		if(bg <= 15 && bg > 0) {
			if(fg <= 15 && fg > 0) {
				terminal_buffer = VGA_MEMORY;
				terminal_color = vga_entry_color(fg, bg);
				return true;
			}
		}
		return false;
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
			clear_screen();
	}

	void k_print(const char str[256],...) {
		unsigned int hold = 0;
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

	void t_print(const char str[256],...) {
		unsigned int hold = 0;
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
}

char *convert(unsigned int num, int base) {
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

size_t strlen(const char *str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

int strcmp(const char *a, const char *b) {
	while(*a && *a == *b) {
		a++;
		b++;
	}
	return (int)(unsigned char)(*a) - (int)(unsigned char)(*b);
}

char *strcpy(char *dest, const char *src) {
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

void update_cursor(size_t terminal_row, size_t terminal_column) {
	unsigned short position = terminal_row * 80 + terminal_column;
     	outb(0x3D4, 0x0F);
     	outb(0x3D5, (unsigned char)(position & 0xFF));
     	outb(0x3D4, 0x0E);
     	outb(0x3D5, (unsigned char )((position >> 8) & 0xFF));
}

inline uint8_t vga_entry_color(uint8_t foreground, uint8_t background) {
	return foreground | background << 4;
}

inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
