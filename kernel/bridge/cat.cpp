#include <kernel/bridge/commandHub.h>
#include <kernel/bridge/kterm.h>
#include <kernel/fs/ext2/ext2.h>
#include <kernel/bridge/cat.h>
#include <lib/stringUtils.h>
#include <lib/output.h>

namespace kernel { 

void cdHandler(char *str, char *path) {
    uint32_t cnt = strlen(str);
    for(int i = 0; i < strlen(str); i++) {
        if(str[i] == ' ') {
            cnt = i + 1;
        }
    }

    if(cnt == strlen(str)) {
        strcpy(path, "/");
        return;
    }

    char newPath[50] = { 0 };
    strncpy(newPath, str + cnt, strlen(str) - cnt);

    if(strcmp(newPath, "..") == 0) {
        int i;
        for(i = 0; i < strlen(str); i++) {
            if(str[i] == '/')
                break;
        }
    }

    if(strcmp(newPath, ".") == 0) {
        return;
    }

    strcpy(path, newPath);
} 

}
