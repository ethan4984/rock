#include <sched/scheduler.h>
#include <fs/fd.h>
#include <types.h>
#include <debug.h>

static_hash_table(struct fd, fd_list);

static int translate_internal_fd(int fd) {
    struct core_local *core_local = get_core_local(CURRENT_CORE);
    struct task *current_task = hash_search(struct task, tasks, core_local->pid);

    int *ptr = hash_search(int, current_task->fd_list, fd);
    if(ptr == NULL) { 
        set_errno(EBADF);
        return -1;
    }

    return *ptr;
}

int fd_open_task(struct task *task, char *path, int flags) {
    int fd = open(path, flags);
    return hash_push(int, task->fd_list, fd);
}

int open(char *path, int flags) {
    if(vfs_open(path, flags) == -1)
        return -1;

    struct vfs_node *node = vfs_absolute_path(path);

    if(node == NULL) {
        return -1;
    }

    struct fd fd = {    .vfs_node = node,
                        .flags = kmalloc(sizeof(int)),
                        .loc = kmalloc(sizeof(size_t))
                   };

    *fd.flags = flags;
    *fd.loc = 0;

    return hash_push(struct fd, fd_list, fd);
}

int syscall_open(struct regs *regs) {
    int fd = open((void*)regs->rdi, (int)regs->rsi);

    if(fd == -1) {
        regs->rax = -1;
        return -1;
    }

    struct core_local *core_local = get_core_local(CURRENT_CORE);
    struct task *current_task = hash_search(struct task, tasks, core_local->pid);

    regs->rax = hash_push(int, current_task->fd_list, fd);

    return regs->rax;
}

int read(int fd, void *buf, size_t cnt) {
    struct fd *fd_entry = hash_search(struct fd, fd_list, (size_t)fd);
    if(fd_entry == NULL) {
        set_errno(EBADF);
        return -1;
    }

    int ret = vfs_read(fd_entry->vfs_node, *fd_entry->loc, cnt, buf);
    if(ret == -1) {
        return -1;
    }

    *fd_entry->loc += cnt;

    return ret;
}

int syscall_read(struct regs *regs) {
    int internal_fd = translate_internal_fd(regs->rdi);
    if(internal_fd == -1) {
        regs->rax = -1;
        return -1;
    }

    regs->rax = read(internal_fd, (void*)regs->rsi, regs->rdx);

    return regs->rax;
}

int write(int fd, void *buf, size_t cnt) {
    struct fd *fd_entry = hash_search(struct fd, fd_list, (size_t)fd);
    if(fd_entry == NULL) {
        set_errno(EBADF);
        return -1;
    }

    int ret = vfs_write(fd_entry->vfs_node, *fd_entry->loc, cnt, buf);
    if(ret == -1) 
        return -1;

    *fd_entry->loc += cnt;

    return ret;
}

int syscall_write(struct regs *regs) {
    int internal_fd = translate_internal_fd(regs->rdi);
    if(internal_fd == -1) {
        regs->rax = -1;
        return -1;
    }

    if(regs->rdi == 1) {
        char *str = kcalloc(regs->rdx + 1);
        strncpy(str, (void*)regs->rsi, regs->rdx);
        kprintf("stdout: %s\n", str);
    }

    regs->rax = write(internal_fd, (void*)regs->rsi, regs->rdx);

    return regs->rax;
}

int lseek(int fd, off_t off, int whence) {
    struct fd *fd_entry = hash_search(struct fd, fd_list, (size_t)fd);
    if(fd_entry == NULL) {
        set_errno(EBADF);
        return -1;
    }

    switch(whence) {
        case SEEK_SET:
            return (*fd_entry->loc = off); 
        case SEEK_CUR:
            return (*fd_entry->loc += off);
        case SEEK_END:
            return (*fd_entry->loc = fd_entry->vfs_node->stat.st_size + off);
    }

    return -1;
}

int syscall_lseek(struct regs *regs) {
    int internal_fd = translate_internal_fd(regs->rdi);
    if(internal_fd == -1) {
        regs->rax = -1;
        return -1;
    }

    regs->rax = lseek(internal_fd, (off_t)regs->rsi, (int)regs->rdx);

    return regs->rax;
}

int close(int fd) {
    struct fd *fd_entry = hash_search(struct fd, fd_list, (size_t)fd);
    if(fd_entry == NULL) {
        set_errno(EBADF);
        return -1;
    }
    
    return hash_remove(struct fd, fd_list, (size_t)fd);
}

int syscall_close(struct regs *regs) {
    int internal_fd = translate_internal_fd(regs->rdi);
    if(internal_fd == -1) {
        regs->rax = -1;
        return -1;
    }

    struct core_local *core_local = get_core_local(CURRENT_CORE);
    struct task *current_task = hash_search(struct task, tasks, core_local->pid);

    close(internal_fd);
    hash_remove(int, current_task->fd_list, regs->rdi);
    regs->rax = regs->rdi;

    return regs->rax;
}

int dup(int fd) {
    struct fd *fd_entry = hash_search(struct fd, fd_list, (size_t)fd);
    if(fd_entry == NULL) {
        set_errno(EBADF);
        return -1;
    }

    struct fd new_fd = *fd_entry;

    return hash_push(struct fd, fd_list, new_fd);
}

int syscall_dup(struct regs *regs) {
    int internal_fd = translate_internal_fd(regs->rdi);
    if(internal_fd == -1) {
        regs->rax = -1;
        return -1;
    }

    regs->rax = dup(internal_fd);

    return regs->rax;
}

int dup2(int old_fd, int new_fd) {
    struct fd *old_fd_entry = hash_search(struct fd, fd_list, (size_t)old_fd);
    if(old_fd_entry == NULL) { 
        set_errno(EBADF);
        return -1;
    }

    struct fd *new_fd_entry = hash_search(struct fd, fd_list, (size_t)new_fd);
    if(new_fd_entry != NULL) {
        close(new_fd);
    }

    struct fd fd = *old_fd_entry;

    return hash_push(struct fd, fd_list, fd);
}

int syscall_dup2(struct regs *regs) {
    int internal_fd = translate_internal_fd(regs->rdi);
    if(internal_fd == -1) {
        regs->rax = -1;
        return -1;
    }

    regs->rax = dup2(internal_fd, (int)regs->rsi);

    return regs->rax;
}

void syscall_stat(struct regs *regs) {
    char *path = (void*)regs->rdi;
    struct stat *stat = (void*)regs->rsi;

    struct regs open_regs = { .rdi = regs->rdi, .rsi = 0 };

    int fd = syscall_open(&open_regs);
    if(fd == -1) {
        regs->rax = -1;
        return;
    }

    struct fd *fd_struct = hash_search(struct fd, fd_list, (size_t)fd);
    *stat = fd_struct->vfs_node->stat;
    regs->rax = 0;
}

int syscall_fstat(struct regs *regs) {
    int internal_fd = translate_internal_fd(regs->rdi);
    if(internal_fd == -1) {
        regs->rax = -1;
        return -1;
    }
    
    struct stat *stat = (void*)regs->rsi;

    struct fd *fd_struct = hash_search(struct fd, fd_list, (size_t)internal_fd);
    if(fd_struct == NULL) {
        regs->rax = -1;
        return -1;
    }
    
    *stat = fd_struct->vfs_node->stat; 
    regs->rax = 0;

    return 0;
}

#define F_DUPFD 1
#define F_DUPFD_CLOEXEC 2
#define F_GETFD 3
#define F_SETFD 4
#define F_GETFL 5
#define F_SETFL 6
#define F_GETLK 7
#define F_SETLK 8
#define F_SETLKW 9
#define F_GETOWN 10
#define F_SETOWN 11

#define FD_CLOEXEC 1

void syscall_fcntl(struct regs *regs) {
    int internal_fd = translate_internal_fd(regs->rdi);
    if(internal_fd == -1) {
        regs->rax = -1;
        return;
    }

    int cmd = (int)regs->rsi;

    struct fd *fd_struct = hash_search(struct fd, fd_list, internal_fd);
    if(fd_struct == NULL) {
        regs->rax = -1;
        return;
    }

    switch(cmd) {
        case F_DUPFD: 
            regs->rax = (size_t)dup2(internal_fd, (int)regs->rdx);
            return;
        case F_GETFD:
            regs->rax = (size_t)((regs->rdx & FD_CLOEXEC) ? O_CLOEXEC : 0);
            return; 
        case F_SETFD:
            *fd_struct->flags = (int)((regs->rdx & FD_CLOEXEC) ? O_CLOEXEC : 0);
            break;
        case F_GETFL:
            regs->rax = (size_t)fd_struct->flags; 
            return;
        case F_SETFL:
            *fd_struct->flags = (int)regs->rdx;
            break;
        default:
            regs->rax = -1;
            return; 
    }

    regs->rax = 0;
}
