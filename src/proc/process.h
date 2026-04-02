#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define MAX_TASKS   8       // maximum number of concurrent tasks
#define STACK_SIZE  8192    // each task gets 8 KB kernel stack

typedef enum {
    TASK_UNUSED  = 0,
    TASK_READY   = 1,
    TASK_RUNNING = 2,
    TASK_DEAD    = 3,
} task_state_t;

/**
 * Process Control Block (PCB)
 * Stores all information needed for a task.
 */
typedef struct {
    uint32_t     pid;                // process ID
    char         name[32];           // human‑readable name
    task_state_t state;              // current state (READY, RUNNING, …)
    uint32_t     esp;                // saved stack pointer (when not running)
    uint8_t      stack[STACK_SIZE];  // private kernel stack
    uint32_t     ticks;              // number of times scheduled (statistics)
    void         (*entry)(void);     // task entry function
} task_t;

void     scheduler_init(void);
int      task_create(const char *name, void (*entry)(void));
void     schedule(uint32_t current_esp);   // called from timer IRQ
void     task_exit(void);
task_t  *get_current_task(void);

#endif