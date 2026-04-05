// syscall.c - System call handler
// Triggered by: int 0x80
// EAX = syscall number
// EBX = first argument
// ECX = second argument

#include "syscall.h"
#include "idt.h"
#include "../drivers/vga.h"
#include "../proc/process.h"

// The registers struct from idt.h gives us access to EAX, EBX, ECX
// when the syscall interrupt fires

static void syscall_handler(struct registers *regs) {
    uint32_t syscall_num = regs->eax;
    uint32_t arg1        = regs->ebx;
    uint32_t arg2        = regs->ecx;

    switch (syscall_num) {

        case SYS_WRITE:
            // EBX = pointer to null-terminated string
            if (arg1) {
                vga_print((const char *)arg1);
            }
            regs->eax = 0; // return 0 = success
            break;

        case SYS_SPAWN: {
            // EBX = function pointer (entry)
            // ECX = pointer to name string
            void (*entry)(void) = (void (*)(void))arg1;
            const char *name    = (const char *)arg2;
            if (!name) name = "task";
            int pid = task_create(name, entry);
            regs->eax = (uint32_t)pid; // return PID or -1
            break;
        }

        case SYS_EXIT:
            // Terminate current task
            task_exit();
            break; // never reached

        default:
            vga_print("[syscall] unknown syscall: ");
            vga_print_int((int)syscall_num);
            vga_print("\n");
            regs->eax = (uint32_t)-1; // return -1 = error
            break;
    }
}

void syscall_init(void) {
    // Register syscall handler at interrupt vector 0x80 (128)
    idt_set_handler(0x80, syscall_handler);
    vga_print("[Syscall] int 0x80 ready\n");
}