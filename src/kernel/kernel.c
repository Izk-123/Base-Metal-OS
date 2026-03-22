#include "../drivers/vga.h"
#include "../drivers/pic.h"
#include "../drivers/timer.h"
#include "../drivers/keyboard.h"
#include "../proc/process.h"
#include "gdt.h"
#include "idt.h"

static void task_a(void) {
    uint32_t n = 0;
    while (1) {
        vga_print("[A=");
        vga_print_int(n++);
        vga_print("]");
        for (volatile uint32_t i = 0; i < 5000000; i++);
    }
}

static void task_b(void) {
    uint32_t n = 0;
    while (1) {
        vga_print("[B=");
        vga_print_int(n++);
        vga_print("]");
        for (volatile uint32_t i = 0; i < 5000000; i++);
    }
}

void kernel_main(void) {
    vga_clear();
    vga_print("===========================================\n");
    vga_print("   MyOS - Isaac Ndoka & Promise Harare    \n");
    vga_print("   ELE-OPS-411 Class Project 2026         \n");
    vga_print("===========================================\n\n");

    gdt_init();        vga_print("[GDT] OK\n");
    idt_init();        vga_print("[IDT] OK\n");
    pic_init();        vga_print("[PIC] OK\n");
    scheduler_init();  vga_print("[Scheduler] OK\n");
    timer_init(100);   vga_print("[Timer] OK\n");
    keyboard_init();   vga_print("[Keyboard] OK\n");

    task_create("task_a", task_a);
    task_create("task_b", task_b);
    vga_print("[Tasks] A and B created\n\n");
    vga_print("Scheduler running:\n");
    vga_print("------------------\n");

    __asm__ volatile ("sti");

    // Idle loop — schedule() is called HERE, outside any IRQ
    // This is the key fix: context switch never happens inside an IRQ
    while (1) {
        schedule();
        __asm__ volatile ("hlt");
    }
}