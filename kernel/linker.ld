OUTPUT_FORMAT(elf64-x86-64)
ENTRY(main)

kernel_VMA_start = 0xffffffff80000000;

SECTIONS {
    . = kernel_VMA_start + 0x200000;

    kernel_start = ALIGN(4K);

    .stivalehdr : ALIGN(4K) {
        KEEP(*(.stivalehdr))
    }

    .text : ALIGN(4K) {
        *(.text*)
    }

    .rodata : ALIGN(4K) {
        *(.rodata*)
    }

    .data : ALIGN(4K) {
        __ctors_start = .;
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        __ctors_end = .;
        *(.data*)
    }
 

    .bss : ALIGN(4K) {
        *(.bss*)
        *(COMMON)
    }

    kernel_end = ALIGN(4K);
}
