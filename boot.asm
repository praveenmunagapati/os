bits 64

extern kmain
global _start

section .text

_start:
    ; Ensure interrupts are disabled
    cli
    
    ; Call our C kernel
    call kmain

.hang:
    hlt
    jmp .hang

; ---------- IDT Assembly Wrappers ----------
global load_idt
global isr0
global isr1
global irq0
global irq1
global isr_stub
extern keyboard_handler

load_idt:
    lidt [rdi]
    ret

; Generic ISR for unhandled exceptions
isr_stub:
    cli
    hlt
    jmp isr_stub

; ISR0: Divide by zero
isr0:
    cli
    push 0
    push 0
    ; (Would call a C handler here)
    add rsp, 16
    sti
    iretq

; ISR1: Debug
isr1:
    cli
    push 0
    push 1
    add rsp, 16
    sti
    iretq

; IRQ0: Timer
irq0:
    cli
    push rax
    ; Send EOI to master PIC
    mov al, 0x20
    out 0x20, al
    pop rax
    sti
    iretq

; IRQ1: Keyboard
irq1:
    cli
    push rax
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11
    
    call keyboard_handler
    
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rax
    sti
    iretq
