#include "io.h"
#include <stdint.h>

#define PIC1_COMMAND 0x20

extern void term_putchar(char c);

/* Very basic scancode to ASCII map for the US layout */
static const char kbd_us[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', /* <-- Tab */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, /* <-- control key */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* F1 - F10 keys */
    0,  /* Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0, /* All other keys are undefined */
};

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);
    
    /* If the top bit is set, a key was just released */
    if (scancode & 0x80) {
        /* You can handle shift/ctrl/alt release here */
    } else {
        /* A key was pressed */
        char c = kbd_us[scancode];
        if (c) {
            term_putchar(c);
        }
    }
    
    /* Send End Of Interrupt (EOI) to the master PIC */
    outb(PIC1_COMMAND, 0x20);
}
