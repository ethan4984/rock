#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "port.h"

/* colour constants */

#define VGA_BLACK 0
#define VGA_BLUE 1
#define VGA_REEN 2
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

namespace standardout {
	static uint16_t *const VGA_MEMORY = (uint16_t*)0xB8000;

	void initalize();

	void k_print(const char str[256],...);

	void t_print(const char str[256],...);

	void putchar(char c);

	void clear_screen();

	bool end_of_terminal();

	bool terminal_setcolor(uint8_t background, uint8_t text);
}
