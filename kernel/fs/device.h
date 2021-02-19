#ifndef DEVICE_H_
#define DEVICE_H_

#include <vec.h>
#include <fs/ext2/types.h>
#include <types.h>

#define SECTOR_SIZE 0x200
#define DEVFS_PERMS 0x555
#define DEV_NAME_MAX 32

struct mbr_partition {
    uint8_t drive_status;
    uint8_t starting_chs[3];
    uint8_t partition_type;
    uint8_t ending_chs[3];
    uint32_t starting_lba;
    uint32_t sector_cnt;
} __attribute__((packed));

struct gpt_partition_table_hdr {
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
    uint64_t guid_lba;
    uint32_t partition_ent_cnt;
    uint32_t partition_ent_size;
    uint32_t crc32_partition_array;
} __attribute__((packed));

struct filesystem;

struct msd {
    struct filesystem *fs;
    size_t device_index;
    size_t partition_cnt;
    int (*read)(int, uint64_t, uint64_t, void*);
    int (*write)(int, uint64_t, uint64_t, void*);
};

struct devfs_node {
    size_t devfs_id;
    char *dev_name;
    size_t partition_offset;
    struct msd *device;
};

struct vfs_node;

struct filesystem {
    struct devfs_node *devfs_node;

    struct ext2_fs *ext2_fs;
    char *mount_gate;

    int (*read)(struct vfs_node*, off_t, off_t, void*);
    int (*write)(struct vfs_node*, off_t, off_t, void*);
    int (*mkdir)(struct vfs_node*, uint16_t);
    int (*refresh)(struct vfs_node*);
    int (*open)(struct vfs_node*, int flags);
    int (*unlink)(struct vfs_node*);
};

void msd_raw_read(struct devfs_node *dev, uint64_t start, uint64_t cnt, void *ret);
void msd_raw_write(struct devfs_node *dev, uint64_t start, uint64_t cnt, void *ret);
void scan_device_partitions(struct msd *device);
void devfs_init();
int devfs_add_device(struct devfs_node node);
struct devfs_node *path2devfs(char *path);

extern_vec(struct msd, msd_list);
extern_vec(struct filesystem, filesystem_list);

#endif
