#include "process.h"
#include "../drivers/vga.h"
#include <stdint.h>

static task_t    tasks[MAX_TASKS];
static int       current_task    = 0;
static uint32_t  next_pid        = 1;
static volatile int need_switch  = 0;  // flag set by timer, read by idle loop
static uint32_t  tick_counter    = 0;

extern void do_switch(uint32_t *old_esp, uint32_t new_esp);

static void mem_set(void *ptr, uint8_t val, uint32_t len) {
    uint8_t *p = (uint8_t *)ptr;
    while (len--) *p++ = val;
}

static void str_copy(char *dst, const char *src, uint32_t max) {
    uint32_t i = 0;
    while (i < max - 1 && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

static void task_entry_point(void) {
    __asm__ volatile ("sti");
    void (*fn)(void) = tasks[current_task].entry;
    fn();
    task_exit();
}

void scheduler_init(void) {
    mem_set(tasks, 0, sizeof(tasks));
    tasks[0].pid   = 0;
    str_copy(tasks[0].name, "idle", 32);
    tasks[0].state = TASK_RUNNING;
    current_task   = 0;
    need_switch    = 0;
    tick_counter   = 0;
}

int task_create(const char *name, void (*entry)(void)) {
    int slot = -1;
    for (int i = 1; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_UNUSED) { slot = i; break; }
    }
    if (slot == -1) return -1;

    task_t *t = &tasks[slot];
    mem_set(t->stack, 0, STACK_SIZE);  // zero only the stack
    t->pid   = next_pid++;
    t->state = TASK_READY;
    t->ticks = 0;
    t->entry = entry;
    str_copy(t->name, name, 32);

    // Build stack: do_switch does pop edi,esi,ebx,ebp then ret
    uint32_t *stk = (uint32_t *)((uint32_t)t->stack + STACK_SIZE);
    *--stk = (uint32_t)task_entry_point; // ret target
    *--stk = 0;  // ebp
    *--stk = 0;  // ebx
    *--stk = 0;  // esi
    *--stk = 0;  // edi  <- esp points here

    t->esp = (uint32_t)stk;
    return (int)t->pid;
}

// Called from timer IRQ — only sets the flag, NO switching here
void scheduler_tick(void) {
    tick_counter++;
    if (tick_counter >= 10) {
        tick_counter  = 0;
        need_switch   = 1;  // signal the idle loop to switch
    }
}

// Called from the idle loop OUTSIDE any IRQ handler
// This is where the actual context switch happens safely
void schedule(void) {
    if (!need_switch) return;
    need_switch = 0;

    int old = current_task;

    if (tasks[old].state == TASK_RUNNING)
        tasks[old].state = TASK_READY;

    // Find next READY task — skip slot 0 (idle) unless nothing else
    int next = -1;
    for (int i = 1; i <= MAX_TASKS; i++) {
        int c = (old + i) % MAX_TASKS;
        if (c == 0) continue;  // skip idle slot
        if (tasks[c].state == TASK_READY) {
            next = c;
            break;
        }
    }

    if (next == -1) {
        // nothing ready — stay on current or go idle
        tasks[old].state = TASK_RUNNING;
        return;
    }

    tasks[next].state = TASK_RUNNING;
    tasks[next].ticks++;
    current_task = next;

    do_switch(&tasks[old].esp, tasks[next].esp);
}

void task_exit(void) {
    __asm__ volatile ("cli");
    tasks[current_task].state = TASK_DEAD;
    vga_print("\n[task exited]\n");
    __asm__ volatile ("sti");
    // Force a reschedule
    need_switch = 1;
    while (1) schedule();
}

task_t *get_current_task(void) { return &tasks[current_task]; }