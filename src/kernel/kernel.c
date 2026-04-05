#include "../drivers/vga.h"
#include "../drivers/pic.h"
#include "../drivers/timer.h"
#include "../drivers/keyboard.h"
#include "../proc/process.h"
#include "../shell/shell.h"
#include "gdt.h"
#include "idt.h"
#include "syscall.h"

// Shell runs as its own task
static void shell_task(void) {
    shell_run();   // never returns
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
    timer_init(20);    vga_print("[Timer] OK\n");
    keyboard_init();   vga_print("[Keyboard] OK\n");
    syscall_init();

    __asm__ volatile ("sti");   // enable interrupts NOW before shell starts

    vga_print("\nBooting shell...\n");

    // Run shell directly in kernel_main — no task switch needed
    // This guarantees keyboard interrupts reach the shell
    shell_run();

    // Should never reach here
    while (1) __asm__ volatile ("hlt");
}