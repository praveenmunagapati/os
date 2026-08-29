ENTRY(_start)
OUTPUT_FORMAT("elf64-x86-64")

SECTIONS
{
    . = 1M;

    /* The multiboot header must be near the beginning of the file */
    .multiboot ALIGN(4K) :
    {
        KEEP(*(.multiboot))
    }

    .text ALIGN(4K) :
    {
        *(.text)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata*)
    }

    .data ALIGN(4K) :
    {
        *(.data)
        data_end = .;
    }

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
        bss_end = .;
    }
}
