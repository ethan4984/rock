global load_idt
global load_gdt
global draw

global keyboard_handler
global time_handler
global save_regs
global save_segment
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global mouse_handler
global irq13
global irq14
global irq15
global tss_flush

global disable_pic

global test_div

extern keyboard_handler_main
extern PITI
extern mouse_handler_main
extern irq_l
extern irq_h
extern gdt_info
extern gen_reg
extern gen_reg16
extern segment
extern divide

disable_pic:
    mov al, 0xff
    out 0xa1, al
    out 0x21, al
    ret

%macro pushall 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popall 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

test_div:
    mov rdx, 0
    mov rax, 250
    mov rcx, 0
    div rcx
    ret

time_handler:
    pushall
    call PITI
    popall
    iretq

keyboard_handler:
    pushall
    call    keyboard_handler_main
    popall
    iretq

irq2:
    pushall
    call irq_l
    popall
    iretq

irq3:
    pushall
    call irq_l
    popall
    iretq

irq4:
    pushall
    call irq_l
    popall
    iretq

irq5:
    pushall
    call irq_l
    popall
    iretq

irq6:
    pushall
    call irq_l
    popall
    iretq

irq7:
    pushall
    call irq_l
    popall
    iretq

irq8:
    pushall
    call irq_h
    popall
    iretq

irq9:
    pushall
    call irq_h
    popall
    iretq

irq10:
    pushall
    call irq_h
    popall
    iretq

irq11:
    pushall
    call irq_h
    popall
    iretq

mouse_handler:
    pushall
    call irq_h
    popall
    iretq

irq13:
    pushall
    call irq_h
    popall
    iretq

irq14:
    pushall
    call irq_h
    popall
    iretq

irq15:
    pushall
    call irq_h
    popall
    iretq

extern gen_reg
extern segment

save_regs:
    mov [gen_reg], rax
    mov [gen_reg + 8], rbx
    mov [gen_reg + 16], rcx
    mov [gen_reg + 24], rdx

    mov [gen_reg + 32], rdi
    mov [gen_reg + 40], rsi
    mov [gen_reg + 48], rbp
    mov [gen_reg + 56], rsp

    mov [gen_reg + 64], r8
    mov [gen_reg + 72], r9
    mov [gen_reg + 80], r10
    mov [gen_reg + 88], r11
    mov [gen_reg + 96], r12
    mov [gen_reg + 104], r13
    mov [gen_reg + 112], r14
    mov [gen_reg + 120], r15
    ret

save_segment:
    mov [segment], ss
    mov [segment + 2], cs
    mov [segment + 4], ds
    mov [segment + 6], es
    mov [segment + 8], fs
    mov [segment + 10], gs
    ret
