ENTRY(_start)
OUTPUT_FORMAT("elf64-x86-64")

SECTIONS
{
    /* Link in the Higher Half as required by Limine */
    . = 0xffffffff80200000;

    .text ALIGN(4K) :
    {
        *(.text)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata*)
        /* Limine Requests */
        *(.requests)
    }

    .data ALIGN(4K) :
    {
        *(.data)
    }

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }
}
