global load_idt
global load_gdt
global draw

global keyboard_handler
global time_handler
global save_regs
global save_regs16
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

;save_regs:
;    pushad
;    mov [gen_reg + 4], [esp - 4]
;    mov [gen_reg + 8], [esp - 8]
;    mov [gen_reg + 12], [esp - 12]
;    mov [gen_reg + 16], [esp - 16]
;
;    mov [gen_reg + 20], [esp - 20]
;    mov [gen_reg + 24], [esp - 24]
;    mov [gen_reg + 28], [esp - 28]
;    mov [gen_reg + 32], [esp - 32]
;    popad
;    ret
;
;global save_regs16
;extern gen_regs16
;
;save_regs16:
;    pusha
;    mov[gen_reg16 + 2], ax
;    mov[gen_reg16 + 4], bx
;    mov[gen_reg16 + 6], cx
;    mov[gen_reg16 + 8], dx
;
;    mov[gen_reg16 + 10], si
;    mov[gen_reg16 + 12], di
;    mov[gen_reg16 + 14], sp
;    mov[gen_reg16 + 16], bp
;    popa
;    ret
;
;save_segment:
;    mov[segment + 2], ss
;    mov[segment + 4], cs
;    mov[segment + 6], ds
;    mov[segment + 8], es
;    mov[segment + 10], fs
;    mov[segment + 12], gs
;    ret
