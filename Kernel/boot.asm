MBALIGN  equ 1 << 0
MEMINFO  equ 1 << 1
FLAGS    equ MBALIGN | MEMINFO
MAGIC    equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

%define kernel_high 0xffffffff80000000

section .text

global _start

_start:
    mov edi, ebx
    mov esp, stack.top - kernel_high

    bits 32 ; protected mode

    mov eax, 0x80000000 ; checks for long mode
    cpuid
    cmp eax, 0x80000001
    jb .NoX86_64

    mov eax, 0x80000001 ; tests for long mode
    cpuid
    test edx, 1 << 29
    jz .NoX86_64

    mov eax, p4_table - kernel_high ; page init
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0 ; enable paging
    or eax, 1 << 31
    mov cr0, eax

    lgdt [gdt.ptr_32 - kernel_high]

    jmp (0x8):(load_64 - kernel_high)

    .NoX86_64:
        hlt

bits 64

load_64:
    mov ax, 0x10
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov rax, higher_half
    jmp rax

higher_half:
    mov rsp, stack.top
    lgdt [gdt.gen_ptr]

    extern kernel_main
    call kernel_main

    cli
    .hang:
        hlt
    jmp .hang
    .end

section .data

gdt:
    .null: equ $ - gdt
        dw 0xFFFF               ; Limit (low)
        dw 0                    ; Base (low)
        db 0                    ; Base (middle)
        db 0                    ; Access
        db 0                    ; Granularity
        db 0                    ; Base (high)

    .ring0_code: equ $ - gdt
        dw 0                    ; Limit (low)
        dw 0                    ; Base (low)
        db 0                    ; Base (middle)
        db 10011010b            ; Access (exec/read)
        db 10101111b            ; Granularity
        db 0                    ; Base (high)

    .ring0_data: equ $ - gdt
        dw 0                    ; Limit (low)
        dw 0                    ; Base (low)
        db 0                    ; Base (middle)
        db 10010010b            ; Access (read/write)
        db 0                    ; Granularity
        db 0                    ; Base (high)

    .ring3_code: equ $ - gdt
        dw 0                    ; Limit (low)
        dw 0                    ; Base (low)
        db 0                    ; Base (middle)
        db 11111010b            ; Access (exec/read)
        db 10101111b            ; Granularity
        db 0                    ; Base (high)

    .ring3_data: equ $ - gdt
        dw 0                    ; Limit (low)
        dw 0                    ; Base (low)
        db 0                    ; Base (middle)
        db 10010010b            ; Access (read/write)
        db 0                    ; Granularity
        db 0                    ; Base (high)

    .gen_ptr:                   ; GDT Pointer
        dw $ - gdt - 1          ; Limit
        dq gdt                  ; Base

    .ptr_32:                    ; 32 bit GDT pointer
        dw $ - gdt - 1          ; Limit
        dq gdt - kernel_high    ; Base

align 4096

p4_table:
    dq p3_table_low + 0x3 - kernel_high
    times 510 dq 0
    dq p3_table_high + 0x3 - kernel_high

p3_table_low:
    dq p2_table + 0x3 - kernel_high
    times 511 dq 0

p3_table_high:
    times 510 dq 0
    dq p2_table + 0x3 - kernel_high
    dq 0

p2_table:
    %assign i 0
    %rep 50000

    dq (i | 0x83)
    %assign i i + 0x200000 ; 2mb blocks

    %endrep
    %rep 1000

    dq 0

    %endrep

section .multiboot

align 4
dd MAGIC
dd FLAGS
dd CHECKSUM

section .bss

align 16
stack:
    resb 8192
    .top:
