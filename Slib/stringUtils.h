#pragma once

#include <stdint.h>

char *grabWord(const char *line, int wordNumber);

char *getInput();

char *strcpy(char *dest, const char *src);

int strcmp(const char *a, const char *b);

int strncmp(const char *str1, const char *str2, uint64_t n);

uint64_t strlen(const char *str);

char *itob(uint64_t num, int base); 
