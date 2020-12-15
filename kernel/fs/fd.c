#include <fs/fd.h>

typedef struct {
    uint64_t flags, fd_index, exists, file_offset;
    char *path;
} fd_t;

static fd_t *fd;
static uint64_t max_fd = 0x200;

static int alloc_fd() {
    for(uint64_t i = 0; i < max_fd; i++) {
        if(fd[i].exists == 0) {
            fd[i].exists = 1;
            return i;
        }
    }
    return -1;
}

static void create_fd(uint64_t cnt) { 
    max_fd += cnt;
    fd = krealloc(fd, sizeof(fd_t) * max_fd);
}

static int is_valid_fd(int fd_idx) {
    if((fd_idx >= 0) || (fd_idx <= (int)max_fd)) {
        return 0;    
    }
    return -1;
}

int open(char *path, int flags) {
    int ret = alloc_fd();
    if(ret == -1) {
        create_fd(0x200);
        ret = alloc_fd();
    }

    fd[ret] = (fd_t) {  .path = path,
                        .flags = flags,
                        .fd_index = ret,
                        .exists = 1
                     };

    return ret;
}

int close(int fd_idx) {
    if(is_valid_fd(fd_idx) == -1)
        return -1;

    fd[fd_idx].exists = 0;
    return 0;
}

int read(int fd_idx, void *buf, uint64_t cnt) {
    if(is_valid_fd(fd_idx) == -1)
        return -1;

    int ret = fs_read(fd[fd_idx].path, fd[fd_idx].file_offset, cnt, buf);
    fd[fd_idx].file_offset += cnt;
    return ret;
} 

int write(int fd_idx, void *buf, uint64_t cnt) {
    if(is_valid_fd(fd_idx) == -1)
        return -1;

    int ret = fs_write(fd[fd_idx].path, fd[fd_idx].file_offset, cnt, buf);
    fd[fd_idx].file_offset += cnt;
    return ret;
}

int lseek(int fd_idx, off_t offset, int whence) {
    if(is_valid_fd(fd_idx) == -1)
        return -1;

    switch(whence) {
        case SEEK_SET:
            fd[fd_idx].file_offset = offset;
            break;
        case SEEK_CUR:
            fd[fd_idx].file_offset += offset;
            break;
        case SEEK_END:
            break;
        default:
            return -1;
    }

    return fd[fd_idx].file_offset;
}

int dup(int fd_idx) {
    int new_fd  = alloc_fd();
    if(new_fd == -1) {
        create_fd(0x1000);
        new_fd = alloc_fd();
    }

    if(is_valid_fd(fd_idx) == -1)
        return -1;

    fd[new_fd] = fd[fd_idx];
    return new_fd;
}

int dup2(int old_fd, int new_fd) {
    if((is_valid_fd(old_fd) == -1) || (is_valid_fd(new_fd) == -1))
        return -1;

    if(fd[new_fd].exists == 1)
        close(new_fd);

    fd[new_fd] = fd[old_fd];
    return new_fd;
}

void init_fd() {
    fd = kcalloc(sizeof(fd_t) * 0x200);
    open("stdin", 0);
    open("stdout", 0);
}
