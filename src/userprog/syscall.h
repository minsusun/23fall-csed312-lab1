#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

/* Lab2 - userProcess */
#include "userprog/process.h"

#define CODE_SEGMENTATION_BASE 0x08048000

void syscall_init (void);

/* Lab2 - userProcess */
void load_arguments (int *esp, int *argv, int n);
bool is_valid_vaddr (const void *vaddr);

/* Lab2 - systemCall(userProcess) */
void    syscall_halt (void);
void    syscall_exit (int status);
pid_t   syscall_exec (const char *command);
int     syscall_wait (pid_t pid);

/* Lab2 - systemCall(fileManipulation) */
bool    syscall_create (const char *filename, size_t size);
bool    syscall_remove (const char *filename);
int     syscall_open (const char *filename);
int     syscall_filesize (int fd);
int     syscall_read(int fd, void *buffer, size_t size);
int     syscall_write (int fd, void *buffer, size_t size);
void    syscall_seek (int fd, size_t pos);
size_t  syscall_tell (int fd);
void    syscall_close (int fd);

#endif /* userprog/syscall.h */
