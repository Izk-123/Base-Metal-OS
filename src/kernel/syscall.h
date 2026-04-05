#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

// Syscall numbers — put in EAX before int 0x80
#define SYS_WRITE  1   // write string to console: EBX = char*
#define SYS_SPAWN  2   // create task: EBX = fn ptr, ECX = name ptr
#define SYS_EXIT   3   // terminate current task

void syscall_init(void);

#endif