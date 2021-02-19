#include <memutils.h>
#include <mm/slab.h>

void memset8(uint8_t *src, uint8_t data, size_t count) {
    for(size_t i = 0; i < count; i++)
        *src++ = data;
}

void memset16(uint16_t *src, uint16_t data, size_t count) {
    for(size_t i = 0; i < count; i++)
        *src++ = data;
}

void memset32(uint32_t *src, uint32_t data, size_t count) {
    for(size_t i = 0; i < count; i++)
        *src++ = data;
}

void memset64(uint64_t *src, uint64_t data, size_t count) {
    for(size_t i = 0; i < count; i++)
        *src++ = data;
}

void memcpy8(uint8_t *dest, uint8_t *src, size_t count) {
    for(size_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

void memcpy16(uint16_t *dest, uint16_t *src, size_t count) {
    for(size_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

void memcpy32(uint32_t *dest, uint32_t *src, size_t count) {
    for(size_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

void memcpy64(uint64_t *dest, uint64_t *src, size_t count) {
    for(size_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

size_t strlen(const char *str) {
    size_t len = 0;
    while(str[len])
        len++;
    return len;
}


int strcmp(const char *str0, const char *str1) {
    for(size_t i = 0;; i++) {
        if(str0[i] != str1[i]) 
            return str0[i] - str1[i];
        if(!str0[i])
            return 0;
    }
}

int strncmp(const char *str0, const char *str1, size_t n) {
    for(size_t i = 0; i < n; i++) {
        if(str0[i] != str1[i]) 
            return str0[i] - str1[i];
        if(!str0[i])
            return 0;
    }
    return 0;
}

char *strcpy(char *dest, const char *src) {
    size_t i;

    for(i = 0; src[i]; i++)
        dest[i] = src[i];

    dest[i] = 0;

    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;

    for(i = 0; i < n && src[i]; i++)
        dest[i] = src[i];
    for(; i < n; i++)
        dest[i] = 0;

    return dest;
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

int atoi(const char *str) {
    int ret = 0;
    for(size_t i = 0; i < strlen(str); i++)
        ret = ret * 10 + str[i] - '0';
    return ret;
}

char *itob(uint64_t num, uint64_t base) {
    static char digits[] = "0123456789ABCDEF";
    static char buffer[50];
    char *str;

    str = &buffer[49];
    *str = '\0';

    do {
        *--str = digits[num % base];
        num /= base;
    } while(num != 0);

    return str;
}

void bm_alloc_region(uint8_t *bitmap, size_t start, size_t limit) {
    for(size_t i = start; i < start + limit; i++) {
        BM_SET(bitmap, i);
    }
}

void bm_free_region(uint8_t *bitmap, size_t start, size_t limit) {
    for(size_t i = start; i < start + limit; i++) {
        BM_CLEAR(bitmap, i);
    }    
}

int bm_first_free(uint8_t *bitmap, size_t size) {
    for(size_t i = 0; i < size; i++) {
        if(!BM_TEST(bitmap, i))
            return i;
    }
    return -1;
}

char *str_congregate(char *str1, char *str2) {
    char *new_str = kcalloc(strlen(str1) + strlen(str2) + 1);
    strncpy(new_str, str1, strlen(str1));
    strncpy(new_str + strlen(str1), str2, strlen(str2));
    return new_str;
}

size_t last_char(char *str, char c) {
    size_t ret = 0;
    for(size_t i = 0; i < strlen(str); i++)
        if(str[i] == c)
            ret = i + 1;
    return ret;
}
