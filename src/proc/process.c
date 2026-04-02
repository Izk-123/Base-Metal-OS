/**
 * process.c – Round‑robin task scheduler
 * Implements preemptive multitasking using a timer IRQ.
 * The actual context switch is performed by do_switch() in switch.asm.
 */

#include "process.h"
#include "../drivers/vga.h"
#include <stdint.h>

static task_t    tasks[MAX_TASKS];   // task table
static int       current_task = 0;   // index of currently running task
static uint32_t  next_pid     = 1;   // next available process ID

// Assembly function that saves current context and loads a new one
extern void do_switch(uint32_t *old_esp, uint32_t new_esp);

/* Helper: zero out a memory region */
static void mem_set(void *ptr, uint8_t val, uint32_t len) {
    uint8_t *p = (uint8_t *)ptr;
    while (len--) *p++ = val;
}

/* Helper: bounded string copy */
static void str_copy(char *dst, const char *src, uint32_t max) {
    uint32_t i = 0;
    while (i < max - 1 && src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

/**
 * Entry point for every new task.
 * Enables interrupts, calls the task's function, and exits when done.
 */
static void task_entry_point(void) {
    __asm__ volatile ("sti");          // allow preemption
    void (*fn)(void) = tasks[current_task].entry;
    fn();                              // run the task
    task_exit();                       // should never return
}

/**
 * Initialise the task table.
 * Slot 0 is reserved for the idle task (currently unused).
 */
void scheduler_init(void) {
    mem_set(tasks, 0, sizeof(tasks));
    tasks[0].pid   = 0;
    str_copy(tasks[0].name, "idle", 32);
    tasks[0].state = TASK_RUNNING;
    current_task   = 0;
}

/**
 * Create a new kernel thread.
 * Allocates a free slot, sets up its initial stack so that the first
 * context switch into it will jump to task_entry_point().
 * Returns PID on success, -1 if no free slot.
 */
int task_create(const char *name, void (*entry)(void)) {
    int slot = -1;
    for (int i = 1; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_UNUSED) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1;

    task_t *t = &tasks[slot];
    mem_set(t->stack, 0, STACK_SIZE);
    t->pid   = next_pid++;
    t->state = TASK_READY;
    t->ticks = 0;
    t->entry = entry;
    str_copy(t->name, name, 32);

    /* Build the initial stack to match do_switch() expectations:
     * do_switch pops edi, esi, ebx, ebp, then ret.
     * So we push a return address (task_entry_point) and zeroes for the
     * four registers.
     */
    uint32_t *stk = (uint32_t *)((uint32_t)t->stack + STACK_SIZE);
    *--stk = (uint32_t)task_entry_point;   // return address
    *--stk = 0;   // ebp
    *--stk = 0;   // ebx
    *--stk = 0;   // esi
    *--stk = 0;   // edi
    t->esp = (uint32_t)stk;

    return (int)t->pid;
}

/**
 * The scheduler – called from the timer interrupt handler.
 * Implements round‑robin: marks the current task READY, finds the next
 * READY task (skipping the idle slot if other tasks exist), then performs
 * a context switch via do_switch().
 */
void schedule(uint32_t current_esp) {
    (void)current_esp;   // do_switch captures ESP itself

    int old = current_task;

    // If the current task is still RUNNING, mark it READY
    if (tasks[old].state == TASK_RUNNING)
        tasks[old].state = TASK_READY;

    // Round‑robin search for the next READY task (skip slot 0 if possible)
    int next = -1;
    for (int i = 1; i <= MAX_TASKS; i++) {
        int c = (old + i) % MAX_TASKS;
        if (c == 0) continue;          // idle slot – only used if nothing else
        if (tasks[c].state == TASK_READY) {
            next = c;
            break;
        }
    }

    if (next == -1) {
        // No other ready task – stay on current (or go idle)
        if (tasks[old].state == TASK_READY)
            tasks[old].state = TASK_RUNNING;
        return;
    }

    // Switch to the new task
    tasks[next].state = TASK_RUNNING;
    tasks[next].ticks++;
    current_task = next;

    // Perform the context switch – this function will not return on the old stack
    do_switch(&tasks[old].esp, tasks[next].esp);
}

/**
 * Terminate the current task.
 * Marks it DEAD and then halts the CPU (the scheduler will never pick it again).
 */
void task_exit(void) {
    __asm__ volatile ("cli");
    tasks[current_task].state = TASK_DEAD;
    vga_print("\n[task exited]\n");
    __asm__ volatile ("sti");
    while (1) __asm__ volatile ("hlt");
}

task_t *get_current_task(void) {
    return &tasks[current_task];
}