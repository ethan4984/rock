#pragma once

#include <stdint.h>

class IDTentries
{
    public:
        uint16_t lowOffset;
        uint16_t selector;
        uint8_t zero8;
        uint8_t flags;
        uint16_t middleOffset;
        uint32_t highOffset;
        uint32_t zero32;
} __attribute__((packed));

class IDTregister
{
    public:
        uint16_t limit;
        uint64_t base;
} __attribute__((packed));

void setIdtGate(uint64_t vectorNum, uint8_t flags);

void callHandlers(uint8_t IRQnum);

void maskGate(uint8_t vectorNum);

void clearGate(uint8_t vectorNum);

void idtInit();

extern uint64_t irq0() asm("irq0");
extern uint64_t irq1() asm("irq1");
extern uint64_t irq2() asm("irq2");
extern uint64_t irq3() asm("irq3");
extern uint64_t irq4() asm("irq4");
extern uint64_t irq5() asm("irq5");
extern uint64_t irq6() asm("irq6");
extern uint64_t irq7() asm("irq7");
extern uint64_t irq8() asm("irq8");
extern uint64_t irq9() asm("irq9");
extern uint64_t irq10() asm("irq10");
extern uint64_t irq11() asm("irq11");
extern uint64_t irq12() asm("irq12");
extern uint64_t irq13() asm("irq13");
extern uint64_t irq14() asm("irq14");
extern uint64_t irq15() asm("irq15");

extern void isr0() asm("isr0");
extern void isr1() asm("isr1");
extern void isr2() asm("isr2");
extern void isr3() asm("isr3");
extern void isr4() asm("isr4");
extern void isr5() asm("isr5");
extern void isr6() asm("isr6");
extern void isr7() asm("isr7");
extern void isr8() asm("isr8");
extern void isr9() asm("isr9");
extern void isr10() asm("isr10");
extern void isr11() asm("isr11");
extern void isr12() asm("isr12");
extern void isr13() asm("isr13");
extern void isr14() asm("isr14");
extern void isr15() asm("isr15");
extern void isr16() asm("isr16");
extern void isr17() asm("isr17");
extern void isr18() asm("isr18");
extern void isr19() asm("isr19");
extern void isr20() asm("isr20");
