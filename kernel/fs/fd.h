#ifndef FD_H_
#define FD_H_

#include <fs/vfs.h>
#include <bitmap.h>

enum {
    SEEK_SET = 1,
    SEEK_CUR,
    SEEK_END
};

typedef int64_t off_t;

int open(char *path, int flags);

int close(int fd);

int read(int fd, void *buf, uint64_t cnt);

int write(int fd, void *buf, uint64_t cnt);

int lseek(int fd, off_t offset, int whence);

int dup(int fd);

int dup2(int old_fd, int new_fd);

void init_fd();

#endif
