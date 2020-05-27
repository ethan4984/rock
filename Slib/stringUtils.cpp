#include <stringUtils.h>
#include <stddef.h>

char *itob(uint64_t num, int base)
{
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

uint64_t strlen(const char *str)
{
    uint64_t len = 0;
    while (str[len])
        len++;
    return len;
}


int strcmp(const char *a, const char *b)
{
    while(*a && *a == *b) {
        a++;
        b++;
    }
    return (int)(uint8_t)(*a) - (int)(uint8_t)(*b);
}

int strncmp(const char* str1, const char* str2, uint64_t n) {
    for(uint64_t i = 0; i < n; i++) {
        if (str1[i] != str2[i]) 
            return 1;
    }
	return 0;
}

char *strcpy(char *dest, const char *src)
{
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

