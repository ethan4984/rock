#pragma once

#include <kernel/fs/ext2/inode.h>

namespace ext2 {
	
struct [[gnu::packed]] dirEntry_t {
    uint32_t inode;
    uint16_t sizeofEntry;
    uint8_t nameLength;
    uint8_t typeIndicator;
    char name[];
};

struct directory_t {
    dirEntry_t *dirEntries;
    char **names;
    uint64_t dirCnt;
};
	
struct dir : public inode_t {
    dir(inode_t inode, const char *path, int part);
    dir(const char *path, int part);

    void getDirEntry(inode_t indoe, const char *path, int part);
    dirEntry_t dirEntry;

    static void getDir(inode_t inode, directory_t *ret, int part);
};
	
}
