%include 'klib/asm_mac.inc'

syscall_list:

.end:

syscall_cnt equ ((syscall_list.end - syscall_list) / 8)

global syscall_main_stub

syscall_main_stub:
    swapgs

    mov qword [gs:16], rsp ; save user stack
    mov rsp, qword [gs:8] ; init kernel stack

    sti

    push rcx ; rip
    push r11 ; rflags

    pushall

    cmp rax, syscall_cnt
    jae .error

    call [syscall_list + rax * 8]

.leave:
    popall ; does not pop rax cuz its the return value

    pop r11 ; rflags
    pop rcx ; rip

    cli

    mov rsp, qword [gs:16] ; user stack
    swapgs

    o64 sysret ; ensure 64 bit operanh size so we returned to 64 bit mode

.error:
    mov rax, -1
    jmp .leave
