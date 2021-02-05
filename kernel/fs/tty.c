#include <fs/tty.h>
#include <graphics.h>
#include <vec.h>

static_vec(tty_t, tty_list);
tty_t *current_tty;

static void tty_putchar(tty_t *tty, char c) {
    switch(c) {
        case '\n':
            if(tty->y_pos + 16 == fb_height) {
                scroll(tty->background, 16);
            } else {
                tty->y_pos += 16;
            }
            tty->x_pos = 0;
            break;
        case '\t':
            for(int i = 0; i < tty->tabsize; i++)
                tty_putchar(tty, ' ');
            break;
        default:
            render_char(tty->x_pos, tty->y_pos, 0xffffffff, c);
            if(tty->x_pos + 8 >= fb_width)
                tty_putchar(tty, '\n');
            else
                tty->x_pos += 8;
    }
}
