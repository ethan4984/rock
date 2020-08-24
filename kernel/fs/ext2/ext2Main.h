#pragma once

#include <kernel/fs/ext2/inode.h>

namespace kernel {

class ext2_t {
public:
    void init();

    inodeDescriptor_t readInode(uint64_t index);

    inode_t inode;
private:
    blockGroupDescriptor_t readBGD(uint64_t index);
};

inline ext2_t ext2;

}
