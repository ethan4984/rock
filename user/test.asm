global _start

_start:
    mov rsp, 0x1500
    syscall
    mov rax, 0x6969
    jmp $
