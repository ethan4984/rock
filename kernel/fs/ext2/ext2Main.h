#pragma once

#include <kernel/fs/ext2/inode.h>

namespace kernel {

class ext2_t {
public:
    void init();

    inode_t inode;
};

inline ext2_t ext2;

}
