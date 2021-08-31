#include <fs/dev.hpp>
#include <fs/ramfs.hpp>
#include <debug.hpp>
#include <fs/ext2/ext2.hpp>

namespace dev {

ssize_t msd::read(size_t off, size_t cnt, void *buf) {
    cache_block_size = 0x10000;

    size_t headway = 0;

    while(headway < cnt) {
        size_t cache_block_index = (off + headway) / cache_block_size;

        size_t size = cnt - headway;
        size_t offset = (off + headway) % cache_block_size;

        if(size > (cache_block_size - offset))
            size = cache_block_size - offset;

        void *cache = search_cache(cache_block_index);
        
        if(cache) { 
            memcpy8((uint8_t*)buf, (uint8_t*)cache + (offset % cache_block_size), size);
            headway += cache_block_size;
            continue;
        }

        cache_block *new_cache_block = new cache_block;

        new_cache_block->block = cache_block_index;
        new_cache_block->buffer = (void*)(pmm::alloc(div_roundup(cache_block_size, vmm::page_size)) + vmm::high_vma);

        raw_read(cache_block_index * cache_block_size, cache_block_size, new_cache_block->buffer);

        memcpy8((uint8_t*)buf, (uint8_t*)new_cache_block->buffer + (offset % cache_block_size), size);

        cache_list.push(new_cache_block);

        headway += cache_block_size;
    }

    return cnt;
}

void *msd::search_cache(size_t block) {
    for(size_t i = 0; i < cache_list.size(); i++) {
        if(cache_list[i]->block == block) {
            return cache_list[i]->buffer;
        }
    }

    return NULL; 
}

ssize_t msd::write(size_t off, size_t cnt, void *buf) {
    cache_block_size = 0x10000;

    size_t headway = 0;

    while(headway < cnt) {
        size_t cache_block_index = (off + headway) / cache_block_size;

        size_t size = cnt - headway;
        size_t offset = (off + headway) % cache_block_size;

        if(size > cache_block_size - offset)
            size = cache_block_size - offset;

        void *cache = search_cache(cache_block_index);
        
        if(cache) { 
            memcpy8((uint8_t*)cache + (offset % cache_block_size), (uint8_t*)buf, size);
            raw_write(cache_block_index * cache_block_size, cache_block_size, cache);
            headway += cache_block_size;
            continue;
        }

        cache_block *new_cache_block = new cache_block;

        new_cache_block->block = cache_block_index;
        new_cache_block->buffer = (void*)(pmm::alloc(div_roundup(cache_block_size, vmm::page_size)) + vmm::high_vma);

        raw_read(cache_block_index * cache_block_size, cache_block_size, new_cache_block->buffer);
        memcpy8((uint8_t*)new_cache_block->buffer + (offset % cache_block_size), (uint8_t*)buf, size);
        raw_write(cache_block_index * cache_block_size, cache_block_size, new_cache_block->buffer);

        cache_list.push(new_cache_block);

        headway += cache_block_size;
    }

    return cnt;
}

ssize_t node::read(size_t start, size_t cnt, void *ret) {
    if(!device)
        return -1;

    return device->read(start + partition_offset, cnt, ret);
}

ssize_t node::write(size_t start, size_t cnt, void *ret) {
    if(!device)
        return -1;

    return device->write(start + partition_offset, cnt, ret);
}

node::node(vfs::node *vfs_search_node) : vfs_node(NULL) {
    for(size_t i = 0; i < node_list.size(); i++) {
        if(vfs_search_node == node_list[i].vfs_node) {
            *this = node_list[i];
            return;
        }
    }
}

void scan_partitions(msd *device) {
    gpt_partition_table_hdr gpt_hdr;
    device->read(device->sector_size, sizeof(gpt_partition_table_hdr), &gpt_hdr);

    if(gpt_hdr.identifier == 0x4546492050415254) {
        size_t partition_array_lba = gpt_hdr.partition_array_lba;
        uint32_t entry_cnt = gpt_hdr.partition_ent_cnt;

        gpt_partition_entry *part_arr = new gpt_partition_entry[entry_cnt];

        device->read(partition_array_lba * device->sector_size, sizeof(gpt_partition_entry) * entry_cnt, part_arr);

        for(size_t i = 0; i < entry_cnt; i++) {
            if(part_arr[i].partition_type_guid[0] == 0 && part_arr[i].partition_type_guid[1] == 0) // unused entry
                continue;

            lib::string absolute_path = device->device_prefix + device->device_index + "-" + device->partition_cnt++;
            root_cluster.generate_node(absolute_path, NULL, 0);

            vfs::node *vfs_node = root_cluster.search_absolute(absolute_path);

            print("[DEVFS] Creating partition device {}\n", lib::string("/dev") + absolute_path);

            node_list.push(node(vfs_node, part_arr[i].starting_lba * device->sector_size, part_arr[i].starting_lba - part_arr[i].last_lba, device));
        }

        return;
    }

    uint16_t mbr_signature;
    device->read(510, 2, &mbr_signature);

    if(mbr_signature == 0xaa55) {
        mbr_partition partitions[4];
        device->read(0x1be, sizeof(partitions), &partitions);

        for(size_t i = 0; i < 4; i++) {
            if(partitions[i].partition_type == 0 || partitions[i].partition_type == 0xee)
                continue;

            lib::string absolute_path = device->device_prefix + device->device_index + "-" + device->partition_cnt++;
            root_cluster.generate_node(absolute_path, NULL, 0);

            vfs::node *vfs_node = root_cluster.search_absolute(absolute_path);

            print("[DEVFS] Creating partition device {}\n", lib::string("/dev/") + absolute_path);

            node_list.push(node(vfs_node, partitions[i].starting_lba * device->sector_size, partitions[i].sector_cnt, device));

            new ext2::fs(node_list.last());
        }

        return;
    }
}

void init() {
    root_cluster = vfs::cluster(new ramfs::fs);
}

}
