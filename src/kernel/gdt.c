// gdt.c - Global Descriptor Table
// The GDT tells the CPU about memory segments.
// In 32-bit protected mode we need at least:
//   0: null descriptor (required by CPU)
//   1: kernel code segment
//   2: kernel data segment

#include "gdt.h"
#include <stdint.h>

// A GDT entry is 8 bytes, packed into this structure
struct gdt_entry {
    uint16_t limit_low;    // lower 16 bits of segment limit
    uint16_t base_low;     // lower 16 bits of base address
    uint8_t  base_mid;     // next 8 bits of base address
    uint8_t  access;       // access flags (ring level, type)
    uint8_t  granularity;  // granularity + upper 4 bits of limit
    uint8_t  base_high;    // upper 8 bits of base address
} __attribute__((packed)); // packed = no padding bytes

// GDTR register value - pointer to GDT + size
struct gdt_ptr {
    uint16_t limit;        // size of GDT - 1
    uint32_t base;         // address of GDT
} __attribute__((packed));

static struct gdt_entry gdt[3]; // 3 entries: null, code, data
static struct gdt_ptr   gp;

// Assembly function that loads the GDT into the CPU
extern void gdt_flush(uint32_t);

static void gdt_set_entry(int i, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t gran) {
    gdt[i].base_low    = (base & 0xFFFF);
    gdt[i].base_mid    = (base >> 16) & 0xFF;
    gdt[i].base_high   = (base >> 24) & 0xFF;
    gdt[i].limit_low   = (limit & 0xFFFF);
    gdt[i].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].access      = access;
}

void gdt_init(void) {
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base  = (uint32_t)&gdt;

    gdt_set_entry(0, 0, 0,          0,    0);    // null
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data

    gdt_flush((uint32_t)&gp);
}