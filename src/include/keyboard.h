#ifndef KEYBOARD_H
#define KEYBOARD_H

extern char kbd_buffer[256];
extern volatile int kbd_enter_pressed;

void init_keyboard(void);
void keyboard_handler(void);

#endif
