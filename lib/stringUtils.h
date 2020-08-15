#pragma once

#include <stdint.h>

namespace kernel {

uint64_t strlen(const char *str);

int strcmp(const char *str0, const char *str1);

int strncmp(const char *str0, const char *str1, uint64_t n);

char *itob(uint64_t num, uint64_t base);

}
