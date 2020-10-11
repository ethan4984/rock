#pragma once

#include <kernel/fs/ext2/inode.h>

#include <stddef.h>
#include <stdint.h>

namespace ext2 {

struct [[gnu::packed]] BGD_t {
    uint32_t blockAddressBitmap;
    uint32_t blockAddressInodeBitmap;
    uint32_t startingBlock;

    uint16_t unallocatedBlocks;
    uint16_t unallocatedInodes;
    uint16_t directoryCnt;

    uint16_t reserved[7];
};

struct [[gnu::packed]] superBlockData_t {
    uint32_t inodeCount;
    uint32_t blockCount;
    uint32_t reservedBlocksCount;
    uint32_t freeBlocksCount;
    uint32_t freeInodesCount;
    uint32_t sbBlock;
    uint32_t blockSize;
    uint32_t fragSize;
    uint32_t blocksPerGroup;
    uint32_t fragsPerGroup;
    uint32_t inodesPerGroup;
    uint32_t lastMount;
    uint32_t lastWrite;
    uint16_t mountCount;
    uint16_t maxMount;
    uint16_t magicNum;
    uint16_t state;
    uint16_t errors;
    uint16_t minorVersion; 
    uint32_t lastCheck;
    uint32_t checkInterval;
    uint32_t osID;
    uint32_t majorVersion;
    uint16_t userID;
    uint16_t groupID;

    /* Extended Superblock */
    uint32_t firstInode;
    uint16_t inodeSize;
    uint16_t spBlockGroup;
    uint32_t featureCompat;
    uint32_t featureIncompat;
    uint32_t featureRCompat;
    uint64_t uuid[2];
    uint64_t volumeName[2];
    uint64_t lastMounted[8];
};

}
