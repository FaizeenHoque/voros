#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

void keyboard_init(void);
void keyboard_irq_handler(void);
void keyboard_poll(void);
bool keyboard_read_char(char *out_char);

#endif