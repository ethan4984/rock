%include 'klib/asm_mac.inc'

extern syscall_open
extern syscall_close
extern syscall_write
extern syscall_read
extern syscall_lseek
extern syscall_dup
extern syscall_dup2
extern syscall_set_fs_base
extern syscall_set_gs_base
extern syscall_get_fs_base
extern syscall_get_gs_base
extern syscall_getpid
extern syscall_gettid
extern syscall_getppid

syscall_list:

dq syscall_open
dq syscall_close
dq syscall_write
dq syscall_read
dq syscall_lseek
dq syscall_dup
dq syscall_dup2
dq syscall_set_fs_base
dq syscall_set_gs_base
dq syscall_get_fs_base
dq syscall_get_gs_base
dq syscall_getpid
dq syscall_gettid
dq syscall_getppid

.end:

syscall_cnt equ ((syscall_list.end - syscall_list) / 8)

global syscall_main_stub

syscall_main_stub:
    swapgs

    mov qword [gs:16], rsp ; save user stack
    mov rsp, qword [gs:8] ; init kernel stack

    sti
    cld

    push rcx ; rip
    push r11 ; rflags

    push 0x1b ; ss
    push qword [gs:16] ; rsp
    push r11 ; rflags
    push 0x23 ; cs
    push rcx ; rip 

    push 0
    push 0

    pushall

    cmp rax, syscall_cnt
    jae .error

    mov rdi, rsp
    call [syscall_list + rax * 8]

.leave:
    popall

    add rsp, 56

    pop r11 ; rflags
    pop rcx ; rip

    cli

    mov rsp, qword [gs:16] ; user stack
    swapgs

    o64 sysret ; ensure 64 bit operanh size so we returned to 64 bit mode

.error:
    mov rax, -1
    jmp .leave
