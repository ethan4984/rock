%define kernel_high 0xffffffff80000000

section .stivalehdr

stivale_header:
    dq stack.top
    dw 1
    dw 0
    dw 0
    dw 0

section .text

extern kernel_main
global _start
_start:
    lgdt [gdt.gen_ptr]

    ; Reload CS
    push 0x10
    push rsp
    pushf
    push 0x8
    push loaded_cs
    iretq

loaded_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov ax, 0x20
    mov gs, ax
    mov fs, ax

    call kernel_main

section .data

align 4096

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

section .bss

stack:
    resb 4096
  .top:
