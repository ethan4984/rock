#ifndef EXT2_BLOCKS_H_
#define EXT2_BLOCKS_H_

#include <fs/device.h>
#include <fs/ext2/types.h>

ext2_bgd_t ext2_read_bgd(devfs_node_t *devfs_node, uint32_t index);
void ext2_write_bgd(devfs_node_t *devfs_node, ext2_bgd_t *bgd, uint32_t index);

void ext2_free_block(devfs_node_t *devfs_node, uint32_t block);
uint32_t ext2_alloc_block(devfs_node_t *devfs_node);

#endif
