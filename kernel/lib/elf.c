#include <elf.h>
#include <mm/mmap.h>

static int elf64_validate(int fd, struct elf_hdr *hdr) {
    read(fd, hdr, sizeof(struct elf_hdr));

    if(*(uint32_t*)hdr != ELF_SIGNATURE)
        return -1;

    if(hdr->ident[EI_OSABI] != ABI_SYSTEM_V && hdr->ident[EI_OSABI] != ABI_LINUX) return -1;
    if(hdr->ident[EI_DATA] != LITTLE_ENDIAN) return -1;
    if(hdr->ident[EI_CLASS] != ELF64) return -1;
    if(hdr->machine != MACH_X86_64 && hdr->machine != 0) return -1;

    return 0;
}

int elf64_load(struct page_map *page_map, struct aux *aux, int fd, uint64_t base, char **ld_path) {
    struct elf_hdr hdr;
    if(elf64_validate(fd, &hdr) == -1)
        return -1;

    struct elf64_phdr *phdr = kmalloc(sizeof(struct elf64_phdr) * hdr.ph_num);

    lseek(fd, hdr.phoff, SEEK_SET);
    read(fd, phdr, sizeof(struct elf64_phdr) * hdr.ph_num);

    aux->at_phdr = 0;
    aux->at_phent = sizeof(struct elf64_phdr);
    aux->at_phnum = hdr.ph_num;

    for(size_t i = 0; i < hdr.ph_num; i++) {
        if(phdr[i].p_type == PT_INTERP) {
            if(ld_path == NULL)
                continue;

            *ld_path = kcalloc(phdr[i].p_filesz + 1);

            lseek(fd, phdr[i].p_offset, SEEK_SET);
            read(fd, *ld_path, phdr[i].p_filesz);

            continue;
        } else if(phdr[i].p_type == PT_PHDR) {
            aux->at_phdr = base + phdr[i].p_vaddr;
            continue;
        } else if(phdr[i].p_type != PT_LOAD)
            continue;

        size_t misalignment = phdr[i].p_vaddr & (PAGE_SIZE - 1);
        size_t page_cnt = DIV_ROUNDUP(misalignment + phdr[i].p_memsz, PAGE_SIZE);

        mmap(page_map, (void*)phdr[i].p_vaddr + base, page_cnt * PAGE_SIZE, PROT_WRITE | PROT_READ | (1 << 2), MAP_ANONYMOUS | MAP_FIXED, 0, 0);

        lseek(fd, phdr[i].p_offset, SEEK_SET);
        read(fd, (void*)(phdr[i].p_vaddr + base), phdr[i].p_filesz);
    }

    aux->at_entry = base + hdr.entry;
    
    return 0;
}
