#ifndef __DRIVERS_TTY_HPP_
#define __DRIVERS_TTY_HPP_

#include <stivale.hpp>
#include <fs/vfs.hpp>
#include <map.hpp>

namespace tty {

constexpr size_t default_tab_size = 4;
constexpr size_t default_text_fg = 0xffffff;
constexpr size_t default_text_bg = 0;
constexpr size_t default_cursor_fg = default_text_fg;
constexpr size_t max_escape_size = 256;

struct [[gnu::packed]] winsize {
    uint16_t ws_row;
    uint16_t ws_col;
    uint16_t ws_xpixel;
    uint16_t ws_ypixel;
};

constexpr size_t tiocginsz = 0x5413;

class tty;

class tty_ioctl : public vfs::default_ioctl {
public:
    tty_ioctl(tty &tty_cur) : tty_cur(tty_cur) { }

    int call(regs *regs_cur);
private:
    tty &tty_cur;
};

class screen {
public:
    screen(stivale *stivale_struct);
    screen() = default;

    void set_pixel(ssize_t x, ssize_t y, uint32_t colour);
    uint32_t get_pixel(ssize_t x, ssize_t y);
    void flush(uint32_t colour);

    friend tty;
    friend tty_ioctl;
private:
    volatile uint32_t *framebuffer;
    volatile uint32_t *double_buffer;

    uint32_t pitch;
    uint32_t width; 
    uint32_t height;
    uint32_t bpp;
    uint32_t size;
};

class console_code {
public:
    console_code(tty *parent);
    console_code() = default;

    void add_character(uint8_t c);
    bool validate(uint8_t c);
private:
    bool control_sequence;
    bool dec_private_mode;
    bool decckm;
    bool rrr;

    int escape_grid[max_escape_size];
    int grid_index;

    ssize_t saved_cursor_x;
    ssize_t saved_cursor_y;
    ssize_t scrolling_region_top;
    ssize_t scrolling_region_bottom;

    tty *parent;

    void action_cuu();
    void action_cud();
    void action_cuf();
    void action_cub();
    void action_cnl();
    void action_cpl();
    void action_cha();
    void action_cup();
    void action_ed();
    void action_vpa();
    void action_decstbm();
    void action_u();
    void action_s();
    void action_sm();
    void action_rm();
    void action_sgr();
};

class tty : vfs::fs {
public:
    tty(screen &sc, uint8_t *font, size_t font_height, size_t font_width);
    tty() = default;

    void render_char(ssize_t x, ssize_t y, uint32_t fg, uint32_t bg, char c);
    void plot_char(ssize_t x, ssize_t y, uint32_t fg, uint32_t bg, char c);
    void update_cursor(ssize_t x, ssize_t y);
    void putchar(char c);
    void parse_escape_sequence(unsigned char c);
    void parse_control_sequence(unsigned char c);
    void clear();
    void scroll();

    int raw_open(vfs::node *vfs_node, uint16_t status);
    int raw_read(vfs::node *vfs_node, off_t off, off_t cnt, void *buf);
    int raw_write(vfs::node *vfs_node, off_t off, off_t cnt, void *buf);

    tty_ioctl *ioctl_device;

    friend tty_ioctl;
    friend console_code;
private:
    void draw_cursor();
    void clear_cursor();

    uint32_t cursor_foreground;
    uint32_t text_foreground;
    uint32_t text_background; 

    ssize_t cursor_x;
    ssize_t cursor_y;
    ssize_t tab_size;

    uint8_t *font;
    size_t font_height;
    size_t font_width;

    ssize_t rows;
    ssize_t cols;

    screen sc;

    char *char_grid;
    volatile char last_char;

    bool new_key;
    bool escape;
    console_code escape_sequence;
};

void ps2_keyboard(regs *regs_cur, void*);

inline lib::map<ssize_t, tty*> tty_list;
inline ssize_t current_tty = 0;

inline void putchar(char c) {
    if(current_tty == -1) 
        return;
    tty_list[current_tty]->putchar(c);
}

}

#endif
