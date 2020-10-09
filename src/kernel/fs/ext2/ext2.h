#pragma once

#include <kernel/fs/ext2/ext2types.h>

namespace ext2 {

struct directory_t {
    directoryEntry_t *dirEntries;
    char **names;
    uint64_t dirCnt;
};

void init(int partition);

inode_t getInode(uint64_t index, int partition);

void readInode(inode_t inode, uint64_t block, uint64_t cnt, void *buffer, int partition);

directoryEntry_t getDirEntry(inode_t inode, const char *path, int partition);

void read(const char *path, uint64_t start, uint64_t cnt, void *buffer, int partition);

void write(const char *path, uint64_t start, uint64_t cnt, void *buffer, int partition);

void getDir(inode_t *inode, directory_t *ret, int partition);

inline inode_t rootInode;

blockGroupDescriptor_t readBGD(uint64_t index, int partition);

}
