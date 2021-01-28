#ifndef EXT2_H_
#define EXT2_H_

#include <fs/ext2/ext2.h>
#include <fs/vfs.h>

int ext2_check_fs(devfs_node_t *devfs_node);

int ext2_read(vfs_node_t *vfs_node, off_t off, off_t cnt, void *buf);
int ext2_write(vfs_node_t *vfs_node, off_t off, off_t cnt, void *buf);
int ext2_mkdir(vfs_node_t *vfs_node, uint16_t perms);
int ext2_open(vfs_node_t *vfs_node, int flags);
int ext2_unlink(vfs_node_t *vfs_node);
int ext2_refresh(vfs_node_t *vfs_node);

#endif
