#ifndef RAMFS_H_
#define RAMFS_H_

#include <fs/vfs.h>

typedef struct {
    vfs_node_t vfs_node;
    uint16_t perms;
    void *buf;
} ramfs_node_t;

int ramfs_write(vfs_node_t *vfs_node, off_t off, off_t cnt, void *buf);
int ramfs_read(vfs_node_t *vfs_node, off_t off, off_t cnt, void *buf);
int ramfs_open(vfs_node_t *node, int flags);
int ramfs_delete(vfs_node_t *node);

#endif
