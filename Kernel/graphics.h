#pragma once

void draw_hline(uint8_t color, size_t y, size_t start, size_t size);

void draw_vline(uint8_t color, size_t x, size_t start, size_t size);

void draw_pixels(uint32_t pixels[], int size, size_t offset_x, size_t offset_y);

void sprit_draw_main();
