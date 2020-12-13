global syscall_main_stub

extern read
dq read ; rax = 0
extern write
dq write ; rax = 1
extern lseek
dq lseek ; rax = 2
extern close
dq close ; rax = 3
extern open
dq open ; rax = 4
extern dup
dq dup ; rax = 5
extern dup2
dq dup2 ; rax = 6
extern mmap
dq mmap ; rax = 7
extern getpid
dq getpid ; rax = 8
extern getppid
dq getppid ; rax = 9

syscall_main_stub:
    jmp $ 
