OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

kernel_vma_start = 0xFFFFFFFF80000000;

SECTIONS
{
    . = kernel_vma_start + 0x100000;

    __kernel_start = ALIGN(4K);

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
