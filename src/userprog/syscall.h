#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

/* Lab2 - userProcess */
#include "userprog/process.h"

#define CODE_SEGMENTATION_BASE 0x08048000

void syscall_init (void);

/* Lab2 - userProcess */
void        load_arguments (int *esp, int *argv, int n);
bool        is_valid_vaddr (void *vaddr);

/* Lab2 - systemCall(userProcess) */
void        syscall_halt (void);
void        syscall_exit (int status);
pid_t       syscall_exec (const char *cmd_line);
int         syscall_wait (pid_t pid);

/* Lab2 - systemCall(fileManipulation) */
bool        syscall_create (const char *file, unsigned initial_size);
bool        syscall_remove (const char *file);
int         syscall_open (const char *file);
int         syscall_filesize (int fd);
int         syscall_read(int fd, void *buffer, unsigned size);
int         syscall_write (int fd, void *buffer, unsigned size);
void        syscall_seek (int fd, unsigned position);
unsigned    syscall_tell (int fd);
void        syscall_close (int fd);

#endif /* userprog/syscall.h */
