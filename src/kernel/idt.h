#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Registers saved during interrupt (pushed by our ASM stub)
// This lets our C handler see the CPU state at interrupt time
struct registers {
    uint32_t ds;                            // data segment
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // pushed by pusha
    uint32_t int_no, err_code;              // interrupt number + error
    uint32_t eip, cs, eflags, useresp, ss; // pushed by CPU automatically
};

void idt_init(void);
void idt_set_handler(uint8_t n, void (*handler)(struct registers*));

#endif