#include <fs/ext2/ext2.hpp>
#include <debug.hpp>

namespace ext2 {

fs::fs(dev::node &devfs_node) : devfs_node(devfs_node) {
    devfs_node.read(devfs_node.device->sector_size * 2, sizeof(superblock), &superb);

    if(superb.signature != 0xef53) {
        return;
    }

    print("[EXT2] Filesystem Detected on Device {}\n", lib::string("/dev") + vfs::get_absolute_path(devfs_node.vfs_node));
    print("[EXT2] Inode cnt: {}\n", (unsigned)superb.inode_cnt);
    print("[EXT2] Inodes per group: {}\n", (unsigned)superb.inodes_per_group);
    print("[EXT2] Block cnt: {}\n", (unsigned)superb.block_cnt);
    print("[EXT2] Blocks per group: {}\n", (unsigned)superb.blocks_per_group);
    print("[EXT2] First non-reserved inode: {}\n", (unsigned)superb.first_inode);

    block_size = 1024 << superb.block_size;
    frag_size = 1024 << superb.frag_size;
    bgd_cnt = div_roundup(superb.block_cnt, superb.blocks_per_group);

    root_inode = inode(this, 2);

    devfs_node.filesystem = this;
    root_cluster = new vfs::cluster(this);
}

ssize_t fs::alloc_block() {
    for(size_t i = 0; i < bgd_cnt; i++) {
        bgd bgd_cur(this, i);
        ssize_t block = bgd_cur.alloc_block();
        if(block == -1) {
            continue;
        } else {
            return i * superb.blocks_per_group + block;
        }
    }
    return -1;
}

void fs::free_block(uint32_t block) {
    uint8_t *bitmap = new uint8_t[block_size];
    uint32_t bgd_index = block / superb.blocks_per_group;
    uint32_t bitmap_index = block - bgd_index * superb.blocks_per_group;

    bgd bgd_cur(this, bgd_index);

    devfs_node.read(bgd_cur.raw.block_addr_bitmap, block_size, reinterpret_cast<void*>(bitmap));
    if(!bm_test(bitmap, bitmap_index)) {
        delete bitmap;
        return;
    }

    bm_clear(bitmap, bitmap_index);
    devfs_node.write(bgd_cur.raw.block_addr_bitmap, block_size, reinterpret_cast<void*>(bitmap));

    bgd_cur.raw.unallocated_blocks++;
    bgd_cur.write_back();

    delete bitmap;
}

ssize_t fs::alloc_inode() {
    for(size_t i = 0; i < bgd_cnt; i++) {
        bgd bgd_cur(this, i);
        ssize_t inode_index = bgd_cur.alloc_inode();
        if(inode_index == -1) {
            continue;
        } else {
            return i * superb.inodes_per_group + inode_index;
        }
    }
    return -1;
}

void fs::free_inode(uint32_t inode_index) {
    uint8_t *bitmap = new uint8_t[block_size];
    uint32_t bgd_index = inode_index / superb.inodes_per_group;
    uint32_t bitmap_index = inode_index - bgd_index * superb.inodes_per_group;

    bgd bgd_cur(this, bgd_index);

    devfs_node.read(bgd_cur.raw.block_addr_inode, block_size, reinterpret_cast<void*>(bitmap));
    if(!bm_test(bitmap, bitmap_index)) {
        delete bitmap;
        return;
    }

    bm_clear(bitmap, bitmap_index);
    devfs_node.write(bgd_cur.raw.block_addr_inode, block_size, reinterpret_cast<void*>(bitmap));

    bgd_cur.raw.unallocated_inodes++;
    bgd_cur.write_back();

    delete bitmap;
}

int fs::raw_read(vfs::node *vfs_node, off_t off, off_t cnt, void *buf) {
    if(off > vfs_node->stat_cur->st_size) {
        set_errno(einval);
        return 0;
    }

    if((off + cnt) > vfs_node->stat_cur->st_size) {
        cnt = vfs_node->stat_cur->st_size - off;
    }

    lib::string relative_path = vfs::get_relative_path(vfs_node);

    dir file_dir_entry(&root_inode, relative_path, true);
    if(file_dir_entry.raw == NULL) 
        return -1;

    inode inode_cur(this, file_dir_entry.raw->inode);
    inode_cur.read(off, cnt, buf);

    return cnt;
}

int fs::raw_write(vfs::node *vfs_node, off_t off, off_t cnt, void *buf) {
    lib::string relative_path = vfs::get_relative_path(vfs_node);

    dir file_dir_entry(&root_inode, relative_path, true);
    if(file_dir_entry.raw == NULL) {
        return -1;
    }

    inode inode_cur(this, file_dir_entry.raw->inode);
    inode_cur.write(off, cnt, buf);

    if((off + cnt) > vfs_node->stat_cur->st_size) {
        inode_cur.raw.size32l = (vfs_node->stat_cur->st_size = off + cnt);
        inode_cur.write_back();
    }

    return cnt;
}

int fs::refresh_node(inode &inode_cur, lib::string mount_gate) {
    uint8_t *buffer = new uint8_t[inode_cur.raw.size32l];
    inode_cur.read(0, inode_cur.raw.size32l, buffer);

    for(uint32_t i = 0; i < inode_cur.raw.size32l;) { 
        raw_dir *dir_cur = reinterpret_cast<raw_dir*>(buffer + i);

        if(dir_cur->name_length == 0) {
            delete buffer;
            return 0;
        }

        lib::string name(reinterpret_cast<char*>(dir_cur->name), dir_cur->name_length);
        lib::string absolute_path = mount_gate + name;

        if(name == "." || name == "..") {
            root_cluster->generate_node(absolute_path, NULL, s_ifdir); 
            i += dir_cur->entry_size;
            continue;
        }
    
        inode new_inode(this, dir_cur->inode);

        if(new_inode.raw.permissions & 0x4000) { // is a directory
            lib::string directory_path = absolute_path + "/";
            root_cluster->generate_node(directory_path, NULL, s_ifdir); 
            refresh_node(new_inode, directory_path);
        } else {
            root_cluster->generate_node(absolute_path, NULL, s_ifreg); 
        }

        i += dir_cur->entry_size;
    }

    delete buffer;

    return 0;
}

int fs::unlink(vfs::node *vfs_node) {
    lib::string relative_path = vfs::get_relative_path(vfs_node);

    dir delete_dir(&root_inode, relative_path, false);
    if(delete_dir.exists == false) 
        return -1;

    return 0; 
}

int fs::refresh([[maybe_unused]] vfs::node *vfs_node) {
    return refresh_node(root_inode, "/");
}

int fs::raw_open(vfs::node *vfs_node, uint16_t flags) {
    inode inode_cur;

    if(flags & o_creat) {
        inode parent_inode;

        if(vfs_node->parent == vfs_node->parent_cluster->root_node) {
            parent_inode = root_inode;
        } else {
            dir dir_entry(&root_inode, vfs::get_relative_path(vfs_node->parent), true); 
            if(dir_entry.raw == NULL) {
                return -1;
            }

            parent_inode = inode(this, dir_entry.raw->inode);
        }

        inode_cur = inode(this, alloc_inode());

        if(inode_cur.set_block(0, alloc_block()) == -1)
            return -1;

        inode_cur.raw.sector_cnt = block_size / devfs_node.device->sector_size;
        inode_cur.write_back();

        dir create_dir_entry(&parent_inode, inode_cur.inode_index, 0, vfs_node->name.data());

        parent_inode.raw.hard_link_cnt++;
        parent_inode.raw.permissions = s_ifreg;

        parent_inode.write_back();
    } else {
        if(vfs_node->name == "/") {
            inode_cur = root_inode;
        } else {
            dir dir_entry(&root_inode, vfs::get_relative_path(vfs_node), true); 
            if(dir_entry.exists == false)
                return -1;
            inode_cur = inode(this, dir_entry.raw->inode);
        }
    }
    
    uint16_t file_type = (inode_cur.raw.permissions & 0x4000) ? s_ifdir : s_ifreg;

    vfs_node->stat_cur->st_mode |= file_type | flags;
    vfs_node->stat_cur->st_size = inode_cur.raw.size32l;
    vfs_node->stat_cur->st_nlink = inode_cur.raw.hard_link_cnt;
    vfs_node->stat_cur->st_ino = inode_cur.inode_index;
    vfs_node->stat_cur->st_blksize = 512;
    vfs_node->stat_cur->st_blocks = div_roundup(inode_cur.raw.size32l, 512);

    return 0;
}

}
