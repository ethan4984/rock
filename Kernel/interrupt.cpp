#include <interrupt.h>
#include <port.h>
#include <shitio.h>

using namespace standardout;

struct IDT_entry IDT[256];

void idt_gate(uint32_t referenceIRQ)
{
    static int counter = 32;

    uint32_t irqX = referenceIRQ;
    IDT[counter].offset_low = irqX & 0xffff;
    IDT[counter].selector = 0x08;
    IDT[counter].zero = 0;
    IDT[counter].type_attr = 0x8e;
    IDT[counter].offset_high = (irqX & 0xffff0000) >> 16;

    counter++;
}

void idt_init(void)
{
    uint32_t idt_address;
    uint32_t idt_ptr[2];

    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 40);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    idt_gate((uint32_t)time_handler);
    idt_gate((uint32_t)keyboard_handler);
    idt_gate((uint32_t)irq2);
    idt_gate((uint32_t)irq3);
    idt_gate((uint32_t)irq4);
    idt_gate((uint32_t)irq5);
    idt_gate((uint32_t)irq6);
    idt_gate((uint32_t)irq7);
    idt_gate((uint32_t)irq8);
    idt_gate((uint32_t)irq9);
    idt_gate((uint32_t)irq10);
    idt_gate((uint32_t)irq11);
    idt_gate((uint32_t)irq12);
    idt_gate((uint32_t)irq13);
    idt_gate((uint32_t)irq14);
    idt_gate((uint32_t)irq15);

    idt_address = (uint32_t)IDT ;
    idt_ptr[0] = (sizeof (struct IDT_entry) * 256) + ((idt_address & 0xffff) << 16);
    idt_ptr[1] = idt_address >> 16 ;

    k_print("IDT: initalized at %x\n", idt_address);

    load_idt(idt_ptr);
}

extern "C" void gdt_info(uint32_t addr)
{
    k_print("GDT: mapped to %x\n", addr);
}

extern "C" void irq_l(void)
{
    outb(0x20, 0x20);
}

extern "C" void irq_h(void)
{
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

extern "C" void PITI()
{
    outb(0x20, 0x20);
}

void panic(const char *message, const char *proccess)
{
    asm volatile("cli"); //disable all interrupts
    standardout::initalize(VGA_LIGHT_GREY, VGA_RED);
    standardout::k_print("PANIC : fatal error : %s : in %s\n", message, proccess);
    for(;;);
}

extern void save_regs(void) asm("save_regs");
extern void save_regs16(void) asm("save_regs16");
extern void save_segment(void) asm("save_segment");

struct genregs
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
} gen_reg;

struct genregs16
{
    uint16_t ax;
    uint16_t bx;
    uint16_t cx;
    uint16_t dx;

    uint16_t di;
    uint16_t si;
    uint16_t sp;
    uint16_t bp;
} gen_reg16;

struct segments
{
    uint16_t ss;
    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
} segment;

void reg_flow()
{
    save_regs();
    k_print("\nREGISTER DUMP\n");
    k_print("Dump: EAX %x\n", gen_reg.eax);
    k_print("Dump: EBX %x\n", gen_reg.ebx);
    k_print("Dump: ECX %x\n", gen_reg.ecx);
    k_print("Dump: EDX %x\n", gen_reg.edx);
    k_print("Dump: ESP %x\n", gen_reg.esp);
    k_print("Dump: EBP %x\n", gen_reg.ebp);
    k_print("Dump: EDI %x\n", gen_reg.edi);
    k_print("Dump: ESI %x", gen_reg.esi);
}

void reg_flow16()
{
    save_regs16();
    k_print("\nREGISTER DUMP\n");
    k_print("Dump: AX %x\n", gen_reg16.ax);
    k_print("Dump: BX %x\n", gen_reg16.bx);
    k_print("Dump: CX %x\n", gen_reg16.cx);
    k_print("Dump: DX %x\n", gen_reg16.dx);
    k_print("Dump: SP %x\n", gen_reg16.sp);
    k_print("Dump: BP %x\n", gen_reg16.bp);
    k_print("Dump: DI %x\n", gen_reg16.di);
    k_print("Dump: SI %x", gen_reg16.si);
}

void seg_flow()
{
    save_segment();
    k_print("\nSEGMENT DUMP\n");
    k_print("Dump SS %x\n", segment.ss);
    k_print("Dump CS %x\n", segment.cs);
    k_print("Dump DS %x\n", segment.ds);
    k_print("Dump ES %x\n", segment.es);
    k_print("Dump FS %x\n", segment.fs);
    k_print("Dump GS %x", segment.gs);
}
