#pragma once

#include <kernel/fs/ext2/ext2types.h>

struct directory_t {
    directoryEntry_t *dirEntries;
    char **names;
    uint64_t dirCnt;
};

class ext2_t {
public:
    void init();

    inode_t getInode(uint64_t index);

    void readInode(inode_t inode, uint64_t block, uint64_t cnt, void *buffer);

    directoryEntry_t getDirEntry(inode_t inode, const char *path);

    void read(const char *path, uint64_t start, uint64_t cnt, void *buffer);

    void write(const char *path, uint64_t start, uint64_t cnt, void *buffer);

    void getDir(inode_t *inode, directory_t *ret);

    inode_t rootInode;
private:
    blockGroupDescriptor_t readBGD(uint64_t index);
};

inline ext2_t ext2;
