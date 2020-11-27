section .stivalehdr

dq stack
dw 1
dw 0
dw 0
dw 0
dq 0

section .bss

align 0x10

resb 32768

stack:
