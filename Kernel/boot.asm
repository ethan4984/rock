%define kernel_high 0xffffffff80000000

section .stivalehdr ; stivale

stivalehdr: 
    dq stack.top
    dw 1 
    dw 1024 ; framebuffer height
    dw 768 ; frame buffer width
    dw 32 ; bits per pixel / bpp
    dq _start

section .text

extern kernelMain 

global _start

_start:

    lgdt [GDT] ; load gdt

    ; reload cs
    push GDT.data64 - GDT.start
    push rsp
    pushf
    push GDT.code64 - GDT.start
    push loaded_cs
    iretq

loaded_cs:

    mov ax, GDT.data64 - GDT.start ; GDT.data64
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov ax, GDT.data64user - GDT.start ; GDT.data64user
    mov gs, ax
    mov fs, ax

    ; conrtray to grub whitch passes the boot protocall to ebx, qloader2 passes it directly to rdi so we have no need to 
    ; mov edi, ebx like we would do in grub

    call kernelMain

section .data

align 0x1000

GDT:
    dw .end - .start - 1; Size of GDT ; (Limit)
    dq .start ; base
 
.start:

.null: ; 0x0
    dw 0xffff ; limit low
    dw 0 ; base low
    db 0 ; base middle
    db 0 ; access
    db 0 ; granularity
    db 0 ; base high

.code64: ; 0x8
    dw 0 ; limit
    dw 0 ; base low
    db 0 ; base mid
    db 10011010b ; access
    db 10101111b ; granularity
    db 0 ; base high

.data64: ; 0x10
    dw 0 ; limit
    dw 0 ; base low
    db 0 ; base mid
    db 10010010b ; access
    db 0 ; granularity
    db 0 ; base high

.code64user: ; 0x18
    dw 0 ; limit
    dw 0 ; base low
    db 0 ; base mid
    db 11111010b ; acess
    db 10101111b ; granularity
    db 0 ; base high

.data64user: ; 0x20
    dw 0 ; limit
    dw 0 ; Base
    db 0 ; Base
    db 10010010b ; access
    db 0 ; granularity
    db 0 ; base

.end:

section .bss

stack:
    resb 4096 ; kernel stack of 4 kb
  .top:
