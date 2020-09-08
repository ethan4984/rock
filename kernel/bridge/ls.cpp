#include <kernel/fs/ext2/ext2.h>
#include <kernel/bridge/kterm.h>
#include <kernel/bridge/ls.h>
#include <lib/stringUtils.h>

namespace kernel {

void lsHandler(char *str, char *path) {
    directory_t dir;
    inode_t inode = ext2.rootInode;

    ext2.getDir(&inode, &dir);

    if(*path == '/')
        ++path;
    
    char **paths = new char*[50];

    uint64_t cnt = splitString(paths, path, "/");

    for(int i = 0; i < cnt; i++) {
        for(int j = 0; j < dir.dirCnt; j++) {
            if(strcmp(dir.names[j], paths[i]) == 0) {
                inode = ext2.getInode(dir.dirEntries[j].inode);
                if(!(inode.permissions & 0x4000)) {
                    kterm.print("%s is not a directory", paths[i]);
                    goto end; 
                }
                ext2.getDir(&inode, &dir);
            } else if(j == dir.dirCnt - 1) {
                kterm.print("\"/%s\" no such file or directory", path); 
                goto end;
            }
        }
    }

    for(int i = 0; i < dir.dirCnt; i++) {
        kterm.print("%s ", dir.names[i]);
    }

end:
    kterm.putchar('\n');
    for(int i = 0; i < cnt; i++) {
        delete paths[i];
    } 
    delete paths; 
}

}
