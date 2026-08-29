#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "serial.h"
#include "limine.h"
#include "limine.h"
#include "gui.h"
#include "mouse.h"

/* Request a framebuffer from Limine */
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

struct limine_framebuffer *fb = NULL;

/* Window Bounds */
#define WIN_X 50
#define WIN_Y 50
#define WIN_W 800
#define WIN_H 500
#define WIN_INNER_X (WIN_X + 4)
#define WIN_INNER_Y (WIN_Y + 28)
#define WIN_INNER_W (WIN_W - 8)
#define WIN_INNER_H (WIN_H - 32)

static size_t term_x = WIN_INNER_X;
static size_t term_y = WIN_INNER_Y;
static uint32_t term_color = 0xFFFFFFFF; // White

/* Scroll the terminal window up by 8 pixels */
static void scroll(void) {
    if (!fb) return;
    uint32_t *fb_ptr = fb->address;
    size_t pitch = fb->pitch / 4;
    
    /* Move pixels up by 8 rows inside the window */
    for (size_t y = WIN_INNER_Y + 8; y < WIN_INNER_Y + WIN_INNER_H; y++) {
        for (size_t x = WIN_INNER_X; x < WIN_INNER_X + WIN_INNER_W; x++) {
            fb_ptr[(y - 8) * pitch + x] = fb_ptr[y * pitch + x];
        }
    }
    /* Clear the last 8 rows */
    for (size_t y = WIN_INNER_Y + WIN_INNER_H - 8; y < WIN_INNER_Y + WIN_INNER_H; y++) {
        for (size_t x = WIN_INNER_X; x < WIN_INNER_X + WIN_INNER_W; x++) {
            fb_ptr[y * pitch + x] = 0xFF121212; // Window Background Color
        }
    }
    term_y -= 8;
}

void term_putchar(char c) {
    if (!fb) return;
    
    if (c == '\b') {
        if (term_x >= WIN_INNER_X + 8) {
            term_x -= 8;
        } else if (term_y >= WIN_INNER_Y + 8) {
            term_y -= 8;
            term_x = WIN_INNER_X + ((WIN_INNER_W / 8) * 8) - 8;
        }
        gui_draw_char(' ', term_x, term_y, 0, 0xFF121212);
        return;
    }
    
    if (c == '\n') {
        term_x = WIN_INNER_X;
        term_y += 8;
    } else {
        gui_draw_char(c, term_x, term_y, term_color, 0xFF121212);
        term_x += 8;
        if (term_x + 8 > WIN_INNER_X + WIN_INNER_W) {
            term_x = WIN_INNER_X;
            term_y += 8;
        }
    }
    
    if (term_y + 8 > WIN_INNER_Y + WIN_INNER_H) {
        scroll();
    }
}

void term_puts(const char *str) {
    while (*str) {
        term_putchar(*str++);
    }
}

void print_hex(uint32_t val) {
    char buf[9];
    buf[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = val & 0xF;
        if (nibble < 10) buf[i] = '0' + nibble;
        else buf[i] = 'A' + (nibble - 10);
        val >>= 4;
    }
    term_puts(buf);
}

extern void pci_scan(void);

#include "keyboard.h"
#include "pmm.h"

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

static void term_clear(void) {
    if (!fb) return;
    gui_draw_rect(WIN_INNER_X, WIN_INNER_Y, WIN_INNER_W, WIN_INNER_H, 0xFF121212);
    term_x = WIN_INNER_X;
    term_y = WIN_INNER_Y;
}

static void print_num(uint64_t val) {
    if (val == 0) {
        term_putchar('0');
        return;
    }
    char buf[32];
    int i = 0;
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (i > 0) {
        term_putchar(buf[--i]);
    }
}

void kmain(void) {
    init_serial();
    init_idt();
    
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        for (;;) { __asm__ volatile ("hlt"); }
    }
    
    fb = framebuffer_request.response->framebuffers[0];
    
    /* Draw the full GUI Desktop */
    gui_draw_desktop();
    
    /* Draw the Terminal Window */
    gui_draw_window("Terminal - root@MyOS", WIN_X, WIN_Y, WIN_W, WIN_H);
    term_clear();
    
    term_color = 0xFF00FF00;
    __asm__ volatile ("sti");

    /* Initialize Memory */
    pmm_init();
    
    /* Initialize Mouse */
    mouse_init();
    
    term_puts("==================================================================\n");
    term_puts("                   Welcome to MyOS x86_64 Kernel!                 \n");
    term_puts("==================================================================\n\n");
    
    term_color = 0xFFFFFF00;
    pci_scan();
    term_puts("\n");

    while (1) {
        term_color = 0xFFFF5555; /* Light Red */
        term_puts("root@MyOS# ");
        term_color = 0xFFFFFFFF; /* White */
        
        while (!kbd_enter_pressed) {
            __asm__ volatile ("hlt");
        }
        
        /* Process command */
        kbd_enter_pressed = 0;
        
        if (kbd_buffer[0] == '\0') {
            continue;
        }
        
        if (strcmp(kbd_buffer, "help") == 0) {
            term_puts("Available commands:\n");
            term_puts("  help  - Show this message\n");
            term_puts("  clear - Clear the screen\n");
            term_puts("  lspci - Scan and list PCI devices\n");
            term_puts("  free  - Show physical memory usage\n");
        } else if (strcmp(kbd_buffer, "clear") == 0) {
            term_clear();
        } else if (strcmp(kbd_buffer, "lspci") == 0) {
            term_color = 0xFFFFFF00;
            pci_scan();
        } else if (strcmp(kbd_buffer, "free") == 0) {
            term_color = 0xFF00FFFF; /* Cyan */
            term_puts("Total Memory: "); print_num(pmm_total_memory / (1024*1024)); term_puts(" MB\n");
            term_puts("Used Memory:  "); print_num(pmm_used_memory / (1024*1024)); term_puts(" MB\n");
            term_puts("Free Memory:  "); print_num(pmm_free_memory / (1024*1024)); term_puts(" MB\n");
        } else {
            term_puts("Command not found: ");
            term_puts(kbd_buffer);
            term_puts("\n");
        }
    }
}
