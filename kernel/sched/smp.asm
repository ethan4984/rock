org 0x1000
bits 16

cli
cld

jmp 0:init_CS

init_CS:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov gs, ax
    mov sp, 0x7c00

    in al, 0x92
    or al, 2
    out 0x92, al

    lgdt[GDT]

    mov eax, dword [0x500 + 8]
    mov cr3, eax

    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax

    mov ecx, 0xc0000080
    rdmsr
    or eax, (1 << 8) ; set LME 
    wrmsr

    mov eax, 0x80000011
    mov cr0, eax

    jmp GDT.CODE64 - GDT.start:long_mode_code

    bits 64

long_mode_code: 
    mov ax, GDT.DATA64 - GDT.start
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rsp, qword [0x500] ; stack
    mov rbx, qword [0x500 + 16] ; entry point
    mov rcx, qword [0x500 + 24] ; idtr

    lidt [rcx]

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
