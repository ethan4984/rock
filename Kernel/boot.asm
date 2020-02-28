global _start

global call_constructor
global ctor_until_end
global call_destructor
global dtors_until_end
extern hang

extern kernel_main

extern start_ctors
extern end_ctors
extern start_dtors
extern end_dtors

MODULEALIGN equ  1<<0
MEMINFO     equ  1<<1
FLAGS       equ  MODULEALIGN | MEMINFO
MAGIC       equ    0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .text

align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

STACKSIZE equ 0x4000

_start:
    mov  esp, stack + STACKSIZE
    mov  [magic], eax
    mov  [mbd], ebx

    mov  ebx, start_ctors
    jmp  ctors_until_end

call_constructor:
    call [ebx]
    add  ebx,4

ctors_until_end:
    cmp  ebx, end_ctors
    jb   call_constructor

    call kernel_main

    mov  ebx, end_dtors
    jmp  dtors_until_end

call_destructor:
    sub  ebx, 4
    call [ebx]

dtors_until_end:
    cmp  ebx, start_dtors
    ja   call_destructor
    cli

hang:
    hlt
    jmp hang

section .bss

align 4
magic: resd 1
mbd:   resd 1
stack: resb STACKSIZE
