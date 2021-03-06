%include 'lib/asm_macros.inc'

extern syscall_open
extern syscall_close
extern syscall_write
extern syscall_read
extern syscall_lseek
extern syscall_dup
extern syscall_dup2
extern syscall_fstat
extern syscall_stat
extern syscall_fcntl
extern syscall_set_fs_base
extern syscall_set_gs_base
extern syscall_get_fs_base
extern syscall_get_gs_base
extern syscall_getpid
extern syscall_gettid
extern syscall_getppid
extern syscall_execve
extern syscall_exit
extern syscall_yeild
extern syscall_log
extern syscall_mmap
extern syscall_munmap
extern syscall_chdir

syscall_list:

dq syscall_open
dq syscall_close
dq syscall_write
dq syscall_read
dq syscall_lseek
dq syscall_dup
dq syscall_dup2
dq syscall_fstat
dq syscall_stat
dq syscall_fcntl
dq syscall_set_fs_base
dq syscall_set_gs_base
dq syscall_get_fs_base
dq syscall_get_gs_base
dq syscall_getpid
dq syscall_gettid
dq syscall_getppid
dq syscall_execve
dq syscall_exit
dq syscall_yeild
dq syscall_log
dq syscall_mmap
dq syscall_munmap
dq syscall_chdir

.end:

syscall_cnt equ ((syscall_list.end - syscall_list) / 8)

global syscall_main

syscall_main:
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
    jae .leave

    mov rdi, rsp
    call [syscall_list + rax * 8]

.leave:
    popall

    add rsp, 56

    pop r11 ; rflags
    pop rcx ; rip

    cli
   
    mov rdx, qword [gs:24] ; errno
    mov rsp, qword [gs:16] ; user stack

    swapgs

    o64 sysret ; ensure 64 bit operanh size so we returned to 64 bit mode
