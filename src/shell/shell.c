// shell.c - Interactive kernel shell
// Reads commands from keyboard and executes built-in commands

#include "shell.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../proc/process.h"
#include "../kernel/syscall.h"

#define CMD_BUF_SIZE 128
#define MAX_ARGS     8

// ── String helpers (no libc) ──────────────────────────────────────────────────

static int str_len(const char *s) {
    int n = 0; while (s[n]) n++; return n;
}

static int str_cmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static void str_copy(char *dst, const char *src, int max) {
    int i = 0;
    while (i < max-1 && src[i]) { dst[i]=src[i]; i++; }
    dst[i] = 0;
}

// ── Command parser ────────────────────────────────────────────────────────────
// Splits buf into argc/argv by spaces

static int parse_args(char *buf, char *argv[], int max_args) {
    int argc = 0;
    char *p  = buf;
    while (*p && argc < max_args) {
        // skip spaces
        while (*p == ' ') p++;
        if (!*p) break;
        argv[argc++] = p;
        // find end of word
        while (*p && *p != ' ') p++;
        if (*p == ' ') { *p = '\0'; p++; }
    }
    return argc;
}

// ── Built-in demo tasks for spawn command ────────────────────────────────────

static void demo_task(void) {
    uint32_t n = 0;
    while (1) {
        vga_print("[demo] tick ");
        vga_print_int(n++);
        vga_print("\n");
        for (volatile uint32_t i = 0; i < 50000000; i++);
    }
}

// ── Built-in commands ────────────────────────────────────────────────────────

static void cmd_help(void) {
    vga_print("\n");
    vga_print("  Available commands:\n");
    vga_print("  -------------------\n");
    vga_print("  help          Show this help message\n");
    vga_print("  clear         Clear the screen\n");
    vga_print("  tasks         Show all tasks and their states\n");
    vga_print("  spawn         Spawn a demo background task\n");
    vga_print("  exit          Exit current task (shell)\n");
    vga_print("  about         About this OS\n");
    vga_print("\n");
}

static void cmd_clear(void) {
    vga_clear();
    vga_print("MyOS Shell\n");
    vga_print("----------\n");
}

static void cmd_tasks(void) {
    vga_print("\n");
    vga_print("  PID  State    Ticks  Name\n");
    vga_print("  ---  -------  -----  ----\n");

    // Access task table via get_current_task and known layout
    // We expose a helper to iterate tasks
    extern task_t *get_task(int i);   // defined in process.c
    extern int get_max_tasks(void);   // defined in process.c

    for (int i = 0; i < get_max_tasks(); i++) {
        task_t *t = get_task(i);
        if (t->state == TASK_UNUSED) continue;

        vga_print("  ");
        vga_print_int((int)t->pid);
        vga_print("    ");

        const char *state_name;
        switch (t->state) {
            case TASK_READY:   state_name = "READY  "; break;
            case TASK_RUNNING: state_name = "RUNNING"; break;
            case TASK_DEAD:    state_name = "DEAD   "; break;
            default:           state_name = "???????"; break;
        }
        vga_print(state_name);
        vga_print("  ");
        vga_print_int((int)t->ticks);
        vga_print("    ");
        vga_print(t->name);
        vga_print("\n");
    }
    vga_print("\n");
}

static void cmd_about(void) {
    vga_print("\n");
    vga_print("  MyOS - Bare Metal Operating System\n");
    vga_print("  ELE-OPS-411 Class Project 2026\n");
    vga_print("  Team: Isaac Ndoka & Promise Harare\n");
    vga_print("  MUBAS School of Engineering\n");
    vga_print("  Arch: x86 32-bit protected mode\n");
    vga_print("  Features: GDT, IDT, PIC, Timer,\n");
    vga_print("            Keyboard, Scheduler,\n");
    vga_print("            Syscalls, Shell\n");
    vga_print("\n");
}

static void cmd_spawn(void) {
    // Use sys_spawn syscall
    const char *name = "demo";
    int pid;
    __asm__ volatile (
        "mov $2, %%eax\n"   // SYS_SPAWN
        "mov %1, %%ebx\n"   // entry function
        "mov %2, %%ecx\n"   // name
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(pid)
        : "r"(demo_task), "r"(name)
        : "eax", "ebx", "ecx"
    );
    vga_print("  Spawned demo task, PID=");
    vga_print_int(pid);
    vga_print("\n");
}

// ── Shell main loop ───────────────────────────────────────────────────────────

void shell_run(void) {
    char buf[CMD_BUF_SIZE];
    char *argv[MAX_ARGS];

    vga_print("\n");
    vga_print("==========================================\n");
    vga_print("  Welcome to MyOS Shell\n");
    vga_print("  Type 'help' for available commands\n");
    vga_print("==========================================\n");

    while (1) {
        // Print prompt
        vga_print("\nmyos> ");

        // Read a line from keyboard (blocking)
        int len = keyboard_read_line(buf, CMD_BUF_SIZE);
        if (len == 0) continue;  // empty line

        // Parse into arguments
        int argc = parse_args(buf, argv, MAX_ARGS);
        if (argc == 0) continue;

        // Match command
        if      (str_cmp(argv[0], "help")  == 0) cmd_help();
        else if (str_cmp(argv[0], "clear") == 0) cmd_clear();
        else if (str_cmp(argv[0], "tasks") == 0) cmd_tasks();
        else if (str_cmp(argv[0], "spawn") == 0) cmd_spawn();
        else if (str_cmp(argv[0], "about") == 0) cmd_about();
        else if (str_cmp(argv[0], "exit")  == 0) {
            vga_print("Goodbye!\n");
            task_exit();
        } else {
            vga_print("  Unknown command: '");
            vga_print(argv[0]);
            vga_print("' — type 'help'\n");
        }
    }
}