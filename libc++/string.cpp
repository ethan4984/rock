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
