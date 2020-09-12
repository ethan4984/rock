#include <kernel/drivers/mouse.h>
#include <lib/vesa.h>

namespace kernel {

uint8_t cursorPalate[] = {
                    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0,
                    2, 1, 1, 2, 0, 2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0,
                    2, 1, 2, 0, 0, 0, 2, 1, 1, 1, 1, 1, 2, 0, 0, 0,
                    2, 2, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 2, 0, 0,
                    2, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 2, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 2,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 2, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0,
                   };

int mouseX = 512;
int mouseY = 512;

void mouseHandler(regs_t *regs) {
    static VesaShape cursor(512, 512, &*cursorPalate , 16, 16, 0xffff); 
    
    uint8_t status = inb(0x64);
    if(status != 0x3D) { // check wheather the data on port 0x60 is for the mouse of the keyboard
        return;
    }

    uint8_t flags = inb(0x60);
    uint8_t deltaX = inb(0x60);
    uint8_t deltaY = inb(0x60);

    int x = deltaX - ((flags << 4) & 0x100);
    int y = deltaY - ((flags << 3) & 0x100);

    if((x != 65 && y != 65) && (x != 97 && y != -159)) {
        mouseX += x / mouseSensitivity;
        mouseY -= y / mouseSensitivity;

        if(mouseX >= (int)vesa.width)
            mouseX -= x / mouseSensitivity;
        if(mouseY >= (int)vesa.height)
            mouseY += y / mouseSensitivity;
        if(mouseX < 0)
            mouseX -= x / mouseSensitivity;
        if(mouseY < 0)
            mouseY += y / mouseSensitivity;
    }

    cursor.redraw(mouseX, mouseY);
}

void initMouse() {
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

}
