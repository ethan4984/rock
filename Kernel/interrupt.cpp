#include <interrupt.h>
#include <port.h>
#include <shitio.h>
#include <keyboard.h>
#include <process.h>
#include <scheduler.h>
#include <mouse.h>

using namespace standardout;

struct IDT_entry IDT[256];

void irq_l();
void irq_h();

idtr idt_r;

void idt_gate(uint64_t IRQ)
{
    static uint64_t cnt = 32;

    IDT[cnt].selector = 0x8;
    IDT[cnt].zero32 = 0;
    IDT[cnt].zero8 = 0;
    IDT[cnt].type_addr = 0x8e;
    IDT[cnt].low_offset = (uint16_t)(IRQ >> 0);
    IDT[cnt].middle_offset = (uint16_t)(IRQ >> 16);
    IDT[cnt].high_offset = (uint32_t)(IRQ >> 32);

    cnt++;
}

void idt_expection(uint64_t IRQ, uint64_t over_ride = 0)
{
    static uint64_t cnt = 0;

    if(over_ride)
        cnt = over_ride;

    IDT[cnt].selector = 0x8;
    IDT[cnt].zero32 = 0;
    IDT[cnt].zero8 = 0;
    IDT[cnt].type_addr = 0x8f;
    IDT[cnt].low_offset = (uint16_t)(IRQ >> 0);
    IDT[cnt].middle_offset = (uint16_t)(IRQ >> 16);
    IDT[cnt].high_offset = (uint32_t)(IRQ >> 32);

    cnt++;
}

typedef void (*irqReferences)();

irqReferences irqFuncs[] =  {   PITI, keyboard_handler_main, irq_l, irq_l,
                                irq_l, irq_l, irq_l, irq_h, irq_h, irq_h,
                                irq_h, irq_h, mouse_handler, irq_h
                            };

extern "C" void irq_handler(int irqNum)
{
    irqFuncs[irqNum]();
}

void idt_init(void)
{
    /* cpu exceptions */

    idt_expection((uint64_t)divide_zero);
    idt_expection((uint64_t)debug);
    idt_expection((uint64_t)non_maskable_irq);
    idt_expection((uint64_t)breakpoint);
    idt_expection((uint64_t)overflow);
    idt_expection((uint64_t)bound_range);
    idt_expection((uint64_t)invaild_opcode);
    idt_expection((uint64_t)device_not_available);
    idt_expection((uint64_t)double_fault);
    idt_expection((uint64_t)coprocessor_seg_overrun);
    idt_expection((uint64_t)invaild_tss);
    idt_expection((uint64_t)segment_not_found);
    idt_expection((uint64_t)stack_seg_fault);
    idt_expection((uint64_t)gen_fault);
    idt_expection((uint64_t)page_fault);
    idt_expection((uint64_t)reserved);
    idt_expection((uint64_t)floating_point_fault);
    idt_expection((uint64_t)alignment_check);
    idt_expection((uint64_t)machine_check);
    idt_expection((uint64_t)simd_floating_point);
    idt_expection((uint64_t)vm_expection);
    idt_expection((uint64_t)reserved);
    idt_expection((uint64_t)reserved);
    idt_expection((uint64_t)reserved);
    idt_expection((uint64_t)reserved);
    idt_expection((uint64_t)reserved);
    idt_expection((uint64_t)reserved);
    idt_expection((uint64_t)reserved);
    idt_expection((uint64_t)reserved);
    idt_expection((uint64_t)reserved);
    idt_expection((uint64_t)security_expection);
    idt_expection((uint64_t)reserved);

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

    /* irqs */

    idt_gate((uint64_t)time_handler);
    idt_gate((uint64_t)keyboard_handler);
    idt_gate((uint64_t)irq2);
    idt_gate((uint64_t)irq3);
    idt_gate((uint64_t)irq4);
    idt_gate((uint64_t)irq5);
    idt_gate((uint64_t)irq6);
    idt_gate((uint64_t)irq7);
    idt_gate((uint64_t)irq8);
    idt_gate((uint64_t)irq9);
    idt_gate((uint64_t)irq10);
    idt_gate((uint64_t)irq11);
    idt_gate((uint64_t)mouse);
    idt_gate((uint64_t)irq13);
    idt_gate((uint64_t)irq14);
    idt_gate((uint64_t)irq15);

    idt_r.base = (uint64_t)&IDT;
    idt_r.limit = 256 * sizeof(IDT_entry) - 1;
    asm volatile("lidtq %0" ::"m"(idt_r));
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

extern "C" void gdt_info(uint64_t addr)
{
    k_print("GDT: mapped to %x\n", addr);
}

void irq_l(void)
{
    outb(0x20, 0x20);
}

void irq_h(void)
{
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

extern "C" void panic(const char *message)
{
    initalize(VGA_BLUE, VGA_RED);
    k_print("PANIC : fatal error : %s\n", message);
    reg_flow();
    putchar('\n');
    for(;;);
}

extern void save_regs(void) asm("save_regs");
extern void save_regs16(void) asm("save_regs16");
extern void save_segment(void) asm("save_segment");

seg_register segment;
registers gen_reg;

void reg_flow()
{
    save_regs();
    k_print("\nREGISTER DUMP\n");
    k_print("Dump: RAX %a\n", gen_reg.rax);
    k_print("Dump: RBX %a\n", gen_reg.rbx);
    k_print("Dump: RCX %a\n", gen_reg.rcx);
    k_print("Dump: RDX %a\n", gen_reg.rdx);
    k_print("Dump: RSP %a\n", gen_reg.rsp);
    k_print("Dump: RBP %a\n", gen_reg.rbp);
    k_print("Dump: RDI %a\n", gen_reg.rdi);
    k_print("Dump: RSI %a\n", gen_reg.rsi);
    k_print("Dump: R8  %a\n", gen_reg.r8);
    k_print("Dump: R9  %a\n", gen_reg.r9);
    k_print("Dump: R10 %a\n", gen_reg.r10);
    k_print("Dump: R11 %a\n", gen_reg.r11);
    k_print("Dump: R12 %a\n", gen_reg.r12);
    k_print("Dump: R13 %a\n", gen_reg.r13);
    k_print("Dump: R14 %a\n", gen_reg.r14);
    k_print("Dump: R15 %a\n", gen_reg.r15);
}

void seg_flow()
{
    save_segment();
    k_print("\nSEGMENT DUMP\n");
    k_print("Dump SS %a\n", segment.ss);
    k_print("Dump CS %a\n", segment.cs);
    k_print("Dump DS %ax\n", segment.ds);
    k_print("Dump ES %a\n", segment.es);
    k_print("Dump FS %a\n", segment.fs);
    k_print("Dump GS %a\n", segment.gs);
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

    uint16_t divisor = 1193181 / frequency;

    uint8_t ossal = 0;
    ossal = (ossal & ~0xe) | mode;
    ossal = (ossal & ~0x30) | 0xe;
    ossal = (ossal & ~0xc0) | counter;
    outb(0x43, ossal);

    pit_send_data(divisor & 0xff, 0);
    pit_send_data((divisor >> 8) & 0xff, 0);
}
