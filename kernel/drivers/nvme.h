#ifndef NVME_H_
#define NVME_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t cap;
    uint32_t vs;
    uint32_t initms;
    uint32_t initmc;
    uint32_t cc;
    uint32_t csts;
    uint32_t aqa;
    uint64_t asq;
    uint64_t acq;
} nvme_bar0_t;

void init_nvme();

#endif
