#pragma once

#include <kernel/fs/ext2/ext2types.h>

namespace kernel {

class ext2_t {
public:
    void init();

    inode_t getInode(uint64_t index);

    void readInode(inode_t inode, uint64_t block, uint64_t cnt, void *buffer);

    directoryEntry_t grabDirectoryEntry(const char *path);

    inode_t rootInode;
private:
    blockGroupDescriptor_t readBGD(uint64_t index);
};

inline ext2_t ext2;

}
