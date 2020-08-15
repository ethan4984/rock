#include <lib/stringUtils.h>

#include <stddef.h>

namespace kernel {

char *itob(uint64_t num, uint64_t base) {
    static char hold[] = "0123456789ABCDEF";
    static char buffer[50];
    char *str;

    str = &buffer[49];
    *str = '\0';

    do {
        *--str = hold[num%base];
        num /= base;
    } while(num != 0);

    return str;
}

uint64_t strlen(const char *str) {
    uint64_t len = 0;
    while (str[len])
        len++;
    return len;
}


int strcmp(const char *str0, const char *str1) {
    while(*str0 && *str0 == *str1) {
        str0++;
        str1++;
    }
    return (int)(*str0) - (int)(*str1);
}

int strncmp(const char* str0, const char* str1, uint64_t n) {
    for(uint64_t i = 0; i < n; i++) {
        if (str0[i] != str1[i]) 
            return 1;
    }
    return 0;
}

char *strcpy(char *dest, const char *src) {
    if(dest == NULL)
        return NULL;

    char *new_dest = dest;

    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }

    *dest = '\0';

    return new_dest;
}

}
