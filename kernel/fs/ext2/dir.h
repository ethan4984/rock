#ifndef EXT2_DIR_H_
#define EXT2_DIR_H_

#include <fs/ext2/types.h>
#include <fs/device.h>

int ext2_find_dir(devfs_node_t *devfs_node, ext2_inode_t *parent, ext2_dir_t *ret, char *path);
int ext2_create_dir(devfs_node_t *devfs_node, ext2_inode_t *parent, uint32_t parent_index, uint32_t inode, uint8_t type, char *name);
int ext2_delete_dir(devfs_node_t *devfs_node, ext2_inode_t *parent, uint32_t parent_index, char *name);

#endif
