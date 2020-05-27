#include <Kernel/drivers/vesa.h>
#include <Slib/videoIO.h>

using namespace out;
    
uint32_t fbHeight, fbWidth, fbPitch, fbBpp;
uint32_t *framebuffer;

void initVesa(stivaleInfo_t *bootInfo)
{
    framebuffer = (uint32_t*)bootInfo->framebufferAddr;
    fbHeight = bootInfo->framebufferHeight;
    fbWidth = bootInfo->framebufferWidth;
    fbPitch = bootInfo->framebufferPitch;
    fbBpp = bootInfo->framebufferBpp;

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
