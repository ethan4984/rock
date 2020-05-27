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

void maskClear(uint8_t vectorNum);

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
