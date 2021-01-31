#ifndef EXT2_INODE_H_
#define EXT2_INODE_H_

#include <fs/device.h>
#include <fs/ext2/types.h>

uint32_t ext2_alloc_inode(devfs_node_t *devfs_node);
void ext2_free_inode(devfs_node_t *devfs_node, uint32_t index);
void ext2_inode_read(devfs_node_t *devfs_node, ext2_inode_t *inode, off_t off, off_t cnt, void *buf);
void ext2_inode_write(devfs_node_t *devfs_node, ext2_inode_t *inode, uint32_t inode_index, off_t off, off_t cnt, void *buf);
void ext2_inode_delete(devfs_node_t *devfs_node, ext2_inode_t *inode, uint32_t inode_index);

ext2_inode_t ext2_inode_read_entry(devfs_node_t *devfs_node, uint32_t index);
void ext2_inode_write_entry(devfs_node_t *devfs_node, ext2_inode_t *inode, uint32_t index);

int inode_set_block(devfs_node_t *devfs_node, ext2_inode_t *inode, uint32_t inode_index, uint32_t iblock, uint32_t disk_block);
uint32_t inode_get_block(devfs_node_t *devfs_node, ext2_inode_t *inode, uint32_t iblock);

#endif
