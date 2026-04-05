// keyboard.c - PS/2 keyboard driver with line buffer

#include "keyboard.h"
#include "pic.h"
#include "../kernel/idt.h"
#include "../drivers/vga.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEY_BUFFER_SIZE    256

// Circular key buffer
static volatile char key_buffer[KEY_BUFFER_SIZE];
static volatile int  buf_head = 0;  // write position
static volatile int  buf_tail = 0;  // read position

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// US QWERTY scancode to ASCII table
static const char scancode_table[128] = {
    0,   27,  '1','2','3','4','5','6','7','8','9','0',
    '-', '=', '\b', '\t',
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   // left ctrl
    'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   // left shift
    '\\','z','x','c','v','b','n','m',',','.','/',
    0,   // right shift
    '*',
    0,   // alt
    ' ', // space
};

static void keyboard_handler(struct registers *regs) {
    (void)regs;
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (!(scancode & 0x80)) {  // key press only
        if (scancode < 128) {
            char c = scancode_table[scancode];
            if (c) {
                // Add to circular buffer
                int next_head = (buf_head + 1) % KEY_BUFFER_SIZE;
                if (next_head != buf_tail) { // buffer not full
                    key_buffer[buf_head] = c;
                    buf_head = next_head;
                }
            }
        }
    }
    pic_send_eoi(1);
}

void keyboard_init(void) {
    idt_set_handler(33, keyboard_handler);
}

// Returns next character from buffer, 0 if empty
char keyboard_getchar(void) {
    if (buf_head == buf_tail) return 0;
    char c = key_buffer[buf_tail];
    buf_tail = (buf_tail + 1) % KEY_BUFFER_SIZE;
    return c;
}

// Blocking read — waits for Enter key, returns line in buf
// Echoes characters to screen, handles backspace
int keyboard_read_line(char *buf, int max) {
    int i = 0;
    while (1) {
        // Enable interrupts and wait — keyboard IRQ will fire and fill buffer
        __asm__ volatile ("sti; hlt");

        // Check if anything arrived
        char c = keyboard_getchar();
        if (!c) continue;

        if (c == '\n') {
            buf[i] = '\0';
            vga_putchar('\n');
            return i;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                vga_putchar('\b');
                vga_putchar(' ');
                vga_putchar('\b');
            }
        } else if (i < max - 1) {
            buf[i++] = c;
            vga_putchar(c);
        }
    }
}