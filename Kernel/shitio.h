#pragma once

#include <port.h>
#include <stdint.h>
#include <stddef.h>

/* colour constants */

#define VGA_BLACK 0
#define VGA_BLUE 1
#define VGA_GREEN 2
#define VGA_CYAN 3
#define VGA_RED 4
#define VGA_MAGENTA 5
#define VGA_BROWN 6
#define VGA_LIGHT_GREY 7
#define VGA_DARK_GREY 8
#define VGA_LIGHT_BLUE 9
#define VGA_LIGHT_GREEN 10
#define VGA_LIGHT_CYAN 11
#define VGA_LIGHT_RED 12
#define VGA_LIGHT_MAGENTA 13
#define VGA_LIGHT_BROWN 14
#define VGA_WHITE 15

size_t strlen(const char *str);

int strcmp(const char *a, const char *b);

char *strcpy(char *dest, const char *src);

size_t utoa(uint64_t n, char *s, int base);

namespace standardout
{
    static uint16_t *const VGA_MEMORY = (uint16_t*)0xB8000;

    void initalize(uint8_t bg, uint8_t fg);

    void k_print(const char str[256],...);

    void t_print(const char str[256],...);

    void s_print(uint8_t color, size_t x, size_t y, const char str[256],...);

    void putchar(char c);

    void special_char(char c, size_t x, size_t y, uint8_t fg, uint8_t bg = 17);

    void special_num(int num, int size, size_t x, size_t y, uint8_t fg, uint8_t bg = 17);

    void clear_screen();

    void clear_promnt();

    bool end_of_terminal();

    bool end_of_screen(size_t offset);

    bool terminal_setcolor(uint8_t background, uint8_t text);

    void disable_cursor();

    void enable_cursor();

    int count_digits(int num);

    int grab_current_y();
}

char *convert(uint64_t num, int base);
