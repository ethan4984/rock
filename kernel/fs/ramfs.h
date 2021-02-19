#ifndef RAMFS_H_
#define RAMFS_H_

#include <fs/vfs.h>

struct ramfs_node {
    struct vfs_node vfs_node;
    uint16_t perms;
    void *buf;
};

int ramfs_write(struct vfs_node *vfs_node, off_t off, off_t cnt, void *buf);
int ramfs_read(struct vfs_node *vfs_node, off_t off, off_t cnt, void *buf);
int ramfs_open(struct vfs_node *node, int flags);
int ramfs_delete(struct vfs_node *node);

#endif
