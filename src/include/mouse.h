#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

extern int mouse_x;
extern int mouse_y;

void mouse_init(void);
void mouse_handler(void);

#endif
