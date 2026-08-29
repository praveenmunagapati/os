; Minimal multiboot test - just print OK and halt
MULTIBOOT_MAGIC equ 0x1BADB002
MULTIBOOT_FLAGS equ 0x00000003
MULTIBOOT_CHECK equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Multiboot header MUST be in .text for simple linking
section .text
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECK

global _start
_start:
    mov dword [0xB8000], 0x2F4B2F4F ; 'OK' in green on black
    cli
    hlt
