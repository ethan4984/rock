global load_idt
global load_gdt

global keyboard_handler
global time_handler
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

extern keyboard_handler_main
extern PITI
extern irq_l
extern irq_h

load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	sti
	ret

keyboard_handler:
	pushad
	call    keyboard_handler_main
	popad
	iretd
time_handler:
	pushad
	call time_handler
	popad
	iretd
irq2:
  pusha
  call irq_l
  popa
  iret

irq3:
  pusha
  call irq_l
  popa
  iret

irq4:
  pusha
  call irq_l
  popa
  iret

irq5:
  pusha
  call irq_l
  popa
  iret

irq6:
  pusha
  call irq_l
  popa
  iret

irq7:
  pusha
  call irq_l
  popa
  iret

irq8:
  pusha
  call irq_h
  popa
  iret

irq9:
  pusha
  call irq_h
  popa
  iret

irq10:
  pusha
  call irq_h
  popa
  iret

irq11:
  pusha
  call irq_h
  popa
  iret

irq12:
  pusha
  call irq_h
  popa
  iret

irq13:
  pusha
  call irq_h
  popa
  iret

irq14:
  pusha
  call irq_h
  popa
  iret

irq15:
  pusha
  call irq_h
  popa
  iret

global load_gdt

gdt_start:

gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

load_gdt:
    lgdt [gdt_descriptor]
    jmp CODE_SEG:.setcs
.setcs:
    mov eax, DATA_SEG
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax
    ret
