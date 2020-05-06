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
global irq12
global irq13
global irq14
global irq15

global disable_pic
global test_div
extern gen_reg
extern gen_reg16
extern segment
extern divide

global start_process
global switch_process
global generic_irq

extern PITI

%macro pushall 0
push rax
push rbx
push rcx
push rdx
push rbp
push rdi
push rsi
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
pop rsi
pop rdi
pop rbp
pop rdx
pop rcx
pop rbx
pop rax
%endmacro

%macro irq_send 1
    mov rdi, %1
    call generic_irq
%endmacro

section .text

extern irq_handler

generic_irq:
    pushall
    cld
    call irq_handler
    popall
    ret

time_handler:
    cli
    pushall

    mov rdi, rbp
    mov rsi, rsp

    call PITI
    hlt

start_process:
    mov rbx, rsi ; ptr to main function
    mov rax, rdi ; new stack

    mov rsp, rax ; new stack
    mov rbp, rsp

    sti
    jmp rbx

switch_process:
    mov rbx, rsi ; old rbp
    mov rax, rdi ; old stack pointer

    mov rbp, rbx ; restores stack frame
    mov rsp, rax

    popall

    sti
    iretq

keyboard_handler:
    irq_send 1
    iretq

irq2:
    irq_send 2
    iretq

irq3:
    irq_send 3
    iretq

irq4:
    irq_send 4
    iretq

irq5:
    irq_send 5
    iretq

irq6:
    irq_send 6
    iretq

irq7:
    irq_send  7
    iretq

irq8:
    irq_send 8
    iretq

irq9:
    irq_send 9
    iretq

irq10:
    irq_send 10
    iretq

irq11:
    irq_send 11
    iretq

irq12:
    irq_send 12
    iretq

irq13:
    irq_send 13
    iretq

irq14:
    irq_send 14
    iretq

irq15:
    irq_send 15
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

test_div:
    mov rdx, 0
    mov rax, 250
    mov rcx, 0
    div rcx
    ret

disable_pic:
    mov al, 0xff
    out 0xa1, al
    out 0x21, al
    ret
