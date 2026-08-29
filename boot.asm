; =============================================================================
; boot.asm - Minimal Bootstub for x86_64 C Kernel
; =============================================================================

global _start
extern kmain
extern data_end
extern bss_end

; ---------- Multiboot1 Constants ----------
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_ALIGN     equ (1 << 0)
MULTIBOOT_MEMINFO   equ (1 << 1)
MULTIBOOT_AOUT      equ (1 << 16)
MULTIBOOT_FLAGS     equ (MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO | MULTIBOOT_AOUT)
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; ---------- Paging Constants ----------
PAGE_PRESENT        equ (1 << 0)
PAGE_WRITABLE       equ (1 << 1)
PAGE_HUGE           equ (1 << 7)

; ---------- Multiboot Header ----------
section .multiboot
align 4
mboot:
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM
    ; AOUT Kludge fields
    dd mboot            ; header_addr
    dd 0x100000         ; load_addr (start of file, which is linked at 1MB)
    dd 0                ; load_end_addr (0 means entire file)
    dd bss_end          ; bss_end_addr
    dd _start           ; entry_addr

; ---------- 32-bit Entry Point ----------
section .text
bits 32
_start:
    cli
    mov esp, stack_top

    call check_cpuid
    call check_long_mode
    call setup_paging

    ; Enable PAE
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax

    ; Load PML4
    mov eax, pml4_table
    mov cr3, eax

    ; Enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8)
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax

    lgdt [gdt64_pointer]
    jmp 0x08:long_mode_start

check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, (1 << 21)
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    hlt
    jmp $

check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, (1 << 29)
    jz .no_long_mode
    ret
.no_long_mode:
    hlt
    jmp $

setup_paging:
    mov edi, pml4_table
    xor eax, eax
    mov ecx, 4096 * 3 / 4
    rep stosd

    mov eax, pdpt_table
    or eax, PAGE_PRESENT | PAGE_WRITABLE
    mov [pml4_table], eax

    mov eax, pd_table
    or eax, PAGE_PRESENT | PAGE_WRITABLE
    mov [pdpt_table], eax

    mov ecx, 0
    mov edi, pd_table
.map_pd:
    mov eax, ecx
    shl eax, 21
    or eax, PAGE_PRESENT | PAGE_WRITABLE | PAGE_HUGE
    mov [edi], eax
    add edi, 8
    inc ecx
    cmp ecx, 512
    jne .map_pd
    ret

; ---------- 64-bit Long Mode ----------
bits 64
long_mode_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, stack_top

    ; Call our C kernel main function
    call kmain
    
.hang:
    cli
    hlt
    jmp .hang

; ---------- GDT ----------
align 16
gdt64_start:
    dq 0
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)
    dq (1<<44) | (1<<47) | (1<<41)
gdt64_end:

gdt64_pointer:
    dw gdt64_end - gdt64_start - 1
    dd gdt64_start

; ---------- BSS ----------
section .bss
align 4096
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_table:
    resb 4096
align 16
stack_bottom:
    resb 16384
stack_top:
