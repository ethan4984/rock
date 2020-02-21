#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "port.h"


size_t strlen(const char *str);

static uint16_t *const VGA_MEMORY = (uint16_t*)0xB8000;

void initalize();

void k_print(char str[256],...);

void t_print(char str[256],...);

void putchar(char c);

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

size_t strlen(const char *str);

extern "C" void gdt_flush(uint32_t);
