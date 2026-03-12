// kernel.c - Updated with interrupts
#include "../drivers/vga.h"
#include "../drivers/pic.h"
#include "../drivers/timer.h"
#include "../drivers/keyboard.h"
#include "gdt.h"
#include "idt.h"

void kernel_main(void) {
    vga_clear();
    vga_print("===========================================\n");
    vga_print("   MyOS - Isaac Ndoka & Promise Harare    \n");
    vga_print("   ELE-OPS-411 Class Project 2026         \n");
    vga_print("===========================================\n\n");

    vga_print("[GDT] Initializing...\n");
    gdt_init();
    vga_print("[GDT] OK\n");

    vga_print("[IDT] Initializing...\n");
    idt_init();
    vga_print("[IDT] OK\n");

    vga_print("[PIC] Initializing...\n");
    pic_init();
    vga_print("[PIC] OK\n");

    vga_print("[Timer] Starting at 100Hz...\n");
    timer_init(100);

    vga_print("[Keyboard] Ready\n");
    keyboard_init();

    // Enable interrupts! (was disabled during boot)
    __asm__ volatile ("sti");
    vga_print("\nInterrupts enabled! Timer dots below:\n");
    vga_print("Type on keyboard to see input echo:\n\n");

    // Main kernel loop
    while (1) {
        __asm__ volatile ("hlt"); // sleep until next interrupt
    }
}