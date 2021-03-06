#include <mm/vmm.h>
#include <memutils.h>
#include <mm/pmm.h>
#include <tty.h>
#include <font.h>
#include <vec.h>
#include <debug.h>

global_hash_table(tty_list);

static uint32_t *framebuffer, fb_height, fb_width, fb_pitch, fb_bpp, *double_buffer, fb_size;

void set_pixel(uint32_t x, uint32_t y, uint32_t colour) {
    *(volatile uint32_t*)(((uint64_t)framebuffer + HIGH_VMA) + ((y * fb_pitch) + (x * fb_bpp / 8))) = colour;
    *(volatile uint32_t*)(((uint64_t)double_buffer) + ((y * fb_pitch) + (x * fb_bpp / 8))) = colour;
}

uint32_t get_pixel(uint32_t x, uint32_t y) {
    return *(volatile uint32_t*)(((uint64_t)double_buffer) + ((y * fb_pitch) + (x * fb_bpp / 8)));
}

void render_char(uint32_t x, uint32_t y, uint32_t fg, char c) {
    uint16_t offset = ((uint8_t)c - 0x20) * 16;
    for(uint8_t i = 0, i_cnt = 8; i < 8 && i_cnt > 0; i++, i_cnt--) {
        for(uint8_t j = 0; j < 16; j++) {
            if((font_bitmap[offset + j] >> i) & 1)
                set_pixel(x + i_cnt, y + j, fg);
        }
    }
}

static void clear_screen(uint32_t bg) {
    memset32((void*)framebuffer + HIGH_VMA, bg, fb_size / 4);
    memset32(double_buffer, bg, fb_size / 4);
}

static void tty_add_char(struct tty *tty, char c) {
    size_t offset = (tty->cursor_y / TTY_FONT_HEIGHT) * tty->row_num + (tty->cursor_x / TTY_FONT_WIDTH);
    tty->grid[offset] = c;

    render_char(tty->cursor_x + tty->x, tty->cursor_y + tty->y, tty->foreground, c);
    if(tty->cursor_x > tty->width)
        tty_print_char(tty, '\n');
    else
        tty->cursor_x += TTY_FONT_WIDTH;
}

static inline void tty_plot_char(struct tty *tty, size_t x, size_t y, char c) {
    render_char(x * TTY_FONT_WIDTH, y * TTY_FONT_HEIGHT, tty->foreground, c);
}

static inline char tty_get_char(struct tty *tty, size_t x, size_t y) {
    size_t offset = y * tty->row_num + x;
    return tty->grid[offset];
}

static inline void tty_set_char(struct tty *tty, size_t x, size_t y, char c) {
    size_t offset = y * tty->row_num + x;
    tty->grid[offset] = c;
}

static void tty_scroll(struct tty *tty) {
    clear_screen(tty->background);
    for(size_t i = 0; i < tty->column_num; i++) {
        for(size_t j = 0; j < tty->row_num; j++) {
            tty_set_char(tty, j, i, tty_get_char(tty, j, i + 1));
            tty_plot_char(tty, j, i, tty_get_char(tty, j, i));
        }
    }
}

void tty_print_char(struct tty *tty, char c) {
    switch(c) {
        case '\n':
            if(tty->cursor_y == tty->height) {
                tty_scroll(tty);
            } else {
                tty->cursor_y += TTY_FONT_HEIGHT;
            }

            tty->cursor_x = 0;

            break; 
        case '\t':
            for(int i = 0; i < tty->tab_size; i++)
                tty_add_char(tty, ' ');
            break;
        default:
            tty_add_char(tty, c);
    }
}

void tty_print_string(struct tty *tty, const char *str) {
    for(size_t i = 0; i < strlen(str); i++)
        tty_print_char(tty, str[i]);
}

struct tty *tty_create() {
    size_t index =  hash_push(struct tty, tty_list, ((struct tty) {  .height = fb_height,
                                                                     .width = fb_width,
                                                                     .column_num = fb_height / TTY_FONT_HEIGHT,
                                                                     .row_num = fb_width / TTY_FONT_WIDTH,
                                                                     .grid = kmalloc((fb_height / TTY_FONT_HEIGHT) * (fb_width / TTY_FONT_WIDTH)),
                                                                     .tab_size = 4,
                                                                     .foreground = 0xffffffff,
                                                                     .path = kmalloc(256)
                                                                  }));

    struct tty *tty = hash_search(struct tty, tty_list, index);

    sprintf(tty->path, "/dev/tty%d", 1, index);
    tty->number = index;
    
    open(tty->path, 0);
}

void tty_init(struct stivale *stivale) {
    framebuffer = (uint32_t*)stivale->fb_addr;
    fb_height = stivale->fb_height;
    fb_width = stivale->fb_width;
    fb_bpp = stivale->fb_bpp;
    fb_pitch = stivale->fb_pitch;
    fb_size = fb_height * fb_pitch;
    double_buffer = (uint32_t*)(pmm_calloc(DIV_ROUNDUP(fb_height * fb_width * (fb_bpp / 8), 0x1000)) + HIGH_VMA);
    tty_create();
}
