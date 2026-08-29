; =============================================================================
; boot.asm - Complete x86_64 Hello World Kernel
; =============================================================================
; Flat binary format (nasm -f bin) for QEMU multiboot loading using AOUT kludge.
; =============================================================================

global _start
org 0x100000

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
VGA_BUFFER          equ 0xB8000
COM1_PORT           equ 0x3F8

; ---------- Multiboot1 Header ----------
align 4
mboot:
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM
    ; AOUT Kludge fields
    dd mboot            ; header_addr
    dd $$               ; load_addr (start of file, which is org 0x100000)
    dd bss_start        ; load_end_addr
    dd bss_end          ; bss_end_addr
    dd _start           ; entry_addr

; ---------- 32-bit Entry Point ----------
bits 32
_start:
    cli
    mov esp, stack_top

    ; Initialize serial port COM1 (0x3F8)
    mov dx, 0x3F9 ; IER
    mov al, 0x00
    out dx, al
    mov dx, 0x3FB ; LCR
    mov al, 0x80
    out dx, al
    mov dx, 0x3F8 ; DLL
    mov al, 0x03  ; 38400 baud
    out dx, al
    mov dx, 0x3F9 ; DLH
    mov al, 0x00
    out dx, al
    mov dx, 0x3FB ; LCR
    mov al, 0x03  ; 8-N-1
    out dx, al
    mov dx, 0x3FA ; FCR
    mov al, 0xC7
    out dx, al
    mov dx, 0x3FC ; MCR
    mov al, 0x0B
    out dx, al

    ; Print serial message
    mov esi, msg_32bit
    call serial_print_32

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

serial_print_32:
.loop:
    lodsb
    test al, al
    jz .done
    mov dx, 0x3F8
    out dx, al
    jmp .loop
.done:
    ret

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
    mov al, 'C'
    jmp error

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
    mov al, 'L'
    jmp error

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

error:
    mov dword [0xB8000], 0x4F524F45
    mov dword [0xB8004], 0x4F3A4F52
    mov dword [0xB8008], 0x4F204F20
    mov byte  [0xB800A], al
    hlt
    jmp $

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

    ; Print serial message from 64-bit mode
    mov rsi, msg_64bit
    call serial_print_64

    ; Clear screen
    mov rdi, VGA_BUFFER
    mov rcx, 80 * 25
    mov ax, 0x0720
    rep stosw

    ; Banner row 0
    mov rdi, VGA_BUFFER
    mov ah, 0x0B
    mov rcx, 80
    mov al, '='
.bar1:
    stosw
    loop .bar1

    ; Row 1
    mov rdi, VGA_BUFFER + (1 * 80 * 2)
    mov ah, 0x0F
    mov rcx, 14
    mov al, ' '
.spc1:
    stosw
    loop .spc1

    mov rsi, msg_welcome
    mov ah, 0x0F
    call print_str64

    mov rsi, msg_myos
    mov ah, 0x0A
    call print_str64

    mov rsi, msg_kernel
    mov ah, 0x0F
    call print_str64

    ; Row 2
    mov rdi, VGA_BUFFER + (2 * 80 * 2)
    mov ah, 0x0B
    mov rcx, 80
    mov al, '='
.bar2:
    stosw
    loop .bar2

    ; Row 4
    mov rdi, VGA_BUFFER + (4 * 80 * 2)
    mov rsi, msg_arrow
    mov ah, 0x0A
    call print_str64
    mov rsi, msg_hello
    mov ah, 0x0F
    call print_str64

    ; Info rows
    mov rdi, VGA_BUFFER + (6 * 80 * 2)
    mov rsi, msg_info1
    mov ah, 0x07
    call print_str64

    mov rdi, VGA_BUFFER + (7 * 80 * 2)
    mov rsi, msg_info2
    mov ah, 0x07
    call print_str64

    mov rdi, VGA_BUFFER + (8 * 80 * 2)
    mov rsi, msg_info3
    mov ah, 0x07
    call print_str64

    mov rdi, VGA_BUFFER + (10 * 80 * 2)
    mov rsi, msg_halt
    mov ah, 0x08
    call print_str64

.hang:
    cli
    hlt
    jmp .hang

print_str64:
    lodsb
    test al, al
    jz .done
    stosw
    jmp print_str64
.done:
    ret

serial_print_64:
.loop:
    lodsb
    test al, al
    jz .done2
    mov dx, 0x3F8
    out dx, al
    jmp .loop
.done2:
    ret

align 4
msg_32bit:   db "[KERNEL] Booted into 32-bit protected mode.", 13, 10, 0
msg_64bit:   db "[KERNEL] Transitioned to 64-bit long mode. Hello, World!", 13, 10, 0
msg_welcome: db "Welcome to ", 0
msg_myos:    db "MyOS", 0
msg_kernel:  db " x86_64 Kernel!", 0
msg_arrow:   db "  >> ", 0
msg_hello:   db "Hello, World from Long Mode (64-bit)!", 0
msg_info1:   db "  Kernel loaded via Multiboot1 flat binary (AOUT kludge)", 0
msg_info2:   db "  VGA text mode: 80x25", 0
msg_info3:   db "  Identity-mapped first 1GB of RAM", 0
msg_halt:    db "  System halted. Nothing more to do.", 0

align 16
gdt64_start:
    dq 0
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)
    dq (1<<44) | (1<<47) | (1<<41)
gdt64_end:

gdt64_pointer:
    dw gdt64_end - gdt64_start - 1
    dd gdt64_start

; ---------- BSS (must be at the end) ----------
align 4096
bss_start:
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
bss_end:
