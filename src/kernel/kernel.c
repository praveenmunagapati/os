#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "serial.h"
#include "limine.h"
#include "font.h" /* font8x8_basic array */

/* Request a framebuffer from Limine */
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static struct limine_framebuffer *fb = NULL;

static size_t term_x = 0;
static size_t term_y = 0;
static uint32_t term_color = 0xFFFFFFFF; // White

static void draw_pixel(size_t x, size_t y, uint32_t color) {
    if (!fb || x >= fb->width || y >= fb->height) return;
    uint32_t *fb_ptr = fb->address;
    fb_ptr[y * (fb->pitch / 4) + x] = color;
}

static void draw_char(char c, size_t x, size_t y, uint32_t fg, uint32_t bg) {
    if (c < 0 || c > 127) c = '?';
    char *bitmap = font8x8_basic[(int)c];
    for (int r = 0; r < 8; r++) {
        for (int c_bit = 0; c_bit < 8; c_bit++) {
            if ((bitmap[r] >> c_bit) & 1) {
                draw_pixel(x + c_bit, y + r, fg);
            } else {
                draw_pixel(x + c_bit, y + r, bg);
            }
        }
    }
}

/* Scroll the framebuffer up by 8 pixels */
static void scroll(void) {
    if (!fb) return;
    uint32_t *fb_ptr = fb->address;
    size_t pitch = fb->pitch / 4;
    
    /* Move pixels up by 8 rows */
    for (size_t y = 8; y < fb->height; y++) {
        for (size_t x = 0; x < fb->width; x++) {
            fb_ptr[(y - 8) * pitch + x] = fb_ptr[y * pitch + x];
        }
    }
    /* Clear the last 8 rows */
    for (size_t y = fb->height - 8; y < fb->height; y++) {
        for (size_t x = 0; x < fb->width; x++) {
            fb_ptr[y * pitch + x] = 0x00000000;
        }
    }
    term_y -= 8;
}

void term_putchar(char c) {
    if (!fb) return;
    
    if (c == '\b') {
        if (term_x >= 8) {
            term_x -= 8;
        } else if (term_y >= 8) {
            term_y -= 8;
            term_x = (fb->width / 8) * 8 - 8;
        }
        draw_char(' ', term_x, term_y, 0, 0);
        return;
    }
    
    if (c == '\n') {
        term_x = 0;
        term_y += 8;
    } else {
        draw_char(c, term_x, term_y, term_color, 0x00000000);
        term_x += 8;
        if (term_x >= fb->width) {
            term_x = 0;
            term_y += 8;
        }
    }
    
    if (term_y >= fb->height) {
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
    for (size_t y = 0; y < fb->height; y++) {
        for (size_t x = 0; x < fb->width; x++) {
            draw_pixel(x, y, 0x00000000);
        }
    }
    term_x = 0;
    term_y = 0;
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
    term_clear();
    
    term_color = 0xFF00FF00;
    __asm__ volatile ("sti");

    /* Initialize Memory */
    pmm_init();

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
