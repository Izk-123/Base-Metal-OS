/**
 * kernel.c – Entry point of MyOS
 * Phase 3: Creates two kernel threads (Task A and Task B)
 * and starts the preemptive round‑robin scheduler.
 */

#include "../drivers/vga.h"
#include "../drivers/pic.h"
#include "../drivers/timer.h"
#include "../drivers/keyboard.h"
#include "../proc/process.h"
#include "gdt.h"
#include "idt.h"

/**
 * A long, unoptimizable delay to make task execution visible.
 * The nested volatile loops prevent GCC from discarding them.
 * Adjust the outer loop count to change the delay duration.
 */
static void long_delay(void) {
    volatile uint32_t i, j;
    for (i = 0; i < 30; i++) {
        for (j = 0; j < 1000000; j++);
    }
}

/** Task A – prints "A<counter>" and delays */
static void task_a(void) {
    uint32_t n = 0;
    while (1) {
        vga_print("A");
        vga_print_int(n++);
        vga_print("\n");      // new line for clean output
        long_delay();
    }
}

/** Task B – prints "B<counter>" and delays */
static void task_b(void) {
    uint32_t n = 0;
    while (1) {
        vga_print("B");
        vga_print_int(n++);
        vga_print("\n");
        long_delay();
    }
}

void kernel_main(void) {
    vga_clear();
    vga_print("========================================\n");
    vga_print("   MyOS – Preemptive Round‑Robin Scheduler\n");
    vga_print("   Phase 3 Complete\n");
    vga_print("========================================\n\n");

    // Initialise all core subsystems
    gdt_init();        vga_print("[GDT] OK\n");
    idt_init();        vga_print("[IDT] OK\n");
    pic_init();        vga_print("[PIC] OK\n");
    scheduler_init();  vga_print("[Scheduler] OK\n");
    timer_init(2);     // 2 Hz – preemption every 500 ms (slow for demonstration)
    keyboard_init();   vga_print("[Keyboard] OK\n");

    // Create two kernel threads
    task_create("task_a", task_a);
    task_create("task_b", task_b);
    vga_print("[Tasks] Task A and Task B created\n");
    vga_print("\nStarting scheduler – tasks will alternate:\n");
    vga_print("----------------------------------------\n");

    // Enable interrupts (timer IRQ will now preempt)
    __asm__ volatile ("sti");

    // Idle loop – CPU halts until next interrupt
    while (1) {
        __asm__ volatile ("hlt");
    }
}