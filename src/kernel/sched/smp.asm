org 0x1000
bits 16

coreBootstrap:
    cld
    jmp 0:initCSTramp

initCSTramp:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x7c00

    in al, 0x92
    or al, 2
    out 0x92, al ; enables a20 line

    lgdt[GDT]

    mov eax, dword [0x500 + 8]
    mov cr3, eax

    mov eax, cr4
    or eax, (1 << 5) | (1 << 7) ; set PAE and PGE
    mov cr4, eax

    mov ecx, 0xc0000080
    rdmsr
    or eax, (1 << 0) | (1 << 8) ; set LME and SCE
    wrmsr

    mov eax, 0x80000011
    mov cr0, eax

    jmp GDT.CODE64 - GDT.start:longModeCode

    bits 64

longModeCode:
    mov ax, GDT.DATA64 - GDT.start
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax 

    mov rsp, qword [0x500] ; stack
    mov rbx, qword [0x500 + 16] ; entry point
    mov rcx, qword [0x500 + 24] ; idt
    mov rax, qword [0x500 + 32] ; core number

    mov fs, ax

    lidt[rcx]

    jmp rbx

GDT:
    dw .end - .start - 1 
    dd .start

.start:

.NULL:
    dq 0
.CODE64:
    dw 0 ; limit
    dw 0 ; base low
    db 0 ; base mid 
    db 0b10011010 ; access
    db 0b00100000 ; granularity
    db 0 ; base high
.DATA64:
    dw 0 ; limit
    dw 0 ; base low
    db 0 ; base mid
    db 0b10010010 ; access
    db 0 ; granularity
    db 0 ; base high
.end:
