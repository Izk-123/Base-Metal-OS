#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_init(void);
char keyboard_getchar(void);        // returns last key, 0 if none
int  keyboard_read_line(char *buf, int max); // blocking read of a line

#endif