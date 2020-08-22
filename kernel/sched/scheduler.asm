%include 'lib/asmMacros.inc'

global startTask

startTask:
    mov ds, di
    mov es, di
    mov gs, di

    sti

    push rdi ; ss
    push rsi ; stack
    pushfq
    push rdx ; cs
    push rcx ; rip
    iretq

global switchTask

switchTask:
    mov ds, si
    mov es, si
    mov gs, si

    sti

    mov rsp, rdi

    popall
    add rsp, 24
    iretq
