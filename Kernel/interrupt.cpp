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

void idt_expection(uint32_t call_back, uint32_t over_ride = 0)
{
    static int counter = 0;

    if(over_ride)
        counter = over_ride;

    uint32_t irqX = call_back;
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

    idt_expection((uint32_t)divide_zero);
    idt_expection((uint32_t)debug);
    idt_expection((uint32_t)non_maskable_irq);
    idt_expection((uint32_t)breakpoint);
    idt_expection((uint32_t)overflow);
    idt_expection((uint32_t)bound_range);
    idt_expection((uint32_t)invaild_opcode);
    idt_expection((uint32_t)device_not_available);
    idt_expection((uint32_t)double_fault);
    idt_expection((uint32_t)coprocessor_seg_overrun);
    idt_expection((uint32_t)invaild_tss);
    idt_expection((uint32_t)segment_not_found);
    idt_expection((uint32_t)stack_seg_fault);
    idt_expection((uint32_t)gen_fault);
    idt_expection((uint32_t)page_fault);
    idt_expection((uint32_t)floating_point_fault, 16);
    idt_expection((uint32_t)alignment_check);
    idt_expection((uint32_t)machine_check);
    idt_expection((uint32_t)simd_floating_point);
    idt_expection((uint32_t)vm_expection);
    idt_expection((uint32_t)security_expection);


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

    load_idt(idt_ptr);
}

void mask_irq(unsigned char channel)
{
    uint16_t data;

    if(channel < 8) {
        data = inb(0x20 + 1) | (1 << channel);
        outb(0x20 + 1, data);
    }
    else {
        channel -= 8;
        data = inb(0xa0 + 1) | (1 << channel);
        outb(0xa0 + 1, data);
    }
}

void clear_irq(unsigned char channel)
{
    uint16_t data;

    if(channel < 8) {
        data = inb(0x20 + 1) & ~(1 << channel);
        outb(0x20 + 1, data);
    }
    else {
        channel -= 8;
        data = inb(0xa0 + 1) & ~(1 << channel);
        outb(0xa0 + 1, data);
    }
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

volatile int timer_ticks = 0;
volatile int seconds = 0;

extern "C" void PITI()
{
    outb(0x20, 0x20);
    timer_ticks++;
    if(timer_ticks % 94 == 0)
        ++seconds;
}

extern "C" void panic(const char *message)
{
    initalize(VGA_BLUE, VGA_RED);
    k_print("PANIC : fatal error : %s\n", message);
    reg_flow();
    putchar('\n');
    seg_flow();
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

static inline void pit_send_data(uint16_t data, uint8_t counter)
{
    uint8_t port;
    uint8_t reference_port;

    if(!counter)
        port = 0x40;
    else
        port = reference_port;

    if(counter == 0x40)
        reference_port = 0x40;
    else
        reference_port = 0x42;

    outb (port, (uint8_t)data);
}

void start_counter(int frequency, uint8_t counter, uint8_t mode)
{
    if(!frequency)
        return;

    uint16_t divisor = 1193181/frequency;

    uint8_t ossal = 0;
    ossal = (ossal & ~0xe) | mode;
    ossal = (ossal & ~0x30) | 0xe;
    ossal = (ossal & ~0xc0) | counter;
    outb(0x43, ossal);

    pit_send_data(divisor & 0xff, 0);
    pit_send_data((divisor >> 8) & 0xff, 0);
}

void sleep(int ticks)
{
    seconds = 0;
    while(seconds < ticks)
        asm volatile("nop");
}

