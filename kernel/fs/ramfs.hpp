#ifndef RAMFS_HPP_
#define RAMFS_HPP_

#include <fs/vfs.hpp>

namespace ramfs {

class node {
public:
    node(vfs::node *vfs_node);
    node() = default; 
    ~node();

    int write(off_t off, off_t cnt, void *write_buf);
    int read(off_t off, off_t cnt, void *read_buf);

    friend int vfs2ramfs(vfs::node *vfs_node, node **ret);
private:
    vfs::node *vfs_node;
    uint8_t *buf;
};

struct fs : vfs::fs {
    int raw_read(vfs::node *vfs_node, off_t off, off_t cnt, void *buf); 
    int raw_write(vfs::node *vfs_node, off_t off, off_t cnt, void *buf);
    int raw_mkdir(vfs::node *vfs_node);
    int raw_open(vfs::node *vfs_node, uint16_t status);
    int raw_unlink(vfs::node *vfs_node);
};

};

#endif
