/**
 * timer.c – Programmable Interval Timer (PIT) driver
 * Generates IRQ0 at a configurable frequency.
 * The handler sends EOI, then calls schedule() to preempt the current task.
 */

#include "timer.h"
#include "pic.h"
#include "../kernel/idt.h"
#include "../proc/process.h"
#include "../drivers/vga.h"

static uint32_t tick_count = 0;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/**
 * Timer IRQ handler.
 * Sends EOI immediately (critical – schedule may never return to this function),
 * then invokes the scheduler to possibly switch tasks.
 */
static void timer_handler(struct registers *regs) {
    (void)regs;
    tick_count++;

    // Must send EOI before schedule() – otherwise future timer IRQs are blocked
    pic_send_eoi(0);

    // Get current stack pointer and call the scheduler
    uint32_t current_esp;
    __asm__ volatile ("mov %%esp, %0" : "=r"(current_esp));
    schedule(current_esp);
}

/**
 * Initialise the PIT to generate interrupts at 'frequency' Hz.
 * The handler is installed at IRQ0 (interrupt vector 32).
 */
void timer_init(uint32_t frequency) {
    idt_set_handler(32, timer_handler);
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);          // command byte: channel 0, lobyte/hibyte, rate generator
    outb(0x40,  divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
    vga_print("[Timer] ");
    vga_print_int(frequency);
    vga_print(" Hz\n");
}

uint32_t timer_get_ticks(void) {
    return tick_count;
}