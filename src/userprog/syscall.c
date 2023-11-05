#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

/* Lab2 - userProcess */
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
/* Lab2 - userProcess */
// syscall_handler (struct intr_frame *f UNUSED) 
syscall_handler (struct intr_frame *f)
{
  /* Lab2 - userProcess */
  // printf ("system call!\n");
  // thread_exit ();

  if (!is_valid_vaddr (f -> esp))
    syscall_exit (-1);

  int argv[3];

  switch (*(int *)(f -> esp))
  {
    case SYS_HALT:
      syscall_halt ();
      break;
    
    case SYS_EXIT:
      load_arguments (f -> esp, argv, 1);
      syscall_exit (argv[0]);
      break;
    
    case SYS_EXEC:
      load_arguments (f -> esp, argv, 1);
      f -> eax = syscall_exec (argv[0]);
      break;
    
    case SYS_WAIT:
      load_arguments (f -> esp, argv, 1);
      f -> eax = syscall_wait (argv[0]);
      break;

    case SYS_CREATE:
      laod_arguments (f -> esp, argv, 2);
      f -> eax = syscall_create (argv[0], argv[1]);
      break;

    case SYS_REMOVE:
      laod_arguments (f -> esp, argv, 1);
      f -> eax = syscall_remove (argv[0]);
      break;

    case SYS_OPEN:
      laod_arguments (f -> esp, argv, 1);
      f -> eax = syscall_open (argv[0]);
      break;
    
    case SYS_FILESIZE:
      laod_arguments (f -> esp, argv, 1);
      f -> eax = syscall_filesize (argv[0]);
      break;

    case SYS_READ:
      laod_arguments (f -> esp, argv, 3);
      f -> eax = syscall_read (argv[0], argv[1], argv[3]);
      break;

    case SYS_WRITE:
      load_arguments (f -> esp, argv, 3);
      f -> eax = syscall_write ((int)argv[0], (void *)argv[1], argv[2]);
      break;
    
    case SYS_SEEK:
      laod_arguments (f -> esp, argv, 2);
      syscall_seek (argv[0], argv[1]);
      break;
    
    case SYS_TELL:
      laod_arguments (f -> esp, argv, 1);
      f -> eax = syscall_tell (argv[0]);
      break;
    
    case SYS_CLOSE:
      laod_arguments (f -> esp, argv, 1);
      syscall_close (argv[0]);
      break;

    default:
      /* temporary handling */
      printf ("default syscall handling!!\n");
      // thread_exit ();
  }
}

/* Lab2 - userProcess */

void
load_arguments (int *esp, int *argv, int n)
{
  int i;
  for (i = 0; i < n; i++)
  {
    if (!is_valid_vaddr (esp + 1 + i))
      syscall_exit(-1);
    argv[i] = *(esp + 1 + i);
  }
}

bool
is_valid_vaddr (void *vaddr)
{
  return is_user_vaddr (vaddr) && vaddr >= CODE_SEGMENTATION_BASE && vaddr != NULL;
}

void
syscall_halt(void)
{
  shutdown_power_off ();
}

void
syscall_exit(int status)
{
  struct thread *thread = thread_current ();

  printf ("%s: exit(%d)\n", thread -> name, status);
  thread_exit ();
}

pid_t
sys_exec (const char *command)
{

}

int
sys_wait (pid_t pid)
{

}

bool
syscall_create (const char *filename, size_t size)
{
  if (!is_valid_vaddr (filename))
    syscall_exit (-1);
  return filesys_create (filename, size);
}

bool
syscall_remove (const char *filename)
{
  if (!is_valid_vaddr (filename))
    syscall_exit (-1);
  return filesys_remove (filename);
}

int
syscall_open (const char *filename)
{

}

int
syscall_filesize (int fd)
{

}

int
syscall_read (int fd, void *buffer, size_t size)
{

}

int
syscall_write (int fd, void *buffer, size_t size)
{
  if (!is_valid_vaddr (buffer))
    syscall_exit (-1);
  putbuf (buffer, size);
  return size;
}

void
syscall_seek (int fd, size_t pos)
{

}

size_t
syscall_tell (int fd)
{

}

void
syscall_close (int fd)
{

}