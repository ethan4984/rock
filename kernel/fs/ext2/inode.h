#ifndef EXT2_INODE_H_
#define EXT2_INODE_H_

#include <fs/device.h>
#include <fs/ext2/types.h>

uint32_t ext2_alloc_inode(struct devfs_node *devfs_node);
void ext2_free_inode(struct devfs_node *devfs_node, uint32_t index);
void ext2_inode_read(struct devfs_node *devfs_node, struct ext2_inode *inode, off_t off, off_t cnt, void *buf);
void ext2_inode_write(struct devfs_node *devfs_node, struct ext2_inode *inode, uint32_t inode_index, off_t off, off_t cnt, void *buf);
void ext2_inode_delete(struct devfs_node *devfs_node, struct ext2_inode *inode, uint32_t inode_index);

struct ext2_inode ext2_inode_read_entry(struct devfs_node *devfs_node, uint32_t index);
void ext2_inode_write_entry(struct devfs_node *devfs_node, struct ext2_inode *inode, uint32_t index);

int inode_set_block(struct devfs_node *devfs_node, struct ext2_inode *inode, uint32_t inode_index, uint32_t iblock, uint32_t disk_block);
uint32_t inode_get_block(struct devfs_node *devfs_node, struct ext2_inode *inode, uint32_t iblock);

#endif
