#include <kernel/fs/ext2/dir.h>
#include <lib/stringUtils.h>
#include <lib/output.h>

namespace ext2 {

void dir::getDirEntry(inode_t inode, const char *path, int part) {
    char *buffer = new char[0x400];
    inode.read(0, 0x400, buffer, part);

    if(*path == '/')
        ++path;

    char **paths = new char*[256];

    uint64_t cnt = splitString(paths, path, "/");

    for(uint64_t j = 0; j < cnt; j++) {
        for(uint32_t i = 0; i < inode.inodeStruct.size32l; i++) {     
            dirEntry_t *dir = (dirEntry_t*)((uint64_t)buffer + i);

            if(strncmp(dir->name, paths[j], strlen(paths[j]) - 1) == 0 && j == cnt - 1) {
                dirEntry = *dir;
                goto end;
            }

            if(strncmp(dir->name, paths[j], strlen(paths[j]) - 1) == 0) {
                inode = getInode(dir->inode, part);
                if(!(inode.inodeStruct.permissions & 0x4000)) {
                    kprintDS("[KDEBUG]", "%s is not a directory", paths[j]); 
                }
                inode.read(0, 0x400, buffer, part);
                continue;
            }

            if(dir->sizeofEntry != 0)
                i += dir->sizeofEntry - 1;
        }
    }
    
    kprintDS("[KDEBUG]", "%s not found", path);

end: // todo: get smart pointers setup so we dont have to deal with this mess
    for(uint64_t i = 0; i < cnt; i++)
        delete paths[i];
    delete paths;
    delete buffer;
}

dir::dir(inode_t inode, const char *path, int part) {
    getDirEntry(inode, path, part);
}

dir::dir(const char *path, int part) {
    getDirEntry(inode_t::rootInode, path, part);
}

void dir::getDir(inode_t inode, directory_t *ret, int part) {
    dirEntry_t *dir = new dirEntry_t;

    char *buffer = new char[0x400];
    inode.read(0, 0x400, buffer, part);

    char **names = new char*[256];

    dirEntry_t *dirBuffer = new dirEntry_t[10];

    uint64_t cnt = 0;

    for(uint32_t i = 0; i < inode.inodeStruct.size32l;) {
        dir = (dirEntry_t*)((uint64_t)buffer + i);

        dirBuffer[cnt] = *dir;

        names[cnt] = new char[dir->nameLength];
        strncpy(names[cnt], dir->name, dir->nameLength);

        names[cnt][dir->nameLength] = '\0';

        i += dir->sizeofEntry;
        cnt++;
    }

    delete buffer;

    *ret = (directory_t) { dirBuffer, names, cnt }; // its up to the caller to free dirBuffer/names when theyre done with it
}

}
