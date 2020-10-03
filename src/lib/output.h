#pragma once

#include <lib/memoryUtils.h>

#include <stdint.h>
#include <stdarg.h>

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

class coutBase {
public:
    coutBase &operator<<(const char *str);
    coutBase &operator<<(uint64_t number);
    coutBase &operator+(const char *str);
};

inline coutBase cout;

void kprintDS(const char *prefix, const char *str, ...);

void printArgs(const char *str, va_list arg, void (*fp)(uint8_t));
