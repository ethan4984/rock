#include <string.h>
#include <stdint.h>
#include <stddef.h>

string::string(const char *bruh)
{
    cstring = (char*)malloc(sizeof(char));
    for(size_t i = 0; i < strlen(bruh); i++)
        cstring[i] = bruh[i];
}

string::string()
{
    cstring = (char*)malloc(sizeof(char));
}

char *string::substr(int start, int how_many_chars)
{
    char *substring = (char*)malloc(sizeof(char));
    for(int i = 0; i < how_many_chars; i++)
        substring[i] = cstring[i + start];
    return substring;
}

int string::size()
{
    return strlen(cstring);
}

size_t strlen(const char *str)
{
    size_t len = 0;
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
    return (int)(unsigned char)(*a) - (int)(unsigned char)(*b);
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

void itoa(int n, char str[]) {
    int i, sign;

    if ((sign = n) < 0)
        n = -n;

    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0)
    str[i++] = '-';
    str[i] = '\0';

    reverse(str);
}

void reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int strncmp(const char* str1, const char* str2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (str1[i] != str2[i]) {
            return 1;
        }
    }

    return 0;
}
