#pragma once

#include <stdint.h>

struct [[gnu::packed]] bmpFileHdr_t {
    uint16_t bfType;
    uint32_t bfSize;
    uint32_t reserved;
    uint32_t bfOffset;

    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelPerMeter;
    uint32_t biYPelPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
};

struct bmpImage_t {
    bmpFileHdr_t hdr;
    uint32_t *data;
};

uint32_t bmpGetPixel(uint32_t x, uint32_t y, bmpImage_t bmpImage);

bmpImage_t drawBMP(const char *path); 
