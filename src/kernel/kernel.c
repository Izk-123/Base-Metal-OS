// kernel.c - Kernel entry point (called from boot.asm)
#include "../drivers/vga.h"

void kernel_main(void) {
    vga_clear();
    vga_print("===========================================\n");
    vga_print("   MyOS - Isaac Ndoka & Promise Harare    \n");
    vga_print("   ELE-OPS-411 Class Project 2026         \n");
    vga_print("===========================================\n");
    vga_print("\nKernel booted successfully!\n");
    vga_print("Hello from bare-metal!\n");

    // Hang forever (we'll add more later)
    while (1) {}
}