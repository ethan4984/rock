#ifndef STRUTILS_H_
#define STRUTILS_H_

#include <memutils.h>

uint64_t strlen(const char *str);
int strcmp(const char *str0, const char *str1);
int strncmp(const char *str0, const char *str1, uint64_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, uint64_t n);
size_t last_char(char *str, char c);
int atoi(char *str);
char *itob(uint64_t num, uint64_t base);
char *strtok(char *str, const char *delim);
char *strtok_r(char *__restrict s, const char *__restrict del, char **__restrict m);
char *str_congregate(char *str1, char *str2);

#endif
