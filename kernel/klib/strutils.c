#include <strutils.h>
#include <stdarg.h>
#include <bitmap.h>

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

int strncmp(const char *str0, const char *str1, uint64_t n) {
    for(uint64_t i = 0; i < n; i++) {
        if (str0[i] != str1[i]) 
            return 1;
    }
    return 0;
}

char *strcpy(char *dest, const char *src) {
    uint64_t i;

    for(i = 0; src[i]; i++)
        dest[i] = src[i];

    dest[i] = 0;

    return dest;
}

char *strncpy(char *dest, const char *src, uint64_t n) {
    uint64_t i;

    for(i = 0; i < n && src[i]; i++)
        dest[i] = src[i];
    for(; i < n; i++)
        dest[i] = 0;

    return dest;
}

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

int character_cnt(const char *str, char c) {
    int cnt = 0;
    for(uint64_t i = 0; i < strlen(str); i++) {
        if(str[i] == c)
            cnt++;
    }
    return cnt;
}

char *strchr(const char *s, int c) {
    size_t i = 0;
    while(s[i]) {
        if(s[i] == c)
            return (char*)(&s[i]);
        i++;
    }
    if(c == 0)
        return (char*)(&s[i]);
    return NULL;
}

char *strtok_r(char *__restrict s, const char *__restrict del, char **__restrict m) {
    char *tok;
    if(s) {
        tok = s;
    } else if(*m) {
        tok = *m;
    } else {
        return NULL;
    }

    while(*tok && strchr(del, *tok))
        tok++;

    char *p = tok;
    while(*p && !strchr(del, *p))
        p++;

    if(*p) {
        *p = 0;
        *m = p + 1;
    }else{
        *m = NULL;
    }
    if(p == tok)
        return NULL;
    return tok;
}

char *strtok(char *__restrict s, const char *__restrict delimiter) {
    static char *saved;
    return strtok_r(s, delimiter, &saved);
}

int atoi(char *str) {
    int ret = 0;
    for (uint64_t i = 0; i < strlen(str); i++)
        ret = ret * 10 + str[i] - '0';
    return ret;
}

char *str_congregate(char *str1, char *str2) {
    char *new_str = kcalloc(strlen(str1) + strlen(str2) + 1);
    strncpy(new_str, str1, strlen(str1));
    strncpy(new_str + strlen(str1), str2, strlen(str2) + 1);
    return new_str;
}

size_t last_char(char *str, char c) {
    size_t ret = 0;
    for(size_t i = 0; i < strlen(str); i++)
        if(str[i] == c)
            ret = i + 1;
    return ret;
} 
