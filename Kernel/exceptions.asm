global divide_by_zero
global debug
global non_maskable_irq
global breakpoint
global overflow
global bound_range
global invaild_opcode
global device_not_available
global double_fault
global coprocessor_seg_overrun
global invaild_tss
global segment_not_found
global stack_seg_fault
global gen_fault
global page_fault
global floating_point_fault
global alignment_check
global machine_check
global simd_floating_point
global vm_expection
global security_expection

extern panic

%macro pushall 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popall 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

%macro expection_handler 1
    cli
    pushall
    push %1
    call panic
    popall
%endmacro

section .text

divide_by_zero:
    expection_handler div0_m
    iretq

debug:
    expection_handler debug_m
    iretq

non_maskable_irq:
    expection_handler nmi_m
    iretq

breakpoint:
    expection_handler breakpoint_m
    iretq

overflow:
    expection_handler overflow_m
    iretq

bound_range:
    expection_handler bound_m
    iretq

invaild_opcode:
    expection_handler opcode_m
    iretq

device_not_available:
    expection_handler device_m
    iretq

double_fault:
    expection_handler double_fault_m
    iretq

coprocessor_seg_overrun:
    expection_handler process_m
    iretq

invaild_tss:
    expection_handler tss_m
    iretq

segment_not_found:
    expection_handler seg_not_found_m
    iretq

stack_seg_fault:
    expection_handler ssf_m
    iretq

gen_fault:
    expection_handler gen_m
    iretq

page_fault:
    expection_handler page_m
    iretq

floating_point_fault:
    expection_handler floating_m
    iretq

alignment_check:
    expection_handler align_m
    iretq

machine_check:
    expection_handler machine_m
    iretq

simd_floating_point:
    expection_handler simd_m
    iretq

vm_expection:
    expection_handler machine_m
    iretq

security_expection:
    expection_handler sec_m
    iretq

section .data
    div0_m: dq 'divide by zero retard', 0
    debug_m: db 'debug fault', 0
    nmi_m db 'Non maskable interrupt', 0
    breakpoint_m db 'breakpoint trap', 0
    overflow_m db 'overflow retard', 0
    bound_m db 'bound range exeeced!', 0
    opcode_m db 'invaild opcode bro!', 0
    device_m db 'device not available!', 0
    double_fault_m db 'bruh you got a double fault!', 0
    process_m db 'coprocessor seg overrun!', 0
    tss_m db 'invaild tss!', 0
    seg_not_found_m db 'segment not found bruh!', 0
    ssf_m db 'stack segment fault!', 0
    gen_m dq 'general protection fault!', 0
    page_m db 'you have a page fault bitch!', 0
    floating_m db 'floating pnt expection fault!', 0
    align_m db 'alignment check fault!', 0
    machine_m db 'stop creating a vm!', 0
    simd_m db 'simp!', 0
    sec_m db 'your security is trash!', 0
