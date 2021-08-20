#ifndef DEVFS_HPP_
#define DEVFS_HPP_

#include <fs/vfs.hpp>

namespace dev {

struct cache_block {
    size_t block;
    void *buffer;
};

struct msd {
    virtual ssize_t raw_read([[maybe_unused]] size_t off, [[maybe_unused]] size_t cnt, [[maybe_unused]] void *buf) {
        return -1;
    }

    virtual ssize_t raw_write([[maybe_unused]] size_t off, [[maybe_unused]] size_t cnt, [[maybe_unused]] void *buf) {
        return -1;
    }

    ssize_t read(size_t off, size_t cnt, void *buf);
    ssize_t write(size_t off, size_t cnt, void *buf);

    void *search_cache(size_t block);

    lib::string device_prefix;
    ssize_t device_index;
    ssize_t partition_index;
    ssize_t partition_cnt;
    ssize_t sector_size;
    ssize_t sector_cnt;
    size_t device_type;
    size_t cache_block_size;

    lib::vector<cache_block*> cache_list;
};

class node {
public:
    node(vfs::node *vfs_node, size_t partition_offset, size_t sector_cnt, msd *device) : vfs_node(vfs_node), device(device), partition_offset(partition_offset), sector_cnt(sector_cnt) { }
    node(vfs::node *vfs_node, size_t sector_cnt, msd *device) : vfs_node(vfs_node), device(device), sector_cnt(sector_cnt) { }
    node(vfs::node *vfs_node);
    node() = default;
    
    ssize_t read(size_t start, size_t cnt, void *ret);
    ssize_t write(size_t start, size_t cnt, void *ret);

    vfs::fs *filesystem;
    vfs::node *vfs_node;
    msd *device;
private:
    size_t partition_offset;
    size_t sector_cnt;
};

struct [[gnu::packed]] mbr_partition {
    uint8_t drive_status;
    uint8_t starting_chs[3];
    uint8_t partition_type;
    uint8_t ending_chs[3];
    uint32_t starting_lba;
    uint32_t sector_cnt;
};

struct [[gnu::packed]] gpt_partition_table_hdr {
    uint64_t identifier;
    uint32_t version;
    uint32_t hdr_size;
    uint32_t checksum;
    uint32_t reserved0;
    uint64_t hdr_lba;
    uint64_t alt_hdr_lba;
    uint64_t first_block;
    uint64_t last_block;
    uint64_t guid[2];
    uint64_t partition_array_lba;
    uint32_t partition_ent_cnt;
    uint32_t partition_ent_size;
    uint32_t crc32_partition_array;
};

struct [[gnu::packed]] gpt_partition_entry {
    uint64_t partition_type_guid[2];
    uint64_t partition_guid[2];
    uint64_t starting_lba;
    uint64_t last_lba;
    uint64_t flags;
    uint64_t name[9];
};

void scan_partitions(msd *device);
void init();

inline vfs::cluster root_cluster;
inline lib::vector<node> node_list;

}

#endif
