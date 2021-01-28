%include 'klib/asm_mac.inc'

syscall_list:

extern getpid
dq getpid ; rax = 8
extern getppid
dq getppid ; rax = 9
extern setuid
dq setuid ; rax = 10
extern setppid
dq setppid ; rax = 11
extern getpgrp 
dq getpgrp ; rax = 12

.end:

syscall_cnt equ ((syscall_list.end - syscall_list) / 8)

global syscall_main_stub

syscall_main_stub:
    mov qword [gs:16], rsp ; save user stack
    mov rsp, qword [gs:8] ; init kernel stack

    sti

    push r11 ; rflags
    push rcx ; rip

    pushall

    cmp rax, syscall_cnt
    jae .error

    call [syscall_list + rax * 8]

.leave:
    syscall_popall ; does not pop rax cuz its the return value

    pop rcx ; rip
    pop r11 ; rflags

    mov rsp, qword [gs:16] ; user stack

    o64 sysret ; ensure 64 bit operanh size so we returned to 64 bit mode

.error:
    mov rax, -1
    jmp .leave
