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

%macro expection_handler 1
    cli
    pushad
    push %1
    call panic
    popad
%endmacro

divide_by_zero:
    expection_handler div0_m
    ret

debug:
    expection_handler debug_m
    ret

non_maskable_irq:
    expection_handler nmi_m
    ret

breakpoint:
    expection_handler breakpoint_m
    ret

overflow:
    expection_handler overflow_m
    ret

bound_range:
    expection_handler bound_m
    ret

invaild_opcode:
    expection_handler opcode_m
    ret

device_not_available:
    expection_handler device_m
    ret

double_fault:
    expection_handler double_fault_m
    ret

coprocessor_seg_overrun:
    expection_handler process_m
    ret
invaild_tss:
    expection_handler tss_m
    ret

segment_not_found:
    expection_handler seg_not_found_m
    ret

stack_seg_fault:
    expection_handler ssf_m
    ret

gen_fault:
    expection_handler gen_m
    ret

page_fault:
    expection_handler page_m
    ret

floating_point_fault:
    expection_handler floating_m
    ret

alignment_check:
    expection_handler align_m
    ret

machine_check:
    expection_handler machine_m
    ret

simd_floating_point:
    expection_handler simd_m
    ret

vm_expection:
    expection_handler machine_m
    ret

security_expection:
    expection_handler sec_m
    ret

section .data
    div0_m: db 'divide by zero retard', 0
    debug_m: db 'debug fault', 0
    nmi_m db 'Non maskable interrupt', 0
    breakpoint_m db 'breakpoint trap', 0
    overflow_m db 'overflow retard', 0
    bound_m db 'bound range exeeced!', 0
    opcode_m db 'invaild opcode bro!', 0
    device_m db 'device not available!', 0
    double_fault_m db 'bruh you got a double fault!', 0
    process_m db 'coprocessor segment overrun!', 0
    tss_m db 'invaild tss!', 0
    seg_not_found_m db 'segment not found bruh!', 0
    ssf_m db 'stack segment fault!', 0
    gen_m db 'general protection fault!', 0
    page_m db 'good for you, you have a page fault!', 0
    floating_m db 'floating point expection fault!', 0
    align_m db 'alignment check fault!', 0
    machine_m db 'stop creating a vm!', 0
    simd_m db 'simp!', 0
    sec_m db 'your security is trash!', 0
