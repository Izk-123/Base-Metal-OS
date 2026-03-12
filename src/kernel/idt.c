// idt.c - Interrupt Descriptor Table
// The IDT has 256 entries. Each entry points to an ISR (Interrupt Service Routine).

#include "idt.h"
#include <stdint.h>
#include <stddef.h>

struct idt_entry {
    uint16_t base_low;   // lower 16 bits of handler address
    uint16_t selector;   // kernel code segment selector (0x08)
    uint8_t  zero;       // always 0
    uint8_t  flags;      // type and attributes
    uint16_t base_high;  // upper 16 bits of handler address
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr   ip;

// C-level handlers registered by drivers
static void (*handlers[256])(struct registers*);

// Assembly stubs (defined in isr_stub.asm)
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);
extern void isr3(void);  extern void isr4(void);  extern void isr5(void);
extern void isr6(void);  extern void isr7(void);  extern void isr8(void);
extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void);
extern void isr15(void); extern void isr16(void); extern void isr17(void);
extern void isr18(void); extern void isr19(void); extern void isr20(void);
extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void);
extern void isr27(void); extern void isr28(void); extern void isr29(void);
extern void isr30(void); extern void isr31(void);
// IRQs (hardware interrupts, remapped to 32-47)
extern void irq0(void);  extern void irq1(void);  extern void irq2(void);
extern void irq3(void);  extern void irq4(void);  extern void irq5(void);
extern void irq6(void);  extern void irq7(void);  extern void irq8(void);
extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void);
extern void irq15(void);

extern void idt_flush(uint32_t);

static void idt_set_entry(uint8_t n, uint32_t base) {
    idt[n].base_low  = base & 0xFFFF;
    idt[n].base_high = (base >> 16) & 0xFFFF;
    idt[n].selector  = 0x08; // kernel code segment
    idt[n].zero      = 0;
    idt[n].flags     = 0x8E; // present, ring 0, 32-bit interrupt gate
}

void idt_init(void) {
    ip.limit = (sizeof(struct idt_entry) * 256) - 1;
    ip.base  = (uint32_t)&idt;

    // CPU exceptions (0-31)
    idt_set_entry(0,  (uint32_t)isr0);
    idt_set_entry(1,  (uint32_t)isr1);
    idt_set_entry(2,  (uint32_t)isr2);
    idt_set_entry(3,  (uint32_t)isr3);
    idt_set_entry(4,  (uint32_t)isr4);
    idt_set_entry(5,  (uint32_t)isr5);
    idt_set_entry(6,  (uint32_t)isr6);
    idt_set_entry(7,  (uint32_t)isr7);
    idt_set_entry(8,  (uint32_t)isr8);
    idt_set_entry(9,  (uint32_t)isr9);
    idt_set_entry(10, (uint32_t)isr10);
    idt_set_entry(11, (uint32_t)isr11);
    idt_set_entry(12, (uint32_t)isr12);
    idt_set_entry(13, (uint32_t)isr13);
    idt_set_entry(14, (uint32_t)isr14);
    idt_set_entry(15, (uint32_t)isr15);
    idt_set_entry(16, (uint32_t)isr16);
    idt_set_entry(17, (uint32_t)isr17);
    idt_set_entry(18, (uint32_t)isr18);
    idt_set_entry(19, (uint32_t)isr19);
    idt_set_entry(20, (uint32_t)isr20);
    idt_set_entry(21, (uint32_t)isr21);
    idt_set_entry(22, (uint32_t)isr22);
    idt_set_entry(23, (uint32_t)isr23);
    idt_set_entry(24, (uint32_t)isr24);
    idt_set_entry(25, (uint32_t)isr25);
    idt_set_entry(26, (uint32_t)isr26);
    idt_set_entry(27, (uint32_t)isr27);
    idt_set_entry(28, (uint32_t)isr28);
    idt_set_entry(29, (uint32_t)isr29);
    idt_set_entry(30, (uint32_t)isr30);
    idt_set_entry(31, (uint32_t)isr31);
    // Hardware IRQs remapped to 32-47
    idt_set_entry(32, (uint32_t)irq0);
    idt_set_entry(33, (uint32_t)irq1);
    idt_set_entry(34, (uint32_t)irq2);
    idt_set_entry(35, (uint32_t)irq3);
    idt_set_entry(36, (uint32_t)irq4);
    idt_set_entry(37, (uint32_t)irq5);
    idt_set_entry(38, (uint32_t)irq6);
    idt_set_entry(39, (uint32_t)irq7);
    idt_set_entry(40, (uint32_t)irq8);
    idt_set_entry(41, (uint32_t)irq9);
    idt_set_entry(42, (uint32_t)irq10);
    idt_set_entry(43, (uint32_t)irq11);
    idt_set_entry(44, (uint32_t)irq12);
    idt_set_entry(45, (uint32_t)irq13);
    idt_set_entry(46, (uint32_t)irq14);
    idt_set_entry(47, (uint32_t)irq15);

    idt_flush((uint32_t)&ip);
}

// Register a C handler for a given interrupt number
void idt_set_handler(uint8_t n, void (*handler)(struct registers*)) {
    handlers[n] = handler;
}

// Called from assembly stubs — dispatches to registered handler
void isr_handler(struct registers regs) {
    if (handlers[regs.int_no])
        handlers[regs.int_no](&regs);
}

// Called from assembly stubs for hardware IRQs
void irq_handler(struct registers regs) {
    if (handlers[regs.int_no])
        handlers[regs.int_no](&regs);
}