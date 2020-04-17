#include <shitio.h>
#include <port.h>
#include <memory.h>
#include <mouse.h>
#include <vesa.h>
#include <vesa.h>

#define mouse_sensitivity 3

using namespace standardout;

int64_t mouse_x = 512;
int64_t mouse_y = 512;

uint32_t bruh[4][2] = { { 512, 512 }, { 512, 532 }, { 532, 512 }, { 532, 532 } };

widget cursor(bruh, 4, 0xffffffff, 0xffffffff);

void mouse_handler() {
    outb(0xA0, 0x20); // EOI
    outb(0x20, 0x20);

    uint8_t extra_bits = inb(0x60);
    uint8_t change_in_x = inb(0x60);
    uint8_t change_in_y = inb(0x60);

    int x = (int)change_in_x - (int)((extra_bits << 4) & 0x100);
    int y = (int)change_in_y - (int)((extra_bits << 3) & 0x100);

    int64_t old_x = mouse_x;
    int64_t old_y = mouse_y;

    if((x != 65 && y != 65) && (x != 97 && y != -159)) {
        mouse_x += x / mouse_sensitivity;
        mouse_y -= y / mouse_sensitivity;

        if(mouse_x >= 1024)
            mouse_x -= x / mouse_sensitivity;
        if(mouse_y >= 768)
            mouse_y += y / mouse_sensitivity;
        if(mouse_x < 0)
            mouse_x -= x / mouse_sensitivity;
        if(mouse_y < 0)
            mouse_y += y / mouse_sensitivity;
    }

    int64_t diffrence_x = mouse_x - old_x;
    int64_t diffrence_y = mouse_y - old_y;

    if(diffrence_x > 0)
        for(int i = 0; i < diffrence_x; i++)
            cursor.move_right(bruh, 4);

    if(diffrence_x < 0)
        for(int i = 0; i < old_x - mouse_x; i++)
            cursor.move_left(bruh, 4);

    if(diffrence_y > 0)
        for(int i = 0; i < diffrence_y; i++)
            cursor.move_down(bruh, 4);

    if(diffrence_y < 0)
        for(int i = 0; i < old_y - mouse_y; i++)
            cursor.move_up(bruh, 4);
}

void mouse_setup()
{
    uint8_t status;

    outb(0x64, 0xA8); // enables aux port 2

    // enables irq12
    outb(0x64, 0x20);
    status = inb(0x60) | 2;
    outb(0x64, 0x60);
    outb(0x60, status);

    outb(0x64, 0xD4);
    outb(0x60, 0xF6); // default settings

    outb(0x64, 0xD4);
    outb(0x60, 0xF4); // enables ps2 mouse
}
