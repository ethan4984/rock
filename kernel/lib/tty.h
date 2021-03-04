#ifndef TTY_H_
#define TTY_H_

#include <stivale.h>

struct tty {
    
};

void tty_init(struct stivaie *stivale);
void set_pixel(uint32_t x, uint32_t y, uint32_t colour)
uint32_t get_pixel(uint32_t x, uint32_t y);

#endif
