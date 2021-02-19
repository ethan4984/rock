#include <fs/ext2/ext2.h>
#include <fs/device.h>
#include <fs/ramfs.h>
#include <debug.h>

global_vec(msd_list);
global_vec(filesystem_list);

static_vec(struct devfs_node, devfs_nodes);

static struct filesystem devfs = {  .mount_gate = "/dev",
                                    .read = ramfs_read,
                                    .write = ramfs_write,
                                    .open = ramfs_open,
                                    .unlink = ramfs_delete
                                 };

static size_t devfs_id_cnt = 0, msd_cnt = 0;

int devfs_add_device(struct devfs_node node) {
    struct vfs_node *devfs = vfs_absolute_path("/dev");
    if(devfs == NULL)
        return -1;

    node.devfs_id = devfs_id_cnt++;
    vec_push(struct devfs_node, devfs_nodes, node);

    struct vfs_node *new_dev = vfs_create_node(devfs, node.dev_name);
    return new_dev->fs->open(new_dev, DEVFS_PERMS);
}

struct devfs_node *path2devfs(char *path) {
    struct vfs_node *device = vfs_absolute_path(path);
    if(device == NULL)
        return NULL;

    for(size_t i = 0; i < devfs_nodes.element_cnt; i++) {
        struct devfs_node *node = vec_search(struct devfs_node, devfs_nodes, i);
        if(strcmp(node->dev_name, device->name) == 0)
            return node;
    }

    return NULL; 
}

static int device_check_fs(struct devfs_node *dev) {
    if(ext2_check_fs(dev) == 0) {
        dev->device->fs->devfs_node = dev;
        return 0;
    }
    return -1;
}

void scan_device_partitions(struct msd *device) {
    uint16_t mbr_signature;
    device->read(device->device_index, 510, 2, &mbr_signature);

    if(mbr_signature == 0xaa55) { 
        struct mbr_partition mbr_partitions[4];
        device->read(device->device_index, 0x1be, sizeof(mbr_partitions), &mbr_partitions);

        for(int i = 0; i < 4; i++) {
            if(mbr_partitions[i].partition_type == 0)
                continue;

            struct devfs_node devfs_node = {    .dev_name = kmalloc(DEV_NAME_MAX),
                                                .partition_offset = mbr_partitions[i].starting_lba * SECTOR_SIZE,
                                                .device = device
                                           };

            sprintf(devfs_node.dev_name, "SD%d-%d", 1, msd_cnt, device->partition_cnt++);
            devfs_add_device(devfs_node);
            
            char *dev_absolute_path = str_congregate("/dev/", devfs_node.dev_name);
            struct devfs_node *new_devfs_node = path2devfs(dev_absolute_path);
            kfree(dev_absolute_path); 

            if(device_check_fs(new_devfs_node) == -1) {
                kprintf("[FS]", "Device %s has no detected filesystem", devfs_node.dev_name);
            }
        }
        goto end;
    }

    struct gpt_partition_table_hdr gpt_hdr;
    device->read(device->device_index, SECTOR_SIZE, sizeof(struct gpt_partition_table_hdr), &gpt_hdr);

    if(gpt_hdr.identifier == 0x4546492050415254) {
        goto end;
    }

    kprintf("[FS]", "Warning: device with no partition table found");

    struct devfs_node device_node;

end:
    device_node.dev_name = kmalloc(DEV_NAME_MAX);
    device_node.device = device;

    sprintf(device_node.dev_name, "SD%d", 1, msd_cnt++);
    devfs_add_device(device_node);
}

void msd_raw_read(struct devfs_node *dev, uint64_t start, uint64_t cnt, void *ret) {
    dev->device->read(dev->device->device_index, start + dev->partition_offset, cnt, ret);
}

void msd_raw_write(struct devfs_node *dev, uint64_t start, uint64_t cnt, void *ret) {
    dev->device->write(dev->device->device_index, start + dev->partition_offset, cnt, ret); 
}

void devfs_init() {
    vfs_mkdir(&vfs_root_node, "dev");
    vfs_mount_fs(&devfs);
}
