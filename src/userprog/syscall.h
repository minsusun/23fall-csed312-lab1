#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

/* Lab2 - userProcess */
#include "userprog/process.h"

void syscall_init (void);

/* Lab2 - userProcess */
void load_arguments (int *esp, int *argv, int n);
void syscall_halt (void);
void syscall_exit (int status);
pid_t syscall_exec (const char *command);
int syscall_wait (pid_t pid);

#endif /* userprog/syscall.h */
