#include <fs/ext2/dir.h>
#include <memutils.h>
#include <output.h>
#include <bitmap.h>

void ext2_delete_dir_entry(partition_t *part, ext2_inode_t parent, char *name) { 
    void *buffer = kmalloc(parent.size32l);
    ext2_inode_read(part, parent, 0, parent.size32l, buffer); 

    for(uint32_t i = 0; i < parent.size32l; i++) {
        ext2_dir_entry_t *dir = (ext2_dir_entry_t*)((uint64_t)buffer + i);

        if((dir->inode != 0) && (strncmp(dir->name, name, strlen(name)) == 0)) {
            dir->inode = 0;
            ext2_inode_write(part, parent, 0, parent.size32l, buffer);
            return; 
        }

        
        uint32_t expected_size = ALIGN_UP(sizeof(ext2_dir_entry_t) + dir->name_length, 4);
        if(dir->entry_size != expected_size) {
            return;
        }

        i += dir->entry_size - 1;
    }
}

int ext2_create_dir_entry(partition_t *part, ext2_inode_t parent, uint32_t inode, char *name, uint8_t type) {
    void *buffer = kmalloc(parent.size32l);
    ext2_inode_read(part, parent, 0, parent.size32l, buffer);

    int found = 0;

    for(uint32_t i = 0; i < parent.size32l; i++) {
        ext2_dir_entry_t *dir = (ext2_dir_entry_t*)((uint64_t)buffer + i);

        if(found) {
            dir->name_length = strlen(name);
            dir->type = type;
            dir->inode = inode;
            dir->entry_size = part->ext2_fs->block_size - i;
        
            memcpy8((uint8_t*)dir->name, (uint8_t*)name, strlen(name));

            ext2_inode_write(part, parent, 0, parent.size32l, buffer);
            kfree(buffer);
            return 1; 
        } 

        if(strncmp(dir->name, name, strlen(name)) == 0) {
            kprintf("[FS]", "%s already exists", name);
            kfree(buffer);
            return 0;
        }

        uint32_t expected_size = ALIGN_UP(sizeof(ext2_dir_entry_t) + dir->name_length, 4);
        if(expected_size != dir->entry_size) {
            i += expected_size - 1;
            dir->entry_size = expected_size;
            found = 1;
            continue;
        }
        
        i += dir->entry_size - 1;
    }
    kfree(buffer);
    return 1;
}

int ext2_read_dir_entry(partition_t *part, ext2_inode_t inode, ext2_dir_entry_t *ret, char *path) { // TODO make thread save
    if(*path == '/')
        ++path;

    void *buffer = kcalloc(inode.size32l);
    ext2_inode_read(part, inode, 0, inode.size32l, buffer);

    int cnt = 0, parse_cnt = character_cnt(path, '/');

    char *sub_path = strtok(path, "/");
    while(sub_path != NULL) {
        for(uint32_t i = 0; i < inode.size32l; i++) {
            ext2_dir_entry_t *dir = (ext2_dir_entry_t*)((uint64_t)buffer + i);

            if((strncmp(dir->name, sub_path, strlen(sub_path)) == 0) && (cnt == parse_cnt)) {
                if(dir->inode == 0) {
                    kprintf("[KDEBUG]", "%s not found", path);
                    kfree(buffer);
                    return 0;
                }
                *ret = *dir;
                goto end;
            }

            if(strncmp(dir->name, sub_path, strlen(sub_path)) == 0) {
                inode = ext2_inode_read_entry(part, dir->inode);
                if(!(inode.permissions & 0x4000)) {
                    kprintf("[KDEBUG]", "%s is not a directory", sub_path);
                    kfree(buffer);
                    return 0;
                }
                ext2_inode_read(part, inode, 0, inode.size32l, buffer);
                break;
            }

            if(dir->entry_size != 0) {
                i += dir->entry_size - 1;
            }
        }
        sub_path = strtok(NULL, "/");
        cnt++;
    }

    kfree(buffer);
    kprintf("[KDEBUG]", "%s not found", path);
    return 0;
end:
    kfree(buffer);
    return 1;
}
