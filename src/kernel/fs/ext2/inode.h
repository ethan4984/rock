#pragma once

#include <stdint.h>
#include <stddef.h>

namespace ext2 {
	
struct [[gnu::packed]] inodeStruct_t {
    uint16_t permissions;
    uint16_t userID;

    uint32_t size32l;
    uint32_t accessTime;
    uint32_t creationTime;
    uint32_t modificationTime;
    uint32_t deletionTime;
    
    uint16_t groupID;
    uint16_t hardLinkCnt;

    uint32_t sectorCnt;
    uint32_t flags;
    uint32_t oss1;

    uint32_t blocks[15];

    uint32_t generationNumber;
    uint32_t eab;
    uint32_t size32h;

    uint32_t blockAddrFragment;

    /* ext2 linux */

    uint8_t fragNum;
    uint8_t fragSize;
    
    uint16_t reserved16;
    uint16_t userIDhigh;
    uint16_t groupIDhigh;

    uint32_t reserved32;
};

struct inode_t {
    inode_t(uint64_t index, int part);
    inode_t() = default;

    void read(uint64_t block, uint64_t cnt, void *buffer, int partition);
    inodeStruct_t inodeStruct;

    static inode_t rootInode;
    static inode_t getInode(uint64_t index, int partition);
};

}
