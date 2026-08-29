#include "idt.h"
#include "io.h"

#define IDT_ENTRIES 256
struct idt_entry_64 idt[IDT_ENTRIES];
struct idt_ptr_64 idt_ptr;

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

extern void load_idt(uint64_t ptr);

/* ISRs and IRQs defined in boot.asm */
extern void isr0();
extern void isr1();
extern void irq0(); /* Timer */
extern void irq1(); /* Keyboard */
extern void irq12(); /* Mouse */
extern void isr_stub(); /* Default handler */

void set_idt_gate(int n, uint64_t handler) {
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = 0x28; /* Limine Kernel Code Segment */
    idt[n].ist = 0;
    idt[n].type_attr = 0x8E; /* Interrupt Gate */
    idt[n].offset_mid = (handler >> 16) & 0xFFFF;
    idt[n].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[n].zero = 0;
}

static void pic_remap(void) {
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    
    outb(PIC1_DATA, 0x20); /* Master PIC vector offset: 32 */
    io_wait();
    outb(PIC2_DATA, 0x28); /* Slave PIC vector offset: 40 */
    io_wait();
    
    outb(PIC1_DATA, 4);    /* Tell Master there is a slave PIC at IRQ2 */
    io_wait();
    outb(PIC2_DATA, 2);    /* Tell Slave its cascade identity */
    io_wait();
    
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();
    
    /* Unmask IRQ0 (Timer), IRQ1 (Keyboard), and IRQ12 (Mouse). Mask everything else. */
    outb(PIC1_DATA, 0xFB); 
    outb(PIC2_DATA, 0xEF);
}

void init_idt(void) {
    idt_ptr.limit = sizeof(struct idt_entry_64) * IDT_ENTRIES - 1;
    idt_ptr.base = (uint64_t)&idt;
    
    /* Set all to default stub to prevent triple faults */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        set_idt_gate(i, (uint64_t)isr_stub);
    }
    
    pic_remap();
    
    /* Setup specific gates */
    set_idt_gate(0, (uint64_t)isr0); /* Divide by zero */
    set_idt_gate(1, (uint64_t)isr1); /* Debug */
    set_idt_gate(32, (uint64_t)irq0); /* IRQ0: Timer */
    set_idt_gate(33, (uint64_t)irq1); /* IRQ1: Keyboard */
    set_idt_gate(44, (uint64_t)irq12); /* IRQ12: Mouse */
    
    load_idt((uint64_t)&idt_ptr);
}
