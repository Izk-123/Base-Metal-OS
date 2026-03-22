#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define MAX_TASKS   8
#define STACK_SIZE  8192

typedef enum {
    TASK_UNUSED  = 0,
    TASK_READY   = 1,
    TASK_RUNNING = 2,
    TASK_DEAD    = 3,
} task_state_t;

typedef struct {
    uint32_t     pid;
    char         name[32];
    task_state_t state;
    uint32_t     esp;
    uint8_t      stack[STACK_SIZE];
    uint32_t     ticks;
    void         (*entry)(void);
} task_t;

void     scheduler_init(void);
int      task_create(const char *name, void (*entry)(void));
void     schedule(void);       // called from idle loop
void     scheduler_tick(void); // called from timer IRQ (just sets flag)
void     task_exit(void);
task_t  *get_current_task(void);

#endif