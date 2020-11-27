#include <memutils.h>
#include <bitmap.h>

void memset8(uint8_t *src, uint8_t data, uint64_t count) {
    for(uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memset16(uint16_t *src, uint16_t data, uint64_t count) {
    for(uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memset32(uint32_t *src, uint32_t data, uint64_t count) {
    for (uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memset64(uint64_t *src, uint64_t data, uint64_t count) {
    for (uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memcpy8(uint8_t *dest, uint8_t *src, uint64_t count) {
    for(uint64_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

void memcpy16(uint16_t *dest, uint16_t *src, uint64_t count) {
    for(uint64_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

void memcpy32(uint32_t *dest, uint32_t *src, uint64_t count) {
    for(uint64_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

void memcpy64(uint64_t *dest, uint64_t *src, uint64_t count) {
    for(uint64_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
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
