#ifndef TTY_H_
#define TTY_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
    size_t x_pos;
    size_t y_pos;
    size_t loc;
    uint32_t background;
    int tabsize; 
    char *grid;
} tty_t;

extern tty_t *current_tty;

#endif
