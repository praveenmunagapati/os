#include "gui.h"
#include "limine.h"
#include "font.h"

extern struct limine_framebuffer *fb;

void gui_draw_rect(size_t x, size_t y, size_t width, size_t height, uint32_t color) {
    if (!fb) return;
    uint32_t *fb_ptr = fb->address;
    size_t pitch = fb->pitch / 4;

    /* Clamp bounds */
    if (x + width > fb->width) width = fb->width - x;
    if (y + height > fb->height) height = fb->height - y;

    for (size_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            fb_ptr[(y + row) * pitch + (x + col)] = color;
        }
    }
}

void gui_draw_char(char c, size_t x, size_t y, uint32_t fg, uint32_t bg) {
    if (c < 0 || c > 127) c = '?';
    char *bitmap = font8x8_basic[(int)c];
    uint32_t *fb_ptr = fb->address;
    size_t pitch = fb->pitch / 4;

    for (int r = 0; r < 8; r++) {
        for (int c_bit = 0; c_bit < 8; c_bit++) {
            if ((y + r) >= fb->height || (x + c_bit) >= fb->width) continue;
            
            if ((bitmap[r] >> c_bit) & 1) {
                fb_ptr[(y + r) * pitch + (x + c_bit)] = fg;
            } else {
                /* Transparent background if bg is 0, otherwise fill */
                if (bg != 0) {
                    fb_ptr[(y + r) * pitch + (x + c_bit)] = bg;
                }
            }
        }
    }
}

static void gui_draw_string(const char* str, size_t x, size_t y, uint32_t fg, uint32_t bg) {
    size_t curr_x = x;
    while (*str) {
        gui_draw_char(*str, curr_x, y, fg, bg);
        curr_x += 8;
        str++;
    }
}

void gui_draw_desktop(void) {
    /* Background: Deep Teal/Slate */
    gui_draw_rect(0, 0, fb->width, fb->height, 0xFF0A2229);

    /* Taskbar: Dark Gray at the bottom (40px high) */
    gui_draw_rect(0, fb->height - 40, fb->width, 40, 0xFF1E1E1E);
    
    /* Taskbar top border */
    gui_draw_rect(0, fb->height - 40, fb->width, 2, 0xFF3A3A3A);

    /* Start Button */
    gui_draw_rect(10, fb->height - 32, 60, 24, 0xFF303030);
    gui_draw_string("START", 20, fb->height - 24, 0xFFFFFFFF, 0);
    
    /* Clock placeholder */
    gui_draw_string("12:00 PM", fb->width - 80, fb->height - 24, 0xFFFFFFFF, 0);
}

void gui_draw_window(const char* title, size_t x, size_t y, size_t width, size_t height) {
    /* Window Shadow */
    gui_draw_rect(x + 4, y + 4, width, height, 0x88000000);

    /* Window Border & Background */
    gui_draw_rect(x, y, width, height, 0xFF444444);
    gui_draw_rect(x + 2, y + 2, width - 4, height - 4, 0xFF121212); // Inner Terminal Background

    /* Title Bar */
    gui_draw_rect(x, y, width, 24, 0xFF2A2A2A);
    
    /* Title Text */
    gui_draw_string(title, x + 8, y + 8, 0xFFFFFFFF, 0);

    /* Close Button */
    gui_draw_rect(x + width - 24, y, 24, 24, 0xFFD32F2F); // Red
    gui_draw_string("X", x + width - 16, y + 8, 0xFFFFFFFF, 0);
}

/* --- Mouse Cursor Drawing --- */
#include "mouse.h"

#define CURSOR_W 10
#define CURSOR_H 15

static uint32_t cursor_bg[CURSOR_W * CURSOR_H];
static int cursor_visible = 0;

/* Save the pixels under the mouse */
static void save_cursor_bg(int x, int y) {
    if (!fb) return;
    uint32_t *fb_ptr = fb->address;
    size_t pitch = fb->pitch / 4;
    for (int r = 0; r < CURSOR_H; r++) {
        for (int c = 0; c < CURSOR_W; c++) {
            if (x + c < fb->width && y + r < fb->height) {
                cursor_bg[r * CURSOR_W + c] = fb_ptr[(y + r) * pitch + (x + c)];
            }
        }
    }
}

/* Restore the pixels under the mouse */
static void restore_cursor_bg(int x, int y) {
    if (!fb) return;
    uint32_t *fb_ptr = fb->address;
    size_t pitch = fb->pitch / 4;
    for (int r = 0; r < CURSOR_H; r++) {
        for (int c = 0; c < CURSOR_W; c++) {
            if (x + c < fb->width && y + r < fb->height) {
                fb_ptr[(y + r) * pitch + (x + c)] = cursor_bg[r * CURSOR_W + c];
            }
        }
    }
}

/* Draw a simple triangle cursor */
static void draw_cursor(int x, int y) {
    if (!fb) return;
    uint32_t *fb_ptr = fb->address;
    size_t pitch = fb->pitch / 4;
    for (int r = 0; r < CURSOR_H; r++) {
        for (int c = 0; c < CURSOR_W; c++) {
            if (x + c < fb->width && y + r < fb->height) {
                if (c <= r / 2) { /* Draw a slanted triangle shape */
                    fb_ptr[(y + r) * pitch + (x + c)] = 0xFFFFFFFF; // White pointer
                }
            }
        }
    }
}

void gui_update_cursor(int new_x, int new_y) {
    if (!fb) return;
    
    /* Clamp bounds */
    if (new_x < 0) new_x = 0;
    if (new_y < 0) new_y = 0;
    if (new_x >= fb->width - 2) new_x = fb->width - 2;
    if (new_y >= fb->height - 2) new_y = fb->height - 2;

    /* If it was visible, erase the old cursor */
    if (cursor_visible) {
        restore_cursor_bg(mouse_x, mouse_y);
    } else {
        cursor_visible = 1;
    }

    /* Update coordinates */
    mouse_x = new_x;
    mouse_y = new_y;

    /* Save the new background and draw */
    save_cursor_bg(mouse_x, mouse_y);
    draw_cursor(mouse_x, mouse_y);
}
