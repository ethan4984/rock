OUTPUT_FORMAT(elf64-x86-64)

ENTRY(kernelMain)

kernelVmaStart = 0xffffffff80000000;

SECTIONS {
    . = kernelVmaStart + 0x100000;

    kernelStart = ALIGN(4K);

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
        *(.data*)
    }

    .bss : ALIGN(4K) {
        *(.bss*)
        *(COMMON)
    }

    kernelEnd = ALIGN(4K);
}
