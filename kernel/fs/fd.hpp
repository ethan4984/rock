#ifndef FD_HPP_
#define FD_HPP_

#include <fs/vfs.hpp>

namespace fs {

struct fd {
    fd(lib::string path, int flags, int backing_fd);
    fd(int backing_fd);
    fd() : status(0), backing_fd(-1), _loc(NULL), dirent(false), vfs_node(NULL) { }

    int read(void *buf, size_t cnt);
    int write(void *buf, size_t cnt);
    int seek(off_t off, int whence);

    int status;
    int backing_fd;
    
    size_t *_loc;
    size_t *_flags;
    bool dirent;

    vfs::node *vfs_node;
};

fd &alloc_fd(lib::string path, int flags);
void free_fd(size_t index);

}

#endif
