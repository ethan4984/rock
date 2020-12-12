%include 'klib/asm_mac.inc'

global start_task

start_task:
    sti

    push rdi ; ss
    push rsi ; rsp
    pushfq
    push rdx ; cs
    push rcx ; rip

    iretq

global switch_task

switch_task:
    sti

    mov rsp, rdi
    popall
    add rsp, 16

    iretq
