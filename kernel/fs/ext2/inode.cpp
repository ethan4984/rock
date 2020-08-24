#include <kernel/fs/ext2/superblock.h> 
#include <kernel/fs/ext2/ext2Main.h>
#include <kernel/fs/ext2/inode.h>
#include <kernel/drivers/ahci.h>
#include <lib/output.h>

namespace kernel {

void inode_t::getInode(inodeDescriptor_t *ret, uint64_t inode) {

}

void inode_t::printInode(inodeDescriptor_t inode) {
    cout + "[FS]" << "permissions " << inode.permissions << "\n";
    cout + "[FS]" << "userID " << inode.userID << "\n";
    cout + "[FS]" << "size32Low " << inode.size32Low << "\n";
    cout + "[FS]" << "accessTime " << inode.accessTime << "\n";
    cout + "[FS]" << "creationTime " << inode.creationTime << "\n";
    cout + "[FS]" << "modifcationTime " << inode.modificationTime << "\n";
    cout + "[FS]" << "deletionTime " << inode.deletionTime << "\n";
    cout + "[FS]" << "groupID " << inode.groupID << "\n";
    cout + "[FS]" << "hardLinks " << inode.hardLinks << "\n";
    cout + "[FS]" << "sectorCnt " << inode.sectorCnt << "\n";
    cout + "[FS]" << "flags " << inode.flags << "\n";
    cout + "[FS]" << "osID " << inode.osID << "\n";
    cout + "[FS]" << "eab " << inode.eab << "\n";
    cout + "[FS]" << "size32High " << inode.size32High << "\n";
    cout + "[FS]" << "fragmentAddress " << inode.fragmentAddress << "\n";
}

}
