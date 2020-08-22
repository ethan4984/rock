#pragma once

#include <stdint.h>

namespace kernel {

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

}
