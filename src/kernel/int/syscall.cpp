#include <kernel/int/syscall.h>
#include <lib/asmUtils.h>
#include <lib/output.h>

void syscallHandler() {
    kprintDS("[KDEBUG]", "Hello");
    for(;;);
}

void syscallInit() {
    uint64_t star = (((1ull << 31) + 0x1b) << 16) + 0x8;
    kprintDS("[KDEBUG]", "%x %b", star, star);
    wrmsr(0xc0000081, star);
    uint64_t rstar = rdmsr(0xc0000081); 
    kprintDS("[KDEBUG]", "reading this %x" , rstar);
    wrmsr(0xc0000082, (uint64_t)syscallHandler);
}

extern "C" void syscallMain(regs_t *regs) {
    asm volatile ("cli"); 
    for(;;);
    switch(regs->rax) {
        case 0: 
            break;  
        case 1:
            break;
    }
}
