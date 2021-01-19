#ifndef BMP_H_
#define BMP_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint16_t signature;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;

    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t image_size;
    uint32_t x_cnt;
    uint32_t y_cnt;
    uint32_t colours_used;
    uint32_t important_colours;
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
} __attribute__((packed)) bmp_hdr_t;

typedef struct {
    uint8_t *image;
    uint32_t pitch;
    uint32_t x;
    uint32_t y;
    bmp_hdr_t hdr;
} background_image_t;

uint32_t bmp_get_pixel(background_image_t *bmp, uint32_t x, uint32_t y);
int bmp_draw(char *path, background_image_t *ret, uint32_t x, uint32_t y);

#endif

