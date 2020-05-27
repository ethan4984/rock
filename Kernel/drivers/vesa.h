#pragma once

#include <Kernel/stivale.h>

void initVesa(stivaleInfo_t *bootInfo);

void setPixel(uint16_t x, uint16_t y, uint32_t colour);

void drawLineH(uint16_t x, uint16_t x1, uint16_t y, uint32_t colour);

void drawLineV(uint16_t y, uint16_t y1, uint16_t x, uint32_t colour);
