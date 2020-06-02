#include <Kernel/drivers/vesa.h>
#include <Slib/ports.h>
#include <Slib/videoIO.h>

using namespace out;

#define mouse_sensitivity 3

int64_t mouseX = 512;
int64_t mouseY = 512;

extern "C" void mouseHandler() 
{
    outb(0xA0, 0x20); // EOI
    outb(0x20, 0x20);

    static widget mouseCursor(512, 512, 20, 20, 0xffffff);
    static widget bruh(512, 300, 5, 5, 0xff00000);

    uint8_t extraBits = inb(0x60);
    uint8_t changeX = inb(0x60); // change in X 
    uint8_t changeY = inb(0x60); // change in Y 

    int x = (int)changeX - (int)((extraBits << 4) & 0x100);
    int y = (int)changeY - (int)((extraBits << 3) & 0x100);

    int64_t old_x = mouseX;
    int64_t old_y = mouseY;

    if((x != 65 && y != 65) && (x != 97 && y != -159)) {
        mouseX += x / mouse_sensitivity;
        mouseY -= y / mouse_sensitivity;

        if(mouseX >= 1024)
            mouseX -= x / mouse_sensitivity;
        if(mouseY >= 768)
            mouseY += y / mouse_sensitivity;
        if(mouseX < 0)
            mouseX -= x / mouse_sensitivity;
        if(mouseY < 0)
            mouseY += y / mouse_sensitivity;
    }

    int64_t diffrenceX = mouseX - old_x;
    int64_t diffrenceY = mouseY - old_y;

    asm volatile ("cli");

    if(diffrenceX > 0)
        for(int i = 0; i < diffrenceX; i++)
            mouseCursor.moveRight();

    static bool spaz = false;

    if(diffrenceX < 0) {
        for(int i = 0; i < old_x - mouseX; i++) {
            if(spaz)
                mouseCursor.moveLeft();
        }
    }

    spaz = true;

   if(diffrenceY > 0)
        for(int i = 0; i < diffrenceY; i++) 
            mouseCursor.moveDown();

    if(diffrenceY < 0)
        for(int i = 0; i < old_y - mouseY; i++)
            mouseCursor.moveUp();

    asm volatile ("sti");
}

void mouseSetup()
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
