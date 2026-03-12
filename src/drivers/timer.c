// timer.c - PIT (Programmable Interval Timer) driver
// The PIT chip fires IRQ0 at a configurable frequency.
// We'll set it to ~100 Hz (every 10ms) — our scheduler heartbeat.

#include "timer.h"
#include "pic.h"
#include "../kernel/idt.h"
#include "../drivers/vga.h"

static uint32_t tick_count = 0;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Called on every timer tick (IRQ0 = interrupt 32)
static void timer_handler(struct registers *regs) {
    (void)regs;
    tick_count++;
    if (tick_count % 100 == 0) {
        vga_print(".");
    }
    pic_send_eoi(0);  // tell PIC we handled IRQ0
}

void timer_init(uint32_t frequency) {
    // Register our C handler for IRQ0 (interrupt 32)
    idt_set_handler(32, timer_handler);

    // Configure PIT frequency
    // PIT base frequency is 1,193,180 Hz
    uint32_t divisor = 1193180 / frequency;

    outb(0x43, 0x36);                    // command: channel 0, rate generator
    outb(0x40, divisor & 0xFF);          // low byte of divisor
    outb(0x40, (divisor >> 8) & 0xFF);   // high byte of divisor
}

uint32_t timer_get_ticks(void) {
    return tick_count;
}