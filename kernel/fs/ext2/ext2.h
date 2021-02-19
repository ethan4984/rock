#ifndef EXT2_H_
#define EXT2_H_

#include <fs/ext2/ext2.h>
#include <fs/vfs.h>

int ext2_check_fs(struct devfs_node *devfs_node);

int ext2_read(struct vfs_node *vfs_node, off_t off, off_t cnt, void *buf);
int ext2_write(struct vfs_node *vfs_node, off_t off, off_t cnt, void *buf);
int ext2_mkdir(struct vfs_node *vfs_node, uint16_t perms);
int ext2_open(struct vfs_node *vfs_node, int flags);
int ext2_unlink(struct vfs_node *vfs_node);
int ext2_refresh(struct vfs_node *vfs_node);

#endif
