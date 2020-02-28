ENTRY(_start)

SECTIONS
{
    . = 0x00100000;

    .text ALIGN(0x1000) :
    {
        *(.text)
        *(.gnu.linkonce.t*)
    }

    .rodata ALIGN(0x1000) :
    {
        start_ctors = .;
        *(SORT(.ctors*))  /* Note the "SORT" */
        end_ctors = .;

        start_dtors = .;
        *(SORT(.dtors*))
        end_dtors = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)
    }

    .data ALIGN(0x1000) :
    {
        *(.data)
        *(.gnu.linkonce.d*)
    }

    .bss :
    {
        sbss = .;
        *(COMMON)
        *(.bss)
        *(.gnu.linkonce.b*)
        ebss = .;
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
    }
    kernel_end = .;
}
