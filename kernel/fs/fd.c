#include <fs/fd.h>
#include <types.h>

static_hash_table(fd_t, fd_list);

int open(char *path, int flags) {
    if(vfs_open(path, flags) == -1)
        return -1;

    vfs_node_t *node = vfs_absolute_path(path);

    if(node == NULL) {
        return -1;
    }

    fd_t fd = { .vfs_node = node,
                .flags = kmalloc(sizeof(int)),
                .loc = kmalloc(sizeof(size_t))
              };

    *fd.flags = flags;
    *fd.loc = 0;

    return hash_push(fd_t, fd_list, fd);
}

void syscall_open(regs_t *regs) {
    regs->rax = open((void*)regs->rdi, (int)regs->rsi);
}

int read(int fd, void *buf, size_t cnt) {
    fd_t *fd_entry = hash_search(fd_t, fd_list, (size_t)fd);
    if(fd_entry == NULL)
        return -1;

    int ret = vfs_read(fd_entry->vfs_node, *fd_entry->loc, cnt, buf);
    if(ret == -1) 
        return -1;

    *fd_entry->loc += cnt;

    return ret;
}

void syscall_read(regs_t *regs) {
    regs->rax = read((int)regs->rdi, (void*)regs->rsi, regs->rdx);
}

int write(int fd, void *buf, size_t cnt) {
    fd_t *fd_entry = hash_search(fd_t, fd_list, (size_t)fd);
    if(fd_entry == NULL)
        return -1;

    int ret = vfs_write(fd_entry->vfs_node, *fd_entry->loc, cnt, buf);
    if(ret == -1) 
        return -1;

    *fd_entry->loc += cnt;

    return ret;
}

void syscall_write(regs_t *regs) {
    regs->rax = write((int)regs->rdi, (void*)regs->rsi, regs->rdx);
}

int lseek(int fd, off_t off, int whence) {
    fd_t *fd_entry = hash_search(fd_t, fd_list, (size_t)fd);
    if(fd_entry == NULL)
        return -1;

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

void syscall_lseek(regs_t *regs) {
    regs->rax = lseek((int)regs->rdi, (off_t)regs->rsi, (int)regs->rdx);
}

int close(int fd) {
    fd_t *fd_entry = hash_search(fd_t, fd_list, (size_t)fd);
    if(fd_entry == NULL)
        return -1;
    
    return hash_addr_remove(fd_t, fd_list, fd_entry);
}

void syscall_close(regs_t *regs) {
    regs->rax = close((int)regs->rdi);
}

int dup(int fd) {
    fd_t *fd_entry = hash_search(fd_t, fd_list, (size_t)fd);
    if(fd_entry == NULL)
        return -1;

    fd_t new_fd = *fd_entry;

    return hash_push(fd_t, fd_list, new_fd);
}

void syscall_dup(regs_t *regs) {
    regs->rax = dup((int)regs->rdi);
}

int dup2(int old_fd, int new_fd) {
    fd_t *old_fd_entry = hash_search(fd_t, fd_list, (size_t)old_fd);
    if(old_fd_entry == NULL)
        return -1;

    fd_t *new_fd_entry = hash_search(fd_t, fd_list, (size_t)new_fd);
    if(new_fd_entry != NULL) {
        close(new_fd);
    }

    fd_t fd = *old_fd_entry;

    return hash_push(fd_t, fd_list, fd);
}

void syscall_dup2(regs_t *regs) {
    regs->rax = dup2((int)regs->rdi, (int)regs->rsi);
}

void syscall_stat(regs_t *regs) {
    char *path = (void*)regs->rdi;
    stat_t *stat = (void*)regs->rsi;

    int fd = open(path, 0);
    if(fd == -1) {
        regs->rax = -1;
        return;
    }

    fd_t *fd_struct = hash_search(fd_t, fd_list, (size_t)fd);
    *stat = fd_struct->vfs_node->stat;
    regs->rax = 0;
}

void syscall_fstat(regs_t *regs) {
    int fd = (int)regs->rdi;
    stat_t *stat = (void*)regs->rsi;

    fd_t *fd_struct = hash_search(fd_t, fd_list, (size_t)fd);
    if(fd_struct == NULL) {
        regs->rax = -1;
        return;
    }
    
    *stat = fd_struct->vfs_node->stat; 
    regs->rax = 0;
}
