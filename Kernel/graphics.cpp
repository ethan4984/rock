#include <shitio.h>
#include <memory.h>
#include <interrupt.h>
#include <scheduler.h>

using namespace standardout;

uint32_t bruh[] =   {
                        8, 1, VGA_LIGHT_BLUE,
                        7, 2, VGA_LIGHT_BLUE,
                        8, 2, VGA_LIGHT_BLUE,
                        9, 2, VGA_LIGHT_BLUE,
                        7, 3, VGA_LIGHT_BLUE,
                        8, 3, VGA_LIGHT_BLUE,
                        9, 3, VGA_LIGHT_BLUE,
                        6, 4, VGA_BLUE,
                        7, 4, VGA_BLUE, /* 27 */
                        8, 4, VGA_BLUE,
                        9, 4, VGA_BLUE,
                        10, 4, VGA_BLUE, /* 36 */
                        5, 5, VGA_RED,
                        6, 5, VGA_BLUE,
                        7, 5, VGA_LIGHT_BLUE, /* 45 */
                        8, 5, VGA_BLUE,
                        9, 5, VGA_LIGHT_BLUE,
                        10, 5, VGA_BLUE,
                        11, 5, VGA_RED, /* 57 */
                        4, 6, VGA_RED,
                        5, 6, VGA_LIGHT_GREEN,
                        6, 6, VGA_BLUE, /* 66 */
                        7, 6, VGA_BLUE,
                        8, 6, VGA_RED,
                        9, 6, VGA_BLUE, /* 75 */
                        10, 6, VGA_BLUE,
                        11, 6, VGA_LIGHT_GREEN,
                        12, 6, VGA_RED, /* 84 */
                        3, 7, VGA_RED,
                        4, 7, VGA_LIGHT_GREEN,
                        5, 7, VGA_WHITE,
                        6, 7, VGA_BLUE,
                        7, 7, VGA_RED,
                        8, 7, VGA_RED,
                        9, 7, VGA_RED,
                        10, 7, VGA_BLUE,
                        11, 7, VGA_WHITE,
                        12, 7, VGA_LIGHT_GREEN,
                        13, 7, VGA_RED, /* 117 */
                        2, 8, VGA_RED,
                        3, 8, VGA_LIGHT_GREEN,
                        4, 8, VGA_WHITE,
                        5, 8, VGA_BLUE,
                        6, 8, VGA_RED,
                        7, 8, VGA_RED,
                        8, 8, VGA_RED,
                        9, 8, VGA_RED,
                        10, 8, VGA_RED,
                        11, 8, VGA_BLUE,
                        12, 8, VGA_WHITE,
                        13, 8, VGA_LIGHT_GREEN,
                        14, 8, VGA_RED, /* 156 */
                        1, 9, VGA_RED,
                        2, 9, VGA_LIGHT_GREEN,
                        3, 9, VGA_LIGHT_GREEN,
                        4, 9, VGA_BLUE,
                        5, 9, VGA_RED,
                        6, 9, VGA_RED,
                        7, 9, VGA_RED,
                        8, 9, VGA_RED,
                        9, 9, VGA_RED,
                        10, 9, VGA_RED,
                        11, 9, VGA_RED,
                        12, 9, VGA_BLUE,
                        13, 9, VGA_LIGHT_GREEN,
                        14, 9, VGA_LIGHT_GREEN,
                        15, 9, VGA_RED, /* 201 */
                        1, 10, VGA_RED,
                        2, 10, VGA_LIGHT_GREEN,
                        3, 10, VGA_LIGHT_GREEN,
                        4, 10, VGA_BLUE,
                        5, 10, VGA_RED,
                        6, 10, VGA_RED,
                        7, 10, VGA_RED,
                        8, 10, VGA_RED,
                        9, 10, VGA_RED,
                        10, 10, VGA_RED,
                        11, 10, VGA_RED,
                        12, 10, VGA_BLUE,
                        13, 10, VGA_LIGHT_GREEN,
                        14, 10, VGA_LIGHT_GREEN,
                        15, 10, VGA_RED, /* 246 */
                        2, 11, VGA_RED,
                        3, 11, VGA_LIGHT_GREEN,
                        4, 11, VGA_WHITE,
                        5, 11, VGA_BLUE,
                        6, 11, VGA_RED,
                        7, 11, VGA_RED,
                        8, 11, VGA_RED,
                        9, 11, VGA_RED,
                        10, 11, VGA_RED,
                        11, 11, VGA_BLUE,
                        12, 11, VGA_WHITE,
                        13, 11, VGA_LIGHT_GREEN,
                        14, 11, VGA_RED, /* 285 */
                        3, 12, VGA_RED,
                        4, 12, VGA_LIGHT_GREEN,
                        5, 12, VGA_WHITE,
                        6, 12, VGA_BLUE,
                        7, 12, VGA_RED,
                        8, 12, VGA_RED,
                        9, 12, VGA_RED,
                        10, 12, VGA_BLUE,
                        11, 12, VGA_WHITE,
                        12, 12, VGA_LIGHT_GREEN,
                        13, 12, VGA_RED, /* 318 */
                        4, 13, VGA_RED,
                        5, 13, VGA_LIGHT_GREEN,
                        6, 13, VGA_BLUE,
                        7, 13, VGA_BLUE,
                        8, 13, VGA_RED,
                        9, 13, VGA_BLUE,
                        10, 13, VGA_BLUE,
                        11, 13, VGA_LIGHT_GREEN,
                        12, 13, VGA_RED, /* 345 */
                        5, 14, VGA_RED,
                        6, 14, VGA_BLUE,
                        7, 14, VGA_RED,
                        8, 14, VGA_RED,
                        9, 14, VGA_RED,
                        10, 14, VGA_BLUE,
                        11, 14, VGA_RED, /* 366 */
                        6, 15, VGA_BLUE,
                        7, 15, VGA_BLUE,
                        8, 15, VGA_RED,
                        9, 15, VGA_BLUE,
                        10, 15, VGA_BLUE,
                        7, 16, VGA_BLUE,
                        8, 16, VGA_BLUE,
                        9, 16, VGA_BLUE,
                        7, 17, VGA_BLUE,
                        8, 17, VGA_BLUE,
                        9, 17, VGA_BLUE,
                        8, 18, VGA_BLUE, /* 402 */
                    };

void draw_hline(uint8_t color, size_t y, size_t start, size_t size)
{
    for(size_t i = start; i < size; i++)
        special_char(' ', i, y, color, color);
}

void draw_vline(uint8_t color, size_t x, size_t start, size_t size)
{
    for(size_t i = start; i < size; i++)
        special_char(' ', x, i, color, color);
}

void draw_pixels(uint32_t pixels[], int size, size_t offset_x, size_t offset_y)
{
    int i = 0;
    for(int j = 0; j < size/3; j++) {
        special_char(' ', (pixels[i] - 1) + offset_x, (pixels[i+1] - 1) + offset_y, pixels[i+2], pixels[i+2]);
        i += 3;
    }
}

void sprit_draw_main()
{
    disable_cursor();

    mask_irq(1); /* disables irq1/keyboard */

    draw_vline(VGA_MAGENTA, 3, 0, 25);
    draw_hline(VGA_MAGENTA, 3, 0, 80);
    s_print(VGA_LIGHT_BLUE, 10, 1, "This is crepOS");

    draw_pixels(bruh, 402, 10, 5);

    for(int i = 4; i > 0; i--) {
        if(i == 0) {
            special_char('0', 1, 1, VGA_MAGENTA);
            sleep(1);
            continue;
        }
        special_num(i, count_digits(i), 1, 1, VGA_MAGENTA);
        sleep(1);
    }

    clear_irq(1);

    enable_cursor();
}
