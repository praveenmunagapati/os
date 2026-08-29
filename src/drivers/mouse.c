#include "mouse.h"
#include "io.h"
#include "gui.h"

int mouse_x = 400;
int mouse_y = 300;

static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[3];

static inline void mouse_wait(uint8_t a_type) {
    uint32_t timeout = 100000;
    if (a_type == 0) {
        while (timeout--) {
            if ((inb(0x64) & 1) == 1) return;
        }
    } else {
        while (timeout--) {
            if ((inb(0x64) & 2) == 0) return;
        }
    }
}

static inline void mouse_write(uint8_t a_write) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, a_write);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_init(void) {
    uint8_t status;

    /* Enable auxiliary mouse device */
    mouse_wait(1);
    outb(0x64, 0xA8);
    
    /* Enable interrupts */
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);
    
    /* Tell the mouse to use default settings */
    mouse_write(0xF6);
    mouse_read(); /* Acknowledge */
    
    /* Enable Data Reporting */
    mouse_write(0xF4);
    mouse_read(); /* Acknowledge */
}

extern void gui_update_cursor(int new_x, int new_y);

void mouse_handler(void) {
    uint8_t status = inb(0x64);
    if (!(status & 0x20)) {
        /* This isn't a mouse interrupt */
        outb(0x20, 0x20); /* EOI */
        outb(0xA0, 0x20);
        return;
    }

    switch(mouse_cycle) {
        case 0:
            mouse_byte[0] = inb(0x60);
            if (mouse_byte[0] & 0x08) { /* Bit 3 must be 1 */
                mouse_cycle++;
            }
            break;
        case 1:
            mouse_byte[1] = inb(0x60);
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = inb(0x60);
            mouse_cycle = 0;

            int dx = mouse_byte[1];
            int dy = mouse_byte[2];

            if (mouse_byte[0] & 0x10) dx -= 256; /* X Sign bit */
            if (mouse_byte[0] & 0x20) dy -= 256; /* Y Sign bit */

            int new_x = mouse_x + dx;
            int new_y = mouse_y - dy; /* PS/2 y is bottom-up, framebuffer is top-down */

            /* Call GUI to clamp and visually update the cursor */
            gui_update_cursor(new_x, new_y);
            break;
    }
    
    outb(0x20, 0x20);
    outb(0xA0, 0x20); /* Slave EOI */
}
