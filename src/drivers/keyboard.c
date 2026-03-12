// keyboard.c - PS/2 Keyboard driver
// Keyboard fires IRQ1 (interrupt 33).
// We read the scancode from port 0x60 and convert to ASCII.

#include "keyboard.h"
#include "pic.h"
#include "../kernel/idt.h"
#include "../drivers/vga.h"

#define KEYBOARD_DATA_PORT 0x60

static char last_key = 0;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Scancode to ASCII table (US QWERTY layout, lowercase)
// Index = scancode, value = ASCII character
static const char scancode_table[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   // left ctrl
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   // left shift
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,   // right shift
    '*',
    0,   // alt
    ' ', // space
};

static void keyboard_handler(struct registers *regs) {
    (void)regs;

    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (!(scancode & 0x80)) {  // key press only (not release)
        if (scancode < 128) {
            char c = scancode_table[scancode];
            if (c) {
                last_key = c;
                vga_putchar(c);
            }
        }
    }

    pic_send_eoi(1);  // MUST be last thing we do
}

void keyboard_init(void) {
    idt_set_handler(33, keyboard_handler);
}

char keyboard_getchar(void) {
    char c = last_key;
    last_key = 0;
    return c;
}