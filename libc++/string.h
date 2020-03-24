#pragma once

#include <stdint.h>
#include <stddef.h>
#include <alloc.h>

#include <shitio.h>

using namespace standardout;

size_t strlen(const char *str);

int strcmp(const char *a, const char *b);

char *strcpy(char *dest, const char *src);

class string 
{
    public:
        string(const char *bruh);
        
        string();
        
        char *substr(int start, int how_many_chars);
        
        int size();
        
        char operator[](int index)
        {
            return cstring[index];
        }
        
        void operator+(const char *added_part)
        {
            size_t size = strlen(cstring);
            for(size_t i = 0; i < strlen(added_part); i++)
                cstring[i + size] = added_part[i];
        }
    private:
        char *cstring;
};
