#ifndef EXT2_HPP_
#define EXT2_HPP_

#include <fs/devfs.hpp>

namespace ext2 {

struct bgd {
    uint32_t block_addr_bitmap;
    uint32_t block_addr_inode;
    uint32_t inode_table_block;
    uint16_t unallocated_blocks;
    uint16_t unallocated_inodes;
    uint16_t dir_cnt;
    uint16_t reserved[7];
};

struct [[gnu::packed]] superblock { 
    uint32_t inode_cnt;
    uint32_t block_cnt;
    uint32_t sb_reserved;
    uint32_t unallocated_blocks;
    uint32_t unallocated_inodes;
    uint32_t sb_block;
    uint32_t block_size;
    uint32_t frag_size;
    uint32_t blocks_per_group; 
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mnt_time;
    uint32_t last_written_time;
    uint16_t mnt_cnt;
    uint16_t mnt_allowed;
    uint16_t signature; 
    uint16_t fs_state;
    uint16_t error_response;
    uint16_t version_min;
    uint32_t last_fsck;
    uint32_t forced_fsck;
    uint32_t os_id;
    uint32_t version_maj; 
    uint16_t user_id;
    uint16_t group_id;

    uint32_t first_inode;
    uint16_t inode_size;
    uint16_t sb_bgd;
    uint32_t opt_features;
    uint32_t req_features; 
    uint32_t non_supported_features;
    uint64_t uuid[2];
    uint64_t volume_name[2];
    uint64_t last_mnt_path[8];
};

struct [[gnu::packed]] inode {
    uint16_t permissions;
    uint16_t user_id;
    uint32_t size32l;
    uint32_t access_time;
    uint32_t creation_time;
    uint32_t mod_time;
    uint32_t del_time;
    uint16_t group_id;
    uint16_t hard_link_cnt;
    uint32_t sector_cnt;
    uint32_t flags;
    uint32_t oss1;
    uint32_t blocks[15];
    uint32_t gen_num;
    uint32_t eab;
    uint32_t size32h;
    uint32_t frag_addr;
};

struct [[gnu::packed]] dir_entry {
    uint32_t inode;
    uint16_t entry_size;
    uint8_t name_length;
    uint8_t type;
    char name[];
};

class fs {
public:
    fs(dev::node &devfs_node);
    fs() = default;

    int open(vfs::node *vfs_node, int flags);
    int read(vfs::node *vfs_node, off_t off, off_t cnt, void *buf);
    int write(vfs::node *vfs_node, off_t off, off_t cnt, void *buf);
    int refresh(vfs::node *vfs_node);
    int unlink(vfs::node *vfs_node);
private:
    uint32_t bgd_find_block(uint32_t bgd_index);
    uint32_t alloc_block();
    void free_block(uint32_t block);
    bgd read_bgd(uint32_t index);
    void write_bgd(bgd *buf, uint32_t index);

    uint32_t alloc_inode();
    void free_inode(uint32_t index);
    uint32_t inode_get_block(inode *i, uint32_t block);
    uint32_t inode_set_block(inode *i, uint32_t iblock);
    void inode_delete(inode *i, uint32_t index);
    inode inode_read_entry(uint32_t index);
    void inode_write_entry(inode *i, uint32_t index);

    superblock superb;
    size_t block_size;
    size_t frag_size;
    uint32_t bgd_cnt;

    dev::node devfs_node;
};

}

#endif
