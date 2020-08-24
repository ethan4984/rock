#pragma once

#include <kernel/fs/ext2/ext2types.h>

namespace kernel {

class inode_t {
public:
    void getInode(inodeDescriptor_t *ret, uint64_t inode);

    void printInode(inodeDescriptor_t inodeDesc);
};

}
