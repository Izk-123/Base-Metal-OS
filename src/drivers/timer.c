#include "timer.h"
#include "pic.h"
#include "../kernel/idt.h"
#include "../proc/process.h"

static uint32_t tick_count = 0;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void timer_handler(struct registers *regs) {
    (void)regs;
    tick_count++;
    // Just set the flag — do NOT call schedule() here
    // The switch happens AFTER iret in the idle loop
    scheduler_tick();
    pic_send_eoi(0);
}

void timer_init(uint32_t frequency) {
    idt_set_handler(32, timer_handler);
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40,  divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

uint32_t timer_get_ticks(void) { return tick_count; }