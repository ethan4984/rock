#include <elf.h>

static int elf64_validate(int fd, elf_hdr_t *hdr) {
    read(fd, hdr, sizeof(hdr));
    
    if(*(uint32_t*)hdr != ELF_SIGNATURE)
        return -1;

    if( hdr->ident[EI_OSABI] != ABI_SYSTEM_V ||
        hdr->ident[EI_DATA] != LITTLE_ENDIAN ||
        hdr->ident[EI_CLASS] != ELF64 ||
        hdr->machine != MACH_X86_64) return -1;

    return 0;
}

int elf64_load(int fd, void *loc) {
    elf_hdr_t hdr;
    if(elf64_validate(fd, &hdr) == -1)
        return -1;

    elf64_phdr_t *phdr = kmalloc(sizeof(elf64_phdr_t) * hdr.ph_num);

    lseek(fd, hdr.phoff, SEEK_SET);
    read(fd, phdr, sizeof(elf64_phdr_t) * hdr.ph_num);

    for(size_t i = 0; i < hdr.ph_num; i++) {

    }

    return 0;
}
