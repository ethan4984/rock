%include 'klib/asm_mac.inc'

syscall_list:

extern read
dq read ; rax = 0
extern write
dq write ; rax = 1
extern lseek
dq lseek ; rax = 2
extern close
dq close ; rax = 3
extern open
dq open ; rax = 4
extern dup
dq dup ; rax = 5
extern dup2
dq dup2 ; rax = 6
extern mmap
dq mmap ; rax = 7
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
