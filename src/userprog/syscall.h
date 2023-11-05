#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

/* Lab2 - userProcess */
#include "userprog/process.h"

void syscall_init (void);

/* Lab2 - userProcess */
void load_arguments (int *esp, int *argv, int n);

/* Lab2 - systemCall(userProcess) */
void    syscall_halt (void);
void    syscall_exit (int status);
pid_t   syscall_exec (const char *command);
int     syscall_wait (pid_t pid);

/* Lab2 - systemCall(fileManipulation) */
bool    syscall_create (const char *file, size_t size);
bool    syscall_remove (const char *file);
int     syscall_open (const char *file);
int     syscall_filesize (int fd);
int     syscall_read(int fd, void *buffer, size_t size);
int     syscall_write (int fd, void *buffer, size_t size);
void    syscall_seek (int fd, size_t pos);
size_t  syscall_tell (int fd);
void    syscall_close (int fd);

#endif /* userprog/syscall.h */
