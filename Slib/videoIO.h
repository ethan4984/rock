#pragma once

#include <stdint.h>

void initalizeVESA(uint32_t bg, uint32_t fg, uint64_t x, uint64_t y);

void vesaScroll(uint64_t rows_shift, uint32_t bg); 

namespace out 
{
    void cPrint(const char str[256], ...);    

    void kPrint(const char str[256], ...);

    void putchar(char c);
}
