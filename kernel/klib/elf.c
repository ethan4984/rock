#include <elf.h>
#include <mm/vmm.h>
#include <output.h>

static int elf64_validate(int fd, elf_hdr_t *hdr) {
    read(fd, hdr, sizeof(elf_hdr_t));

    if(*(uint32_t*)hdr != ELF_SIGNATURE)
        return -1;

    if( hdr->ident[EI_OSABI] != ABI_SYSTEM_V ||
        hdr->ident[EI_DATA] != LITTLE_ENDIAN ||
        hdr->ident[EI_CLASS] != ELF64 ||
        (hdr->machine != MACH_X86_64 &&
        hdr->machine != 0)) return -1;

    return 0;
}

int elf64_load(pagestruct_t *pagestruct, int fd) {
    elf_hdr_t hdr;
    if(elf64_validate(fd, &hdr) == -1)
        return -1;

    elf64_phdr_t *phdr = kmalloc(sizeof(elf64_phdr_t) * hdr.ph_num);

    lseek(fd, hdr.phoff, SEEK_SET);
    read(fd, phdr, sizeof(elf64_phdr_t) * hdr.ph_num);

    for(size_t i = 0; i < hdr.ph_num; i++) {
        if(phdr[i].p_type != PT_LOAD) 
            continue;

        size_t misalignment = phdr[i].p_vaddr & (PAGE_SIZE -1);
        size_t page_cnt = ROUNDUP(misalignment + phdr[i].p_memsz, PAGE_SIZE);

        map_range(pagestruct, phdr[i].p_vaddr, page_cnt, 0x3 | (1 << 2));

        lseek(fd, phdr[i].p_offset, SEEK_SET);
        read(fd, (void*)(phdr[i].p_vaddr + misalignment), phdr[i].p_filesz);
    }

    return 0;
}
