%include 'klib/asm_mac.inc'

global start_task

start_task:
    sti

    push rdi ; ss
    push rsi ; rsp
    push rdx ; cs
    pushfq
    push rcx ; rip

    iretq

global switch_task

switch_task:
    sti

    mov rsp, rdi
    popall
    add rsp, 24

    iretq
