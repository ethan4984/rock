#include <bmp.h>
#include <output.h>
#include <memutils.h>
#include <graphics.h>
#include <fs/fd.h>

uint32_t bmp_get_pixel(background_image_t *bmp, uint32_t x, uint32_t y) {
    bmp_hdr_t *header = &bmp->hdr;

    x %= header->width;
    y %= header->height;

    size_t pixel_offset = bmp->pitch * (header->height - y - 1) + x * (header->bpp / 8);

    uint32_t composite = 0;
    for (int i = 0; i < header->bpp / 8; i++)
        composite |= (uint32_t)bmp->image[pixel_offset + i] << (i * 8);

    return composite;
}

int bmp_draw(char *str, background_image_t *ret, uint32_t x, uint32_t y) {
    int fd = open(str, 0);
    bmp_hdr_t bmp;
    read(fd, &bmp, sizeof(bmp_hdr_t));

    if(bmp.signature != 0x4d42)
        return -1;

    *ret = (background_image_t) {   .image = (uint8_t*)(pmm_alloc(ROUNDUP(bmp.file_size, 0x1000)) + HIGH_VMA),
                                    .hdr = bmp,
                                    .pitch = ALIGN_UP(bmp.width * bmp.bpp, 32) / 8,
                                    .x = x,
                                    .y = y
                                };

    kvprintf("Background height: %d\n", bmp.height);
    kvprintf("Background height: %d\n", bmp.width);
    kvprintf("Background height: %d\n", bmp.bpp);

    lseek(fd, bmp.data_offset, SEEK_SET);

    read(fd, ret->image, bmp.file_size);

    for(uint32_t x = 0; x < bmp.width; x++) {
        for(uint32_t y = 0; y < bmp.height; y++) {
            set_pixel(x, y, bmp_get_pixel(ret, x, y));
        }
    }

    return 0;
}
