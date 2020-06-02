#include <Kernel/int/isr.h>
#include <Slib/videoIO.h>
#include <Slib/stringUtils.h>

#include <stdarg.h>

using namespace out;

extern "C" void isrHandlerMain(void *stack)
{
    registers_t *registers = (registers_t*)stack;
 
    kPrint("\n%s [excpetion index] %d\n", isrMessages[registers->exceptionIndex], registers->exceptionIndex);
    
    kPrint("\n[RAX] %a [RBX] %a [RCX] %a [RDX] %a\n", registers->rax, registers->rbx, registers->rcx, registers->rdx);
    kPrint("[RSP] %a [RBP] %a [RDI] %a [RSI] %a\n", stack, registers->rbp, registers->rdi, registers->rsi);
    kPrint("\n-------------------------------------------------------\n\n");
    kPrint("[R8 ] %a [R9 ] %a [R10] %a [R11] %a\n", registers->r8, registers->r9, registers->r10, registers->r11);
    kPrint("[R12] %a [R13] %a [R14] %a [R15] %a\n\n", registers->r12, registers->r13, registers->r14, registers->r15);

    for(;;) {
        asm volatile ("hlt");
    }
}

void panic(const char *str, ...) 
{
    asm volatile ("cli");

    char errorStr[256];
    uint16_t index = 0;

    uint64_t hold = 0;
    char *string;

    va_list arg;
    va_start(arg, str);

    for(uint64_t i = 0; i < strlen(str); i++) {
        if(str[i] != '%')
            errorStr[index++] = str[i]; 
        else {
            i++;
            switch(str[i]) {
                case 'd':
                    hold = va_arg(arg, long);
                    string = itob(hold, 10);
                    for(uint64_t i = 0; i < strlen(string); i++)
                        errorStr[index++] = string[i];
                    break;
                case 's':
                    string = va_arg(arg, char*);
                    for(uint64_t i = 0; i < strlen(string); i++)
                        errorStr[index++] = string[i]; 
                    break;
                case 'x':
                    hold = va_arg(arg, uint64_t);
                    string = itob(hold, 16);
                    errorStr[index++] = '0'; 
                    errorStr[index++] = 'x'; 
                    for(uint64_t i = 0; i < strlen(string); i++)
                        errorStr[index++] = string[i];  
                    break;
            }
        }
    }

    kPrint("\n%s\n", errorStr);

    /*kPrint("\n[RAX] %x [RBX] %x [RCX] %x [RDX]\n", registers.rax, registers.rbx, registers.rcx, registers.rdx);
    kPrint("[RSP] %x [RBP] %x [RDI] %X [RSI] %x\n", registers.rsp, registers.rbp, registers.rdi, registers.rsi);
    kPrint("[R8] %x [R9] %x [R10] %x [R11] %x\n", registers.r8, registers.r9, registers.r10, registers.r11);
    kPrint("[R12] %x [R13] %x [R14] %x [R15] %x\n\n", registers.r12, registers.r13, registers.r14, registers.r15);
    kPrint("[SS] %x [CS] %x [DS] %x [ES] %x [GS] %x [FS]\n", registers.ss, registers.cs, registers.es, registers.gs, registers.fs);*/

    for(;;)
        asm volatile ("hlt");
}
