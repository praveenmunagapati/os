/* Linker Script for x86_64 kernel (32-bit ELF container) */

ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    . = 1M;

    .multiboot : ALIGN(4)
    {
        *(.multiboot)
    }

    .text : ALIGN(4K)
    {
        *(.text)
        *(.text.*)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data : ALIGN(4K)
    {
        *(.data)
        *(.data.*)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
}
