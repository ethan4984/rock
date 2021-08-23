#include <cpu.hpp>
#include <debug.hpp>
#include <memutils.hpp>
#include <sched/smp.hpp>

extern "C" void syscall_open(regs*);
extern "C" void syscall_close(regs*);
extern "C" void syscall_read(regs*);
extern "C" void syscall_write(regs*);
extern "C" void syscall_seek(regs*);
extern "C" void syscall_dup(regs*);
extern "C" void syscall_dup2(regs*);
extern "C" void syscall_mmap(regs*);
extern "C" void syscall_munmap(regs*);
extern "C" void syscall_set_fs_base(regs*);
extern "C" void syscall_set_gs_base(regs*);
extern "C" void syscall_get_fs_base(regs*);
extern "C" void syscall_get_gs_base(regs*);
extern "C" void syscall_syslog(regs*);
extern "C" void syscall_exit(regs*);
extern "C" void syscall_getpid(regs*);
extern "C" void syscall_gettid(regs*);
extern "C" void syscall_getppid(regs*);
extern "C" void syscall_isatty(regs*);
extern "C" void syscall_fcntl(regs*);
extern "C" void syscall_fstat(regs*);
extern "C" void syscall_fstatat(regs*);
extern "C" void syscall_ioctl(regs*);
extern "C" void syscall_fork(regs*);
extern "C" void syscall_waitpid(regs*);
extern "C" void syscall_readdir(regs*);
extern "C" void syscall_execve(regs*);
extern "C" void syscall_getcwd(regs*);
extern "C" void syscall_chdir(regs*);

struct syscall {
    lib::string name;
    void (*logger)(regs*);
    void (*handler)(regs*);
    bool has_return;
};

syscall syscall_list[] = {
    { .name = "open", .logger = [](regs *regs_cur) { print("SYSCALL: open { path: {}, flags {x} }\n", (char*)regs_cur->rdi, regs_cur->rsi); }, .handler = syscall_open, .has_return = true },
    { .name = "close", .logger = [](regs *regs_cur) { print("SYSCALL: close { fd: {} }\n", regs_cur->rdi); }, .handler = syscall_close, .has_return = true },
    { .name = "read", .logger = [](regs *regs_cur) { print("SYSCALL: read {fd: {x}, buf: {x}, cnt: {x} }\n", regs_cur->rdi, regs_cur->rsi, regs_cur->rdx); }, .handler = syscall_read, .has_return = true },
    { .name = "write", .logger = [](regs *regs_cur) { print("SYSCALL: write {fd: {x}, buf: {x}, cnt: {x} }\n", regs_cur->rdi, regs_cur->rsi, regs_cur->rdx); }, .handler = syscall_write, .has_return = true },
    { .name = "seek", .logger = [](regs *regs_cur) { print("SYSCALL: seek { fd: {x}, off: {x}, whence {x} }\n", regs_cur->rdi, regs_cur->rsi, regs_cur->rdx); }, .handler = syscall_seek, .has_return = true },
    { .name = "dup", .logger = [](regs *regs_cur) { print("SYSCALL: dup { oldfd {x} }\n", regs_cur->rdi); }, .handler = syscall_dup, .has_return = true },
    { .name = "dup2", .logger = [](regs *regs_cur) { print("SYSCALL: dup2 { oldfd {x}, newfd {x} }\n", regs_cur->rdi, regs_cur->rsi); }, .handler = syscall_dup2, .has_return = true },
    { .name = "mmap", .logger = [](regs *regs_cur) { print("SYSCALL: mmap { addr {x}, len {x}, prot {x}, flags {x}, fd {x}, off {x} }\n", regs_cur->rdi, regs_cur->rsi, regs_cur->rdx, regs_cur->r10, regs_cur->r8, regs_cur->r9); }, .handler = syscall_mmap, .has_return = true },
    { .name = "munmap", .logger = [](regs *regs_cur) { print("SYSCALL: munmap { addr {x}, len {x} }\n", regs_cur->rdi, regs_cur->rsi); }, .handler = syscall_munmap, .has_return = true },
    { .name = "set_fs_base", .logger = [](regs *regs_cur) { print("SYSCALL: set_fs_base { addr {x} }\n", regs_cur->rdi); }, .handler = syscall_set_fs_base, .has_return = false },
    { .name = "set_gs_base", .logger = [](regs *regs_cur) { print("SYSCALL: set_gs_base { addr {x} }\n", regs_cur->rdi); }, .handler = syscall_set_gs_base, .has_return = false },
    { .name = "get_fs_base", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: get_fs_base { }\n"); }, .handler = syscall_get_fs_base, .has_return = true },
    { .name = "get_gs_base", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: get_gs_base { }\n"); }, .handler = syscall_get_gs_base, .has_return = true },
    { .name = "syslog", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: syslog { }\n"); }, .handler = syscall_syslog, .has_return = false },
    { .name = "exit", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: exit { }\n"); }, .handler = syscall_exit, .has_return = false },
    { .name = "getpid", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: getpid { }\n"); }, .handler = syscall_getpid, .has_return = true },
    { .name = "gettid", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: gettid { }\n"); }, .handler = syscall_gettid, .has_return = true },
    { .name = "getppid", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: getppid { }\n"); }, .handler = syscall_getppid, .has_return = true },
    { .name = "isatty", .logger = [](regs *regs_cur) { print("SYSCALL: isatty { fd {x} }\n", regs_cur->rdi); }, .handler = syscall_isatty, .has_return = true },
    { .name = "fcntl", .logger = [](regs *regs_cur) { print("SYSCALL: fcntl { fd {x}, cmd {x} }\n", regs_cur->rdi, regs_cur->rsi); }, .handler = syscall_fcntl, .has_return = true },
    { .name = "fstat", .logger = [](regs *regs_cur) { print("SYSCALL: fstat { fd {x}, buf {x} }\n", regs_cur->rdi, regs_cur->rsi); }, .handler = syscall_fstat, .has_return = true },
    { .name = "fstatat", .logger = [](regs *regs_cur) { print("SYSCALL: fstatat { dirfd {x}, path {}, buf {x}, flags {x} }\n", regs_cur->rdi, (char*)regs_cur->rsi, regs_cur->rdx, regs_cur->r10); }, .handler = syscall_fstatat, .has_return = true },
    { .name = "ioctl", .logger = [](regs *regs_cur) { print("SYSCALL: ioctl { fd {x}, req {x} }\n", regs_cur->rdi, regs_cur->rsi); }, .handler = syscall_ioctl, .has_return = true },
    { .name = "fork", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: fork { }\n"); }, .handler = syscall_fork, .has_return = true },
    { .name = "waitpid", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: waitpid { pid: {x}, status {x}, flags {x} }\n", regs_cur->rdi, regs_cur->rsi, regs_cur->rdx); }, .handler = syscall_waitpid, .has_return = true },
    { .name = "readdir", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: readdir { fd: {x}, buf {x} }\n", regs_cur->rdi, regs_cur->rsi); }, .handler = syscall_readdir, .has_return = true },
    { .name = "execve", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: execve { }\n"); }, .handler = syscall_execve, .has_return = false },
    { .name = "getcwd", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: getcwd { buffer: {x}, length: {x} }\n", regs_cur->rdi, regs_cur->rsi); }, .handler = syscall_getcwd, .has_return = true },
    { .name = "chdir", .logger = []([[maybe_unused]] regs *regs_cur) { print("SYSCALL: chdir { path: {} }\n", (char*)regs_cur->rdi); }, .handler = syscall_chdir, .has_return = true },
};

extern "C" void syscall_view(regs *regs_cur) {
    if(lengthof(syscall_list) < regs_cur->rax) {
        return;
    }

    size_t index = regs_cur->rax;

    print("\e[36m");

    static char lock = 0;

    spin_lock(&lock);
    syscall_list[index].logger(regs_cur);
    spin_release(&lock);

    syscall_list[index].handler(regs_cur);

    if(syscall_list[index].has_return) {
        print("\e[31m");
        print("SYSCALL: {} returning {x}\n", syscall_list[index].name, regs_cur->rax);
    }

    print("\e[39m");
}
