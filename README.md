# 🖥️ MyOS — Bare-Metal Operating System Kernel

### ELE-OPS-411: Operating Systems for Engineers

👨‍💻 **Authors:** Isaac Ndoka & Promise Harare
🏫 Malawi University of Business and Applied Sciences (MUBAS)
📅 Academic Year: 2026
👨‍🏫 Instructor: S. Mbewe

---

## 📌 Overview

**MyOS** is a **bare-metal operating system kernel** built from scratch for the x86 architecture.
It runs on **QEMU virtual hardware**, boots via **GRUB (Multiboot)**, and is implemented in **C with assembly stubs**.

The project demonstrates core operating system mechanisms including:

* Boot process and kernel initialization
* Interrupt handling (GDT, IDT, PIC)
* Preemptive multitasking
* Context switching
* System calls (`int 0x80`)
* Interactive command-line shell

---

## ⚙️ Features

### ✅ Core Features (Completed)

* **Boot & Kernel Entry**

  * Multiboot-compliant kernel
  * Stack setup and C runtime initialization

* **VGA Console Driver**

  * 80×25 text mode output
  * Scrolling and formatted printing

* **Interrupt System**

  * Global Descriptor Table (GDT)
  * Interrupt Descriptor Table (IDT)
  * ISR/IRQ handling with assembly stubs

* **Hardware Drivers**

  * Programmable Interrupt Controller (PIC)
  * Timer (PIT) — configurable frequency
  * Keyboard driver with ring buffer

* **Scheduler**

  * Preemptive round-robin scheduling
  * Timer-driven context switching

* **Context Switching**

  * Full CPU state save/restore
  * Per-task kernel stacks

* **System Calls (`int 0x80`)**

  * `write` — console output
  * `spawn` — create new task
  * `exit` — terminate task

* **Interactive Shell**

  * `help`, `clear`, `tasks`, `spawn`, `about`, `exit`
  * Command parsing without libc

---

## 🛠️ Tech Stack

| Category        | Tools                      |
| --------------- | -------------------------- |
| Language        | C, Assembly                |
| Compiler        | GCC (`-m32`, freestanding) |
| Assembler       | NASM                       |
| Emulator        | QEMU                       |
| Bootloader      | GRUB (Multiboot)           |
| Build System    | Make                       |
| Debugging       | GDB                        |
| Version Control | Git (GitHub)               |

---

## 📂 Project Structure

```
myos/
├── src/
│   ├── boot/         # boot.asm, ISR stubs
│   ├── kernel/       # kernel, GDT, IDT, syscalls
│   ├── drivers/      # VGA, PIC, timer, keyboard
│   ├── proc/         # scheduler, PCB, context switch
│   ├── shell/        # interactive shell
├── iso/boot/grub/    # grub.cfg
├── linker.ld         # memory layout
├── Makefile          # build automation
└── README.md
```

---

## ▶️ Build Instructions

### 🔧 Prerequisites

Install required tools:

```bash
sudo apt update
sudo apt install build-essential nasm qemu-system-x86 grub-pc-bin xorriso
```

---

### 🏗️ Build the OS

```bash
make clean
make
```

---

## ▶️ Run MyOS

```bash
make run
```

Or manually:

```bash
qemu-system-x86_64 -cdrom myos.iso -k en-us
```

👉 Click inside the QEMU window to enable keyboard input.

---

## 🧪 Testing

✔ Kernel boots via GRUB
✔ VGA output displays correctly
✔ Timer interrupts trigger scheduling
✔ Tasks switch preemptively
✔ Keyboard input works via ring buffer
✔ Shell accepts and executes commands
✔ System calls operate correctly

---

## 💡 Key Design Decisions

* **Flat Memory Model**
  Simplifies segmentation (base=0, limit=4GB), deferring memory protection to future paging.

* **Freestanding Environment**
  No libc — all utilities implemented manually (string handling, printing, parsing).

* **EOI Before Scheduling**
  Ensures PIC continues delivering interrupts after context switch.

* **`int 0x80` Syscall Interface**
  Chosen for simplicity and alignment with early Linux design.

---

## ⚠️ Limitations

* No paging or virtual memory
* No dynamic memory allocation (`malloc/free`)
* No user-mode (ring 3)
* No filesystem
* Single-core execution only

---

## 🚀 Future Work

* Paging and virtual memory
* User-mode processes (ring 3)
* Filesystem support
* IPC mechanisms (pipes, message queues)
* Additional system calls

---

## 🎥 Demo

The demo video demonstrates:

* Boot process
* Interrupt handling
* Task scheduling
* Shell interaction and commands

---

## 📖 Documentation

For full technical details, refer to the **Final Report** included in this repository:

* Design decisions
* Algorithms and data structures
* Debugging challenges
* Implementation phases

---

## 👥 Team Contributions

| Area           | Isaac Ndoka                     | Promise Harare               |
| -------------- | ------------------------------- | ---------------------------- |
| Boot & Linking | boot.asm, linker.ld, Makefile   | grub.cfg, kernel integration |
| Interrupts     | ISR stubs, IDT setup            | PIC, timer, keyboard drivers |
| Scheduler      | Context switch (ASM), debugging | PCB design, scheduling logic |
| System Calls   | syscall interface               | shell implementation         |
| Documentation  | README, debugging logs          | Report writing, testing      |

---

## 🧠 Reflection

This project provided deep practical insight into:

* Low-level system design
* Hardware-software interaction
* Interrupt-driven execution
* Kernel debugging techniques

Building a working OS from scratch reinforced the importance of **precision, debugging discipline, and architectural thinking**.

---

## 📌 How to Use (For Evaluation)

1. Clone the repository
2. Run `make run`
3. Observe kernel boot
4. Interact with the shell using commands:

   ```
   help
   tasks
   spawn
   about
   exit
   ```

---

## ⭐ Final Note

> *“Understanding an operating system begins when you build one.”*

Just say the word 👍
