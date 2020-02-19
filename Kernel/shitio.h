#pragma once

#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08
#define ENTER_KEY_CODE 0x1Ca

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "port.h"

size_t strlen(const char *str);

static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

void initalize();

void k_print(char str[256],...);

void t_print(char str[256],...);

void putchar(char c);

/*static const size_t VGA_WIDTH = 80;

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
};*/

//inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);

//inline uint16_t vga_entry(unsigned char uc, uint8_t color);

//void terminal_setcolor(uint8_t color);

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

//void update_cursor(size_t terminal_row, size_t terminal_column);

extern "C" void gdt_flush(uint32_t);
