// pic.c - Programmable Interrupt Controller
// The PIC routes hardware IRQs to CPU interrupt numbers.
// By default IRQs 0-7 map to CPU interrupts 8-15 which CLASH with CPU exceptions!
// We REMAP them to interrupts 32-47 (safe zone after the 32 CPU exceptions).

#include "pic.h"
#include <stdint.h>

#define PIC1_CMD  0x20   // master PIC command port
#define PIC1_DATA 0x21   // master PIC data port
#define PIC2_CMD  0xA0   // slave PIC command port
#define PIC2_DATA 0xA1   // slave PIC data port
#define PIC_EOI   0x20   // End Of Interrupt command

// Write a byte to an I/O port
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Read a byte from an I/O port
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Small delay (needed between PIC commands)
static inline void io_wait(void) {
    outb(0x80, 0); // write to unused port = tiny delay
}

void pic_init(void) {
    // Save current interrupt masks
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    // Start initialization sequence (ICW1)
    outb(PIC1_CMD,  0x11); io_wait(); // init master
    outb(PIC2_CMD,  0x11); io_wait(); // init slave

    // ICW2: Set vector offsets (remap IRQs)
    outb(PIC1_DATA, 0x20); io_wait(); // master IRQs start at 32
    outb(PIC2_DATA, 0x28); io_wait(); // slave IRQs start at 40

    // ICW3: Tell PICs about each other
    outb(PIC1_DATA, 0x04); io_wait(); // master: slave on IRQ2
    outb(PIC2_DATA, 0x02); io_wait(); // slave: cascade identity

    // ICW4: Set 8086 mode
    outb(PIC1_DATA, 0x01); io_wait();
    outb(PIC2_DATA, 0x01); io_wait();

    // Restore saved masks
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

// Tell PIC we finished handling the interrupt (MUST call this or no more IRQs!)
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_CMD, PIC_EOI); // if from slave PIC, tell slave too
    outb(PIC1_CMD, PIC_EOI);     // always tell master
}