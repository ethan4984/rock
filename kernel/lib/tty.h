#ifndef TTY_H_
#define TTY_H_

#include <stivale.h>
#include <vec.h>

#define TTY_FONT_WIDTH 8
#define TTY_FONT_HEIGHT 0x10

struct tty {
    char *path;
    size_t number;
    size_t x;
    size_t y;
    size_t height;
    size_t width;
    size_t cursor_x;
    size_t cursor_y;
    size_t column_num;
    size_t row_num;
    uint32_t foreground;
    uint32_t background;
    char *grid;
    int tab_size;
};

extern_hash_table(struct tty, tty_list);

void tty_init(struct stivale *stivale);
void set_pixel(uint32_t x, uint32_t y, uint32_t colour);
void tty_print_char(struct tty *tty, char c);
uint32_t get_pixel(uint32_t x, uint32_t y);

#endif
