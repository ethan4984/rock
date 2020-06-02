global irq0
global irq1
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

global startTask
global switchTask
global testDiv

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr30

%macro pushall 0
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rdi
    push rsi
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
    pop rsi
    pop rdi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

extern irqHandlers
extern schedulerMain
extern mouseHandler
extern isrHandlerMain

handlersHandler: ; handlers event handlers
    pushall
    call irqHandlers
    popall
    ret

irq0:
    cli
    pushall

    mov rdi, rbp
    mov rsi, rsp

    call schedulerMain
    popall
    sti
    iretq ; we only get here if there is zero threads


startTask:
    mov rbx, rsi ; ptr to main function
    mov rax, rdi ; new stack

    mov rsp, rax ; new stack
    mov rbp, rsp

    sti
    jmp rbx ; jmp to main of the thread

switchTask:
    mov rbx, rsi ; old rbp
    mov rax, rdi ; old stack pointer

    mov rbp, rbx ; restores sp
    mov rsp, rax ; stores bp

    popall

    sti
    iretq

irq1:
    mov rdi, 1
    call handlersHandler
    iretq

irq2:
    mov rdi, 2
    call handlersHandler
    iretq

irq3:
    mov rdi, 3
    call handlersHandler
    iretq

irq4:
    mov rdi, 4 
    call handlersHandler
    iretq

irq5:
    mov rdi, 5 
    call handlersHandler
    iretq

irq6:
    mov rdi, 6
    call handlersHandler
    iretq

irq7:
    mov rdi, 7 
    call handlersHandler
    iretq

irq8:
    mov rdi, 8 
    call handlersHandler
    iretq

irq9:
    mov rdi, 9
    call handlersHandler
    iretq

irq10:
    mov rdi, 10
    call handlersHandler
    iretq

irq11:
    mov rdi, 11
    call handlersHandler
    iretq

irq12:
    mov rdi, 12
    pushall
    call mouseHandler 
    popall
    iretq

irq13:
    mov rdi, 13
    call handlersHandler
    iretq

irq14:
    mov rdi, 14
    call handlersHandler
    iretq

irq15:
    mov rdi, 15
    call handlersHandler
    iretq

isrHandler: 
    pushall
    cli
    mov rdi, rsp 
    call isrHandlerMain

isr0:
    push 0
    jmp isrHandler

isr1:
    push 1
    jmp isrHandler

isr2:
    push 2
    jmp isrHandler

isr3:
    push 3 
    jmp isrHandler

isr4:
    push 4 
    jmp isrHandler

isr5:
    push 5 
    jmp isrHandler
    
isr6:
    push 6 
    jmp isrHandler

isr7:
    push 7
    jmp isrHandler

isr8:
    push 8
    jmp isrHandler

isr9:
    push 9
    jmp isrHandler
    
isr10:
    push 10
    jmp isrHandler

isr11:
    push 11
    jmp isrHandler

isr12:
    push 12
    jmp isrHandler

isr13:
    push 13
    jmp isrHandler

isr14:
    push 14
    jmp isrHandler

isr15:
    push 15
    jmp isrHandler

isr16:
    push 16 
    jmp isrHandler

isr17:
    push 17
    jmp isrHandler

isr18:
    push 18
    jmp isrHandler

isr19:
    push 19
    jmp isrHandler

isr20:
    push 20
    jmp isrHandler

; isr 20 - 29 are reserved

isr30:
    push 30
    jmp isrHandler

testDiv:
    mov rdx, 0
    mov rax, 250
    mov rcx, 0
    div rcx
    ret
