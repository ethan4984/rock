#ifndef FD_H_
#define FD_H_

#include <sched/scheduler.h>
#include <fs/vfs.h>

struct fd {
    int fd;
    struct vfs_node *vfs_node;
    void *buf;
    size_t *loc;
    int *flags;
    int mode;
};

int fd_open_task(struct task *task, char *path, int flags);
int open(char *path, int flags);
int close(int fd);
int read(int fd, void *buf, size_t cnt);
int write(int fd, void *buf, size_t cnt);
int lseek(int fd, off_t off, int whence);
int dup(int fd);
int dup2(int old_fd, int new_fd);

#endif
