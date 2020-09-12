#include <kernel/int/syscall.h>
#include <kernel/fs/ext2/ext2.h>
#include <lib/asmUtils.h>
#include <lib/output.h>

namespace kernel {

extern "C" void syscallMain(regs_t *regs) {
    switch(regs->r15) {
        case 0:
            switch(regs->r14) {
                case 0:
                    ext2.getDir((inode_t*)regs->r13, (directory_t*)regs->r12);
                    break;
                case 1:
                    ext2.read((const char*)regs->r13, regs->r12, regs->r11, (void*)regs->r10);
            }
            break;
        case 1:
            switch(regs->r14) {
                switch(regs->r14) {
                    case 0:
                        serialWrite((uint8_t)regs->r13);
                }
            }
            break;
    }
}

}
