bits 32

global load_idt
global load_gdt

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
global irq12
global irq13
global irq14
global irq15

extern keyboard_handler_main
extern PITI
extern irq_l
extern irq_h
extern gdt_info
extern gen_reg
extern gen_reg16
extern segment

load_idt:
    mov edx, [esp + 4]
    lidt [edx]
    sti
    ret

time_handler:
    pushad
    call PITI
    popad
    iretd

keyboard_handler:
    pushad
    call    keyboard_handler_main
    popad
    iretd

irq2:
    pusha
    call irq_l
    popa
    iret

irq3:
    pusha
    call irq_l
    popa
    iret

irq4:
    pusha
    call irq_l
    popa
    iret

irq5:
    pusha
    call irq_l
    popa
    iret

irq6:
    pusha
    call irq_l
    popa
    iret

irq7:
    pusha
    call irq_l
    popa
    iret

irq8:
    pusha
    call irq_h
    popa
    iret

irq9:
    pusha
    call irq_h
    popa
    iret

irq10:
    pusha
    call irq_h
    popa
    iret

irq11:
    pusha
    call irq_h
    popa
    iret

irq12:
    pusha
    call irq_h
    popa
    iret

irq13:
    pusha
    call irq_h
    popa
    iret

irq14:
    pusha
    call irq_h
    popa
    iret

irq15:
    pusha
    call irq_h
    popa
    iret

global load_gdt

gdt_start:

gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

load_gdt:
    lgdt [gdt_descriptor]
    jmp CODE_SEG:.setcs
    .setcs:
    mov eax, DATA_SEG
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax
    ret

save_regs:
    pushad
    mov [gen_reg + 4], eax
    mov [gen_reg + 8], ebx
    mov [gen_reg + 12], ecx
    mov [gen_reg + 16], edx

    mov [gen_reg + 20], esi
    mov [gen_reg + 24], edi
    mov [gen_reg + 28], esp
    mov [gen_reg + 32], ebp
    popad
    ret

global save_regs16
extern gen_regs16

save_regs16:
    pusha
    mov[gen_reg16 + 2], ax
    mov[gen_reg16 + 4], bx
    mov[gen_reg16 + 6], cx
    mov[gen_reg16 + 8], dx

    mov[gen_reg16 + 10], si
    mov[gen_reg16 + 12], di
    mov[gen_reg16 + 14], sp
    mov[gen_reg16 + 16], bp
    popa
    ret

save_segment:
    mov[segment + 2], ss
    mov[segment + 4], cs
    mov[segment + 6], ds
    mov[segment + 8], es
    mov[segment + 10], fs
    mov[segment + 12], gs
    ret
