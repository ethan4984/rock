#include <Kernel/int/pic.h>
#include <Slib/ports.h>
#include <Slib/videoIO.h>

#include <stddef.h>

using namespace out;

IDTentries IDT[256];

IDTregister IDTr;

typedef void (*eventHandlers)();

eventHandlers irqHandler[15] = {    NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL, NULL, NULL, NULL, 
                                    NULL, NULL, NULL, NULL, NULL, 
                                };

void setIDTgate(uint8_t vectorNum, uint8_t flags, uint64_t handler)
{
    IDT[vectorNum].selector = 0x8;
    IDT[vectorNum].zero32 = 0;
    IDT[vectorNum].zero8 = 0;
    IDT[vectorNum].flags = flags;
    IDT[vectorNum].lowOffset = (uint16_t)(handler >> 0);
    IDT[vectorNum].middleOffset = (uint16_t)(handler >> 16);
    IDT[vectorNum].highOffset = (uint32_t)(handler >> 32);
}

void maskGate(uint8_t vectorNum)
{
    uint16_t data;

    if(vectorNum < 8) {
        data = inb(0x20 + 1) | (1 << vectorNum);
        outb(0x20 + 1, data);
    } else {
        vectorNum -= 8;
        data = inb(0xa0 + 1) | (1 << vectorNum);
        outb(0xa0 + 1, data);
    }
}

void clearGate(uint8_t vectorNum) 
{
    uint16_t data;

    if(vectorNum < 8) {
        data = inb(0x20 + 1) & ~(1 << vectorNum);
        outb(0x20 + 1, data);
    } else {
        vectorNum -= 8;
        data = inb(0xa0 + 1) & ~(1 << vectorNum);
        outb(0xa0 + 1, vectorNum);
    }
}

extern "C" void irqHandlers(uint64_t vectorNum)
{
    if(vectorNum < 8) 
        outb(0x20, 0x20);
    else {
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
    }
    
    if(irqHandler[vectorNum] != NULL) 
        irqHandler[vectorNum]();
}

void idtInit()
{
    /* remaps the pic */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    setIDTgate(0, 0x8f, (uint64_t)isr0);
    setIDTgate(1, 0x8f, (uint64_t)isr1);
    setIDTgate(2, 0x8f, (uint64_t)isr2);
    setIDTgate(3, 0x8f, (uint64_t)isr3);
    setIDTgate(4, 0x8f, (uint64_t)isr4);
    setIDTgate(5, 0x8f, (uint64_t)isr5);
    setIDTgate(6, 0x8f, (uint64_t)isr6);
    setIDTgate(7, 0x8f, (uint64_t)isr7);
    setIDTgate(8, 0x8f, (uint64_t)isr8);
    setIDTgate(9, 0x8f, (uint64_t)isr9);
    setIDTgate(10, 0x8f, (uint64_t)isr10);
    setIDTgate(11, 0x8f, (uint64_t)isr11);
    setIDTgate(12, 0x8f, (uint64_t)isr12);
    setIDTgate(13, 0x8f, (uint64_t)isr13);
    setIDTgate(14, 0x8f, (uint64_t)isr14);
    setIDTgate(15, 0x8f, (uint64_t)isr15);
    setIDTgate(16, 0x8f, (uint64_t)isr16);
    setIDTgate(17, 0x8f, (uint64_t)isr17);
    setIDTgate(18, 0x8f, (uint64_t)isr18);
    setIDTgate(19, 0x8f, (uint64_t)isr19);
    setIDTgate(20, 0x8f, (uint64_t)isr20);

    setIDTgate(32, 0x8e, (uint64_t)irq0);
    setIDTgate(33, 0x8e, (uint64_t)irq1);
    setIDTgate(34, 0x8e, (uint64_t)irq2);
    setIDTgate(35, 0x8e, (uint64_t)irq3);
    setIDTgate(36, 0x8e, (uint64_t)irq4);
    setIDTgate(37, 0x8e, (uint64_t)irq5);
    setIDTgate(38, 0x8e, (uint64_t)irq6);
    setIDTgate(39, 0x8e, (uint64_t)irq7);
    setIDTgate(40, 0x8e, (uint64_t)irq8);
    setIDTgate(41, 0x8e, (uint64_t)irq9);
    setIDTgate(42, 0x8e, (uint64_t)irq10);
    setIDTgate(43, 0x8e, (uint64_t)irq11);
    setIDTgate(44, 0x8e, (uint64_t)irq12);
    setIDTgate(45, 0x8e, (uint64_t)irq13);
    setIDTgate(46, 0x8e, (uint64_t)irq14);
    setIDTgate(47, 0x8e, (uint64_t)irq15);

    IDTr.base = (uint64_t)&IDT;
    IDTr.limit = 256 * sizeof(IDTentries) - 1;
    asm volatile("lidtq %0" :: "m"(IDTr));
    cPrint("\nIDT intialized at %x", IDTr);
}
