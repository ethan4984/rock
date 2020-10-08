global userTest

global initsyscall

userTest:
    mov r15, 1
    xor r14, r14
    mov r13, 'z'
    int 0x69
    jmp userTest

initsyscall:
    mov ecx, 0xc0000080
    rdmsr
    or eax, (1 << 8) | (1 << 0)
    wrmsr
    ret
