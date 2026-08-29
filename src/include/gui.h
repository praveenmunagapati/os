#ifndef GUI_H
#define GUI_H

#include <stdint.h>
#include <stddef.h>

void gui_init(void);
void gui_draw_rect(size_t x, size_t y, size_t width, size_t height, uint32_t color);
void gui_draw_char(char c, size_t x, size_t y, uint32_t fg, uint32_t bg);
void gui_draw_window(const char* title, size_t x, size_t y, size_t width, size_t height);
void gui_draw_desktop(void);

#endif
