global lgdt

lgdt:
    lgdt [rdi]

    push rbp
    mov rbp, rsp 

    push 0x10 ; data 64
    push rbp
    pushfq
    push 0x8 ; code 64
    push entry
    
    iretq
entry:
    pop rbp
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ret

global ltr

ltr:
    ltr di
    ret
