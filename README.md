# MyOS — Bare-Metal x86 Operating System Kernel

**ELE-OPS-411: Operating Systems for Engineers**  
**Malawi University of Business and Applied Sciences (MUBAS)**  
**Team:** Isaac Ndoka & Promise Harare  
**Instructor:** S. Mbewe  
**Year:** 2026  

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Features Implemented](#features-implemented)
3. [Project Structure](#project-structure)
4. [Requirements](#requirements)
5. [Building the Kernel](#building-the-kernel)
6. [Running the Kernel](#running-the-kernel)
7. [Debugging](#debugging)
8. [Architecture Overview](#architecture-overview)
9. [How Each Component Works](#how-each-component-works)
10. [Known Issues](#known-issues)
11. [Team Contributions](#team-contributions)

---

## Project Overview

MyOS is a small bare-metal operating system kernel for the **x86_64** architecture,
written in **C** with **x86 assembly stubs**. It boots on virtual hardware (QEMU),
provides interrupt handling, basic process management, a round-robin scheduler,
and a VGA text console.

The kernel is loaded by **GRUB** using the **Multiboot** specification, runs in
**32-bit protected mode**, and demonstrates the core mechanisms taught in ELE-OPS-411:
boot process, segmentation, interrupts, and multitasking.

---

## Features Implemented

### ✅ Phase 1 — Boot & Console
- Multiboot-compliant kernel entry point (`boot.asm`)
- x86 protected mode, kernel stack initialisation
- VGA text mode driver (80×25, color support, scrolling)
- Bootable ISO image via GRUB

### ✅ Phase 2 — Interrupts
- Global Descriptor Table (GDT) — flat 32-bit memory model
- Interrupt Descriptor Table (IDT) — all 256 vectors
- Assembly ISR/IRQ stubs with full register save/restore
- 8259A PIC remapping (IRQs 0-15 → vectors 32-47)
- PIT timer at 100 Hz (IRQ0)
- PS/2 keyboard driver with US QWERTY scancode table (IRQ1)

### 🔄 Phase 3 — Scheduler (Partial)
- Process Control Block (PCB) struct
- `task_create()` with private 8 KB kernel stacks
- Stack-based context switch (`do_switch` in assembly)
- Flag-based preemptive round-robin scheduler
- Task entry trampoline with interrupt enable
- Task A running and counting — full two-task alternation in progress

---

## Project Structure

```
myos/
├── src/
│   ├── boot/
│   │   ├── boot.asm          # Multiboot entry, stack setup, GDT/IDT flush
│   │   └── isr_stub.asm      # All 256 interrupt stubs (ISR + IRQ)
│   ├── kernel/
│   │   ├── kernel.c          # kernel_main() — top-level orchestrator
│   │   ├── gdt.c / gdt.h     # Global Descriptor Table
│   │   └── idt.c / idt.h     # Interrupt Descriptor Table + dispatcher
│   ├── drivers/
│   │   ├── vga.c / vga.h     # VGA text framebuffer driver
│   │   ├── pic.c / pic.h     # 8259A PIC driver (remapping + EOI)
│   │   ├── timer.c / timer.h # PIT 8253 timer driver (100 Hz)
│   │   └── keyboard.c / keyboard.h  # PS/2 keyboard driver
│   ├── proc/
│   │   ├── process.c / process.h    # PCB, task_create, scheduler
│   │   └── switch.asm               # do_switch() context switch
│   └── mm/                   # Reserved — memory management (Phase 4)
├── iso/
│   └── boot/
│       └── grub/
│           └── grub.cfg      # GRUB boot menu configuration
├── linker.ld                 # Kernel memory layout script
├── Makefile                  # Build automation
└── README.md                 # This file
```

---

## Requirements

### Both Isaac (WSL) and Promise (Ubuntu) need these packages:

```bash
sudo apt update && sudo apt upgrade -y

sudo apt install -y \
    build-essential \
    gcc \
    nasm \
    qemu-system-x86 \
    grub-pc-bin \
    xorriso \
    mtools \
    git \
    gdb
```

### Verify installation:

```bash
gcc --version        # should print GCC version
nasm --version       # should print NASM version
qemu-system-x86_64 --version   # should print QEMU version
```

---

## Building the Kernel

### Clone the repository (Promise's machine):

```bash
git clone [https://github.com/<your-repo-url>/myos.git](https://github.com/Izk-123/Base-Metal-OS.git)
cd myos
```

### Build:

```bash
make
```

This runs four steps automatically:
1. **Assemble** — NASM assembles all `.asm` files to ELF32 `.o` objects
2. **Compile** — GCC compiles all `.c` files with freestanding flags (`-ffreestanding -fno-pic -fno-pie`)
3. **Link** — GNU ld links everything using `linker.ld` into `myos.elf`
4. **ISO** — `grub-mkrescue` packages `myos.elf` into a bootable `myos.iso`

### Clean build files:

```bash
make clean
```

---

## Running the Kernel

### Run in QEMU:

```bash
make run
```

This launches:
```
qemu-system-x86_64 -cdrom myos.iso -m 32M -k en-us
```

### What you should see:

```
==========================================
  MyOS - Isaac Ndoka & Promise Harare
  ELE-OPS-411 Class Project 2026
==========================================

[GDT] OK
[IDT] OK
[PIC] OK
[Scheduler] OK
[Timer] OK
[Keyboard] OK
[Tasks] A and B created

Scheduler running:
------------------
[A=0][A=1][A=2]...
```

### Keyboard input:
- Click inside the QEMU window to grab keyboard focus
- The title bar will show: **QEMU - Press Ctrl+Alt+G to release grab**
- Type on the keyboard — characters echo to screen
- Press **Ctrl+Alt+G** to release keyboard focus back to host

### Exit QEMU:
- Click the **×** button on the QEMU window, or
- Press **Ctrl+Alt+G** then close the window, or
- In the terminal: press **Ctrl+C**

---

## Debugging

### Debug mode with GDB:

```bash
make debug
```

This launches QEMU paused and waiting for GDB, then opens GDB connected to it:

```
qemu-system-x86_64 -cdrom myos.iso -m 32M -k en-us -s -S
gdb -ex "target remote :1234" -ex "symbol-file myos.elf"
```

### Useful GDB commands inside the debugger:

```gdb
(gdb) c                        # continue execution
(gdb) b kernel_main            # set breakpoint at kernel_main
(gdb) b isr_handler            # break on any interrupt
(gdb) info registers           # show all CPU registers
(gdb) x/10x $esp               # examine 10 words at stack pointer
(gdb) x/5i $eip                # show next 5 instructions
(gdb) stepi                    # single-step one instruction
(gdb) layout asm               # show assembly view
(gdb) quit                     # exit GDB
```

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                      QEMU / Hardware                    │
├─────────────────────────────────────────────────────────┤
│  GRUB Bootloader (Multiboot)                            │
│    └── Loads myos.elf at 0x100000, jumps to _start      │
├─────────────────────────────────────────────────────────┤
│  boot.asm (_start)                                      │
│    └── Sets stack → calls kernel_main()                 │
├─────────────────────────────────────────────────────────┤
│  kernel_main()                                          │
│    ├── gdt_init()     — loads GDT, reloads segments     │
│    ├── idt_init()     — loads IDT with all 256 entries  │
│    ├── pic_init()     — remaps IRQs 0-15 to 32-47       │
│    ├── scheduler_init() — sets up idle task (slot 0)    │
│    ├── timer_init(100)  — PIT at 100 Hz                 │
│    ├── keyboard_init()  — registers IRQ1 handler        │
│    ├── task_create(task_a) — creates Task A (slot 1)    │
│    ├── task_create(task_b) — creates Task B (slot 2)    │
│    ├── sti              — enable interrupts             │
│    └── idle loop: schedule() + hlt                      │
├─────────────────────────────────────────────────────────┤
│  Interrupt Flow                                         │
│    Hardware IRQ → PIC → CPU looks up IDT[vector]        │
│    → isr_stub (save regs) → C handler → EOI → iret     │
├─────────────────────────────────────────────────────────┤
│  Scheduler Flow                                         │
│    Timer IRQ → scheduler_tick() → sets need_switch=1   │
│    idle loop → schedule() → do_switch(old_esp, new_esp) │
│    → Task A runs → timer → switch → Task B runs → ...  │
└─────────────────────────────────────────────────────────┘
```

---

## How Each Component Works

### VGA Driver (`src/drivers/vga.c`)
The VGA text framebuffer is memory-mapped at physical address `0xB8000`.
Each character cell is 2 bytes: `[ASCII][color]`. The color byte encodes
foreground (lower 4 bits) and background (upper 4 bits) using the 16-color
CGA palette. The driver maintains a cursor position and scrolls the screen
when the cursor passes row 24.

### GDT (`src/kernel/gdt.c`)
Three entries: null descriptor (required), kernel code segment (ring 0,
executable), kernel data segment (ring 0, writable). Both segments use base=0
and limit=4 GB (flat model). Loaded with the `lgdt` instruction. A far jump
reloads CS since it cannot be changed with `mov`.

### IDT (`src/kernel/idt.c`)
256 interrupt gate descriptors, each storing a handler address split into
two 16-bit fields, selector 0x08 (kernel code), and flags 0x8E (present,
ring 0, 32-bit interrupt gate). Loaded with `lidt`. Drivers register C
handlers with `idt_set_handler(vector, fn)`.

### ISR Stubs (`src/boot/isr_stub.asm`)
NASM macros generate one stub per interrupt. Each stub pushes a dummy error
code (for exceptions that don't push one) and the interrupt number, then
jumps to `isr_common` or `irq_common`. The common handler does:
`pusha` → save DS → switch to kernel DS → call C handler →
restore DS → `popa` → `add esp, 8` → `sti` → `iret`.

### PIC (`src/drivers/pic.c`)
The 8259A PIC is initialised with a 4-command ICW sequence. Master PIC
is remapped to base vector 32 (IRQs 0-7 → 32-39), slave PIC to base
vector 40 (IRQs 8-15 → 40-47). After each IRQ handler runs,
`pic_send_eoi()` sends command 0x20 to the master (and slave if the IRQ
came from the slave) to allow further interrupts.

### Context Switch (`src/proc/switch.asm`)
`do_switch(uint32_t *old_esp, uint32_t new_esp)` saves the four
callee-saved registers (ebp, ebx, esi, edi) to the current stack, saves
ESP into `*old_esp`, loads `new_esp`, restores the four registers, and
executes `ret`. The return address on the new stack determines where
execution resumes.

### Scheduler (`src/proc/process.c`)
The timer IRQ calls `scheduler_tick()` which sets `need_switch = 1`
every 10 ticks. The idle loop calls `schedule()` which reads the flag,
does a round-robin search skipping slot 0 (idle), and calls `do_switch`
to transfer execution. This flag-based approach ensures context switches
only happen outside interrupt context, preventing stack corruption.

---

## Known Issues

| Issue | Status |
|-------|--------|
| Full two-task (A + B) alternation not yet visible | Debugging in progress |
| No user-mode (ring 3) support | Future work |
| No memory protection between tasks | Future work |
| No filesystem | Future work |

---

## Team Contributions

| Member | Contributions |
|--------|--------------|
| **Isaac Ndoka** | Boot entry, linker script, Makefile, GDT, IDT, ISR stubs, scheduler architecture, context switch assembly, debugging |
| **Promise Harare** | VGA driver, PIC driver, timer driver, keyboard driver, kernel_main, PCB design, testing |

Both members collaborated on design decisions, debugging sessions, and documentation.

---

## Build Flags Reference

| Flag | Meaning |
|------|---------|
| `-m32` | Compile for 32-bit x86 |
| `-ffreestanding` | No standard library, no assumptions about environment |
| `-fno-builtin` | Don't replace functions with built-in equivalents |
| `-fno-stack-protector` | Disable stack canaries (no libc support) |
| `-fno-pic` | No position-independent code |
| `-fno-pie` | No position-independent executable |
| `-nostdlib` | Don't link standard libraries |
| `-nodefaultlibs` | Don't use default libraries |

---

## References

- [OSDev Wiki](https://wiki.osdev.org) — Multiboot, GDT, IDT, PIC, PIT, Context Switching
- Intel 64 and IA-32 Architectures Software Developer Manuals, Vol. 3A
- [NASM Manual](https://www.nasm.us/doc/)
- [QEMU Documentation](https://www.qemu.org/docs/master/)
- GNU Binutils ld — Linker Script Reference
- ELE-OPS-411 Class Project Specification — Instructor S. Mbewe, MUBAS 2026

---

*MyOS — Built from scratch, one interrupt at a time.* 🖥️
