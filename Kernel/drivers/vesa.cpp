#include <Kernel/drivers/vesa.h>
#include <Kernel/mm/memHandler.h>
#include <Kernel/sched/scheduler.h>
#include <Slib/videoIO.h>
#include <Slib/memoryUtils.h>

using namespace out;

#define SETPIXEL(X, Y, COLOUR) *(volatile uint32_t*)((uint64_t)framebuffer + (((Y) * fbPitch) + ((X) * fbBpp / 8))) = (COLOUR)
#define GRABCOLOUR(X, Y)  *(volatile uint32_t*)((uint64_t)framebuffer + (((Y) * fbPitch) + ((X) * fbBpp / 8)))
    
uint32_t fbHeight, fbWidth, fbPitch, fbBpp, fbSize, fbPixels;
uint32_t *framebuffer;

void memcpy32(uint32_t *src, uint32_t *dst, uint64_t count) {
    for (uint64_t i = 0; i<count; i++)
        *dst++ = *src++;
}

void initVesa(stivaleInfo_t *bootInfo)
{
    framebuffer = (uint32_t*)bootInfo->framebufferAddr;
    fbHeight = bootInfo->framebufferHeight;
    fbWidth = bootInfo->framebufferWidth;
    fbPitch = bootInfo->framebufferPitch;
    fbBpp = bootInfo->framebufferBpp;
    fbPixels = fbHeight * fbWidth;
    fbSize =  fbHeight * fbPitch;

    cPrint("Framebuffer address: %x",  framebuffer);
    cPrint("Framebuffer Height: %d",  fbHeight);
    cPrint("Framebuffer Width: %d",  fbWidth);
    cPrint("Framebuffer pitch: %d",  fbPitch);
    cPrint("Framebuffer BPP: %d\n",  fbBpp);
}


void setPixel(uint16_t x, uint16_t y, uint32_t colour)
{
    *(volatile uint32_t*)((uint64_t)framebuffer + ((y * fbPitch) + (x * fbBpp / 8))) = colour;
}

void drawLineH(uint16_t x, uint16_t x1, uint16_t y, uint32_t colour)
{
    for(int i = x; i < x1 + 1; i++)
        setPixel(i, y, colour);
}

void drawLineV(uint16_t y, uint16_t y1, uint16_t x, uint32_t colour)
{
    for(int i = y; i < y1 + 1; i++)
        setPixel(x, i, colour);
}


widget::widget(int x_t, int y_t, int height_t, int width_t, uint32_t colour_t) : x(x_t), y(y_t), height(height_t), width(width_t), colour(colour_t) 
{
    backgroundFB = (uint32_t*)malloc(width_t * height_t * 4);

    for(int i = x_t; i < x_t + height_t; i++) {
        for(int j = y_t; j < y_t + width_t; j++) {
            backgroundFB[backgroundFBsize++] = GRABCOLOUR(i, j);
            SETPIXEL(i, j, colour_t);
        }
    }
}

void widget::moveRight() 
{
    //   x++;
}

void widget::moveLeft() 
{
    //    x--;
}

void widget::moveUp() /* works */
{

}

void widget::moveDown() /* works */
{
    //    y++;
}

void testBruh() {
    SETPIXEL(20, 20, 0xfffffff);
}

void renderChar(uint64_t x, uint64_t y, uint32_t fg, uint32_t bg, char c)
{
    for(uint8_t i = 0; i < 8; i++) {
        for(uint8_t j = 0; j < 8; j++) {
            if((font[(uint8_t)c][i] >> j) & 1) {
                uint64_t offset = ((i + y) * fbPitch) + ((j + x) * 4);
                *(uint32_t*)((uint64_t)framebuffer + offset) = fg;
            }
            else {
                uint64_t offset = ((i + y) * fbPitch) + ((j + x) * 4);
                *(uint32_t*)((uint64_t)framebuffer + offset) = bg;
            }
        }
    }
}

void vesaScroll(uint64_t rows_shift, uint32_t bg) 
{
    uint32_t color_dat = bg;
    uint64_t buf_pos = (uint64_t)framebuffer;
    /* Do the copy */
    memcpy32((uint32_t*) (buf_pos + (fbPitch * rows_shift)), (uint32_t *) (buf_pos), 
        fbPixels - ((fbPitch * rows_shift) / 4));
    /* Do the clear */
    memset32((uint32_t*) (buf_pos + fbSize - (fbPitch * rows_shift)),
        bg, (fbPitch * rows_shift) / 4); 
}
