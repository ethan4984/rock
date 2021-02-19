#ifndef EXT2_DIR_H_
#define EXT2_DIR_H_

#include <fs/ext2/types.h>
#include <fs/device.h>

int ext2_find_dir(struct devfs_node *devfs_node, struct ext2_inode *parent, struct ext2_dir *ret, char *path);
int ext2_create_dir(struct devfs_node *devfs_node, struct ext2_inode *parent, uint32_t parent_index, uint32_t inode, uint8_t type, char *name);
int ext2_delete_dir(struct devfs_node *devfs_node, struct ext2_inode *parent, char *name);

#endif
