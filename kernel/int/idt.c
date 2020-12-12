#include <output.h>
#include <sched/scheduler.h>
#include <int/idt.h>
#include <int/apic.h>
#include <mm/vmm.h>

static idt_entry_t idt[256];

static const char *exception_messages[] = { "Divide by zero",
                                            "Debug",
                                            "NMI",
                                            "Breakpoint",
                                            "Overflow",
                                            "Bound Range Exceeded",
                                            "Invaild Opcode",
                                            "Device Not Available", 
                                            "Double fault", 
                                            "Co-processor Segment Overrun",
                                            "Invaild TSS",
                                            "Segment not present",
                                            "Stack-Segment Fault",
                                            "GPF",
                                            "Page Fault",
                                            "Reserved",
                                            "x87 Floating Point Exception",
                                            "allignement check",
                                            "Machine check",
                                            "SIMD floating-point exception",
                                            "Virtualization Excpetion",
                                            "Deadlock",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Security Exception",
                                            "Reserved",
                                            "Triple Fault",
                                            "FPU error"
                                         };

typedef void (*isr_handler_t)(regs_t *regs); 

static isr_handler_t isr_handlers[] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 32
    scheduler_main, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 64
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 96 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 128 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 160
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 192
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 224
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 256
};

extern void isr_handler_main(regs_t *stack) {
    if(stack->isr_number < 32) {
        static char lock = 0;
        spin_lock(&lock);

        uint64_t cr2;
        asm volatile ("mov %%cr2, %0" : "=a"(cr2));
        
        kvprintf("Kowalski analysis: \"%s\", Error: %x\n", exception_messages[stack->isr_number], stack->error_code);
        kvprintf("RAX: %a | RBX: %a | RCX: %a | RDX: %a\n", stack->rax, stack->rbx, stack->rcx, stack->rdx);
        kvprintf("RSI: %a | RDI: %a | RBP: %a | RSP: %a\n", stack->rsi, stack->rdi, stack->rbp, stack->rsp);
        kvprintf("r8:  %a | r9:  %a | r10: %a | r11: %a\n", stack->r8, stack->r9, stack->r10, stack->r11);
        kvprintf("r12: %a | r13: %a | r14: %a | r15: %a\n", stack->r12, stack->r13, stack->r14, stack->r15); 
        kvprintf("cs:  %a | ss:  %a | cr2: %a | rip: %a\n", stack->cs, stack->ss, cr2, stack->rip);

        kprintf("[KDEBUG]", "Kowalski analysis: \"%s\", Error: %x", exception_messages[stack->isr_number], stack->error_code);
        kprintf("[KDEBUG]", "RAX: %a | RBX: %a | RCX: %a | RDX: %a", stack->rax, stack->rbx, stack->rcx, stack->rdx);
        kprintf("[KDEBUG]", "RSI: %a | RDI: %a | RBP: %a | RSP: %a", stack->rsi, stack->rdi, stack->rbp, stack->rsp);
        kprintf("[KDEBUG]", "r8:  %a | r9:  %a | r10: %a | r11: %a", stack->r8, stack->r9, stack->r10, stack->r11);
        kprintf("[KDEBUG]", "r12: %a | r13: %a | r14: %a | r15: %a", stack->r12, stack->r13, stack->r14, stack->r15); 
        kprintf("[KDEBUG]", "cs:  %a | ss:  %a | cr2: %a | rip: %a", stack->cs, stack->ss, cr2, stack->rip);

        stacktrace((uint64_t*)stack->rbp);

        spin_release(&lock);

        asm ("hlt");
    }

    if(isr_handlers[stack->isr_number] != NULL) {
        isr_handlers[stack->isr_number](stack); 
    }
    
    lapic_write(LAPIC_EOI, 0);    
}

void set_idt_entry(uint16_t cs, uint8_t ist, uint8_t attributes, uint64_t offset, uint8_t index) {
    idt[index] = (idt_entry_t) {    (uint16_t)offset, // offset low
                                    cs,
                                    ist,
                                    attributes,
                                    (uint16_t)(offset >> 16), // offset mid
                                    (uint32_t)(offset >> 32), // offset high
                                    0
                               };
}

void idt_init() {
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr0, 0);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr1, 1);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr2, 2);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr3, 3);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr4, 4);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr5, 5);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr6, 6);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr7, 7);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)error_isr8, 8);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr9, 9);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)error_isr10, 10);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)error_isr11, 11);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)error_isr12, 12);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)error_isr13, 13);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)error_isr14, 14);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr15, 15);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr16, 16);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr17, 17);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr18, 18);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr19, 19);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr20, 20);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr21, 21);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr22, 22);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr23, 23);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr24, 24);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr25, 25);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr26, 26);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr27, 27);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr28, 28);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr29, 29);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr30, 30);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr31, 31);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr32, 32);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr33, 33);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr34, 34);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr35, 35);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr36, 36);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr37, 37);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr38, 38);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr39, 39);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr40, 40);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr41, 41);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr42, 42);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr43, 43);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr44, 44);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr45, 45);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr46, 46);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr47, 47);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr48, 48);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr49, 49);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr50, 50);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr51, 51);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr52, 52);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr53, 53);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr54, 54);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr55, 55);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr56, 56);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr57, 57);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr58, 58);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr59, 59);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr60, 60);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr61, 61);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr62, 62);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr63, 63);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr64, 64);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr65, 65);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr66, 66);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr67, 67);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr68, 68);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr69, 69);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr70, 70);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr71, 71);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr72, 72);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr73, 73);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr74, 74);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr75, 75);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr76, 76);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr77, 77);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr78, 78);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr79, 79);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr80, 80);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr81, 81);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr82, 82);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr83, 83);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr84, 84);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr85, 85);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr86, 86);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr87, 87);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr88, 88);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr89, 89);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr90, 90);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr91, 91);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr92, 92);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr93, 93);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr94, 94);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr95, 95);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr96, 96);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr97, 97);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr98, 98);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr99, 99);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr100, 100);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr101, 101);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr102, 102);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr103, 103);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr104, 104);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr105, 105);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr106, 106);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr107, 107);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr108, 108);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr109, 109);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr110, 110);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr111, 111);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr112, 112);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr113, 113);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr114, 114);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr115, 115);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr116, 116);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr117, 117);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr118, 118);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr119, 119);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr120, 120);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr121, 121);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr122, 122);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr123, 123);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr124, 124);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr125, 125);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr126, 126);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr127, 127);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr128, 128);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr129, 129);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr130, 130);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr131, 131);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr132, 132);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr133, 133);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr134, 134);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr135, 135);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr136, 136);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr137, 137);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr138, 138);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr139, 139);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr140, 140);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr141, 141);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr142, 142);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr143, 143);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr144, 144);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr145, 145);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr146, 146);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr147, 147);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr148, 148);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr149, 149);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr150, 150);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr151, 151);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr152, 152);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr153, 153);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr154, 154);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr155, 155);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr156, 156);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr157, 157);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr158, 158);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr159, 159);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr160, 160);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr161, 161);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr162, 162);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr163, 163);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr164, 164);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr165, 165);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr166, 166);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr167, 167);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr168, 168);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr169, 169);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr170, 170);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr171, 171);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr172, 172);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr173, 173);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr174, 174);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr175, 175);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr176, 176);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr177, 177);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr178, 178);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr179, 179);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr180, 180);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr181, 181);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr182, 182);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr183, 183);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr184, 184);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr185, 185);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr186, 186);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr187, 187);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr188, 188);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr189, 189);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr190, 190);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr191, 191);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr192, 192);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr193, 193);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr194, 194);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr195, 195);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr196, 196);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr197, 197);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr198, 198);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr199, 199);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr200, 200);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr201, 201);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr202, 202);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr203, 203);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr204, 204);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr205, 205);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr206, 206);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr207, 207);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr208, 208);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr209, 209);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr210, 210);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr211, 211);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr212, 212);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr213, 213);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr214, 214);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr215, 215);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr216, 216);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr217, 217);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr218, 218);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr219, 219);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr220, 220);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr221, 221);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr222, 222);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr223, 223);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr224, 224);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr225, 225);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr226, 226);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr227, 227);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr228, 228);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr229, 229);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr230, 230);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr231, 231);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr232, 232);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr233, 233);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr234, 234);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr235, 235);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr236, 236);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr237, 237);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr238, 238);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr239, 239);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr240, 240);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr241, 241);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr242, 242);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr243, 243);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr244, 244);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr245, 245);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr246, 246);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr247, 247);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr248, 248);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr249, 249);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr250, 250);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr251, 251);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr252, 252);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr253, 253);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr254, 254);
    set_idt_entry(0x8, 0, 0x8e, (uint64_t)isr255, 255);

    idtr_t idtr = { 256 * sizeof(idt_entry_t) - 1, (uint64_t)idt - KERNEL_HIGH_VMA };
    asm volatile ("lidtq %0" : "=m"(idtr));
}
