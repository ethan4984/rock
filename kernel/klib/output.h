#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <stdarg.h>

#include <asmutils.h>

#define COM1 0x3f8 
#define COM2 0x2f8
#define COM3 0x3e8
#define COM4 0x2e8

#define TAB_SIZE 4
#define FOREGROUND_COLOUR 0xff69b4
#define BACKGROUND_COLOUR 0xffffff

enum {
    DEFAULT,
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    LIGHTGRAY,
    RIGHTGRAY,
    LIGHTRED,
    LIGHTGREEN,
    LIGHTYELLOW,
    LIGHTBLUE,
    LIGHTMAGENTA,
    LIGHTCYAN,
    WHITE
};

uint8_t serial_read();

void serial_write(uint8_t data);

void serial_write_str(const char *str);

void print_args(const char *str, va_list arg, void (*fp)(uint8_t));

void kprintf(const char *prefix, const char *str, ...);

void kvprintf(const char *str, ...);

void kpanic(const char *message, ...);

void g_putchar(uint8_t c);

#endif
