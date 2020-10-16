#include <kernel/drivers/ahci.h>
#include <kernel/fs/ext2/ext2.h>
#include <kernel/mm/kHeap.h>
#include <kernel/fs/vfs.h>
#include <lib/output.h>
#include <stddef.h>

namespace vfs {

static fd *fds = NULL;
static size_t fdCnt = 10;

static int deleteFD(int index);
static int allocFD(fd newFD);
static void addPartition(partition_t newPart);
    
void readPartitions() {
    if(partitions == NULL) {
        partitions = new partition_t[10];
    }

    if(fds == NULL) { 
        fds = new fd[10]; 
        for(int i = 0; i < 10; i++) {
            fds[i].fdID = -1;
        }
    }
 
    void *sector0 = new uint8_t[0x200];
    ahci::sataRW(&ahci::drives[0], 0, 1, sector0, 0);

    mbrPartitionEntry *mbrEntries = (mbrPartitionEntry*)((uint64_t)sector0 + 0x1BE);

    addPartition((partition_t){ EXT2, *mbrEntries });
}

size_t read(uint32_t fd, char *buf, size_t cnt) {
    if(fds[fd].fdID == -1) {
        kprintDS("[KDEBUG]", "trying to read from a non-existent fd");
        return 0;
    }

    size_t readCnt = fds[fd].read(fds[fd].path, fds[fd].index, cnt, buf, fds[fd].partition);
    fds[fd].index += readCnt;
    return readCnt;
}

size_t write(uint32_t fd, char *buf, size_t cnt) {
    if(fds[fd].fdID == -1) {
        kprintDS("[KDEBUG]", "trying to read from a non-existent fd");
        return 0;
    }

    size_t readCnt = fds[fd].write(fds[fd].path, fds[fd].index, cnt, buf, fds[fd].partition);
    fds[fd].index += readCnt;
    return readCnt;
}

int open(const char *path, int flags, int mode) {
    fd newFD = { };

    newFD.path = (char*)path; 
    newFD.flags = flags;
    newFD.read = ext2::read;

    return allocFD(newFD);
}

/*int close(int fd) {
     
}*/

static int allocFD(fd newFD) {
    int index = -1;
    for(size_t i = 0; i < fdCnt; i++) { 
        if(fds[i].fdID == -1) {
            index = i;
            break;
        }
    }

    if(index == -1) {
        fds = (fd*)kheap.realloc(fds, 10);
        index = fdCnt;
    }
    
    fds[index] = newFD;
    fds[index].fdID = index;
    return index;
}

static int deleteFD(int index) {
    fdCnt--;
    fds[index].fdID = -1;
    return 1;
}

static void addPartition(partition_t newPart) {
    if(partitionCnt + 1 % 10 == 0) { 
        partitions = (partition_t*)kheap.realloc(partitions, 10);
    }

    partitions[partitionCnt++] = newPart;
}

}
