// vga.c - VGA text mode driver
// VGA text buffer is at physical address 0xB8000
// Each cell = 2 bytes: [ASCII][color]
// Screen is 80 columns x 25 rows

#include "vga.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_BUFFER ((volatile unsigned short*)0xB8000)

static int cursor_row = 0;
static int cursor_col = 0;

// VGA colors
typedef enum {
    BLACK = 0, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GREY,
    DARK_GREY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, LIGHT_RED,
    LIGHT_MAGENTA, YELLOW, WHITE
} vga_color;

// Pack foreground + background color into one byte
static unsigned char make_color(vga_color fg, vga_color bg) {
    return fg | (bg << 4);
}

// Pack character + color into a VGA entry (2 bytes)
static unsigned short make_entry(char c, unsigned char color) {
    return (unsigned short)c | ((unsigned short)color << 8);
}

// Clear the entire screen
void vga_clear(void) {
    unsigned char color = make_color(WHITE, BLACK);
    for (int row = 0; row < VGA_HEIGHT; row++)
        for (int col = 0; col < VGA_WIDTH; col++)
            VGA_BUFFER[row * VGA_WIDTH + col] = make_entry(' ', color);
    cursor_row = 0;
    cursor_col = 0;
}

// Scroll screen up by one line
static void scroll(void) {
    unsigned char color = make_color(WHITE, BLACK);
    // Move every row up by one
    for (int row = 0; row < VGA_HEIGHT - 1; row++)
        for (int col = 0; col < VGA_WIDTH; col++)
            VGA_BUFFER[row * VGA_WIDTH + col] =
                VGA_BUFFER[(row + 1) * VGA_WIDTH + col];
    // Clear the last row
    for (int col = 0; col < VGA_WIDTH; col++)
        VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = make_entry(' ', color);
    cursor_row = VGA_HEIGHT - 1;
}

// Print a single character
void vga_putchar(char c) {
    if (c == '\n') {           // newline
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\r') {    // carriage return
        cursor_col = 0;
    } else if (c == '\b') {    // backspace
        if (cursor_col > 0) cursor_col--;
    } else {
        unsigned char color = make_color(LIGHT_GREEN, BLACK);
        VGA_BUFFER[cursor_row * VGA_WIDTH + cursor_col] = make_entry(c, color);
        cursor_col++;
        if (cursor_col >= VGA_WIDTH) {
            cursor_col = 0;
            cursor_row++;
        }
    }
    if (cursor_row >= VGA_HEIGHT)
        scroll();
}

// Print a null-terminated string
void vga_print(const char *str) {
    while (*str)
        vga_putchar(*str++);
}