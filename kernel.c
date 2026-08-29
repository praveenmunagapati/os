/*
 * kernel.c - Hello World x86_64 Kernel
 *
 * Writes directly to the VGA text buffer at 0xB8000.
 * Each character cell is 2 bytes: [ASCII char][attribute byte]
 *
 * No standard library headers — fully freestanding.
 */

/* Define our own integer types (no stdint.h in freestanding -nostdinc) */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned long long size_t;

/* VGA text mode constants */
#define VGA_BUFFER ((volatile uint16_t *)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

/* VGA color codes */
enum vga_color {
  VGA_BLACK = 0,
  VGA_BLUE = 1,
  VGA_GREEN = 2,
  VGA_CYAN = 3,
  VGA_RED = 4,
  VGA_MAGENTA = 5,
  VGA_BROWN = 6,
  VGA_LIGHT_GRAY = 7,
  VGA_DARK_GRAY = 8,
  VGA_LIGHT_BLUE = 9,
  VGA_LIGHT_GREEN = 10,
  VGA_LIGHT_CYAN = 11,
  VGA_LIGHT_RED = 12,
  VGA_LIGHT_MAGENTA = 13,
  VGA_YELLOW = 14,
  VGA_WHITE = 15,
};

/* Build a VGA attribute byte from foreground and background colors */
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
  return (uint8_t)(fg | (bg << 4));
}

/* Build a full VGA character entry */
static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
  return (uint16_t)c | ((uint16_t)color << 8);
}

/* Simple terminal state */
static size_t term_row;
static size_t term_col;
static uint8_t term_color;

/* Clear the entire screen */
static void term_clear(void) {
  size_t y, x;
  for (y = 0; y < VGA_HEIGHT; y++) {
    for (x = 0; x < VGA_WIDTH; x++) {
      VGA_BUFFER[y * VGA_WIDTH + x] = vga_entry(' ', term_color);
    }
  }
  term_row = 0;
  term_col = 0;
}

/* Write a single character */
static void term_putchar(char c) {
  if (c == '\n') {
    term_col = 0;
    term_row++;
    if (term_row >= VGA_HEIGHT) {
      term_row = 0;
    }
    return;
  }
  VGA_BUFFER[term_row * VGA_WIDTH + term_col] =
      vga_entry((unsigned char)c, term_color);
  term_col++;
  if (term_col >= VGA_WIDTH) {
    term_col = 0;
    term_row++;
    if (term_row >= VGA_HEIGHT) {
      term_row = 0;
    }
  }
}

/* Write a null-terminated string */
static void term_puts(const char *str) {
  while (*str) {
    term_putchar(*str++);
  }
}

/* Change the current text color */
static void term_set_color(enum vga_color fg, enum vga_color bg) {
  term_color = vga_entry_color(fg, bg);
}

/* =========================================================================
 * Kernel Entry Point
 * ========================================================================= */
void kmain(void) {
  /* Initialize terminal */
  term_color = vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK);
  term_clear();

  /* Print a banner */
  term_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
  term_puts("=================================================================="
            "==============\n");

  term_set_color(VGA_WHITE, VGA_BLACK);
  term_puts("                        Welcome to ");
  term_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
  term_puts("MyOS");
  term_set_color(VGA_WHITE, VGA_BLACK);
  term_puts(" x86_64 Kernel!\n");

  term_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
  term_puts("=================================================================="
            "==============\n");

  term_puts("\n");

  term_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
  term_puts("  >> ");
  term_set_color(VGA_WHITE, VGA_BLACK);
  term_puts("Hello, World! \n\n");

  term_set_color(VGA_LIGHT_GRAY, VGA_BLACK);
  term_puts("  Kernel loaded via Multiboot2\n");
  term_puts("  VGA text mode: 80x25\n");
  term_puts("  Identity-mapped first 1GB of RAM\n\n");

  term_set_color(VGA_DARK_GRAY, VGA_BLACK);
  term_puts("  System halted. Nothing more to do.\n");

  /* Halt the CPU */
  while (1) {
    __asm__ volatile("hlt");
  }
}
