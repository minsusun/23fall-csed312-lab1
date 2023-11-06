#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

/* Lab2 - userProcess */
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "filesys/file.h"
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
      load_arguments (f -> esp, argv, 2);
      f -> eax = syscall_create (argv[0], argv[1]);
      break;

    case SYS_REMOVE:
      load_arguments (f -> esp, argv, 1);
      f -> eax = syscall_remove (argv[0]);
      break;

    case SYS_OPEN:
      load_arguments (f -> esp, argv, 1);
      f -> eax = syscall_open (argv[0]);
      break;
    
    case SYS_FILESIZE:
      load_arguments (f -> esp, argv, 1);
      f -> eax = syscall_filesize (argv[0]);
      break;

    case SYS_READ:
      load_arguments (f -> esp, argv, 3);
      f -> eax = syscall_read (argv[0], argv[1], argv[2]);
      break;

    case SYS_WRITE:
      load_arguments (f -> esp, argv, 3);
      f -> eax = syscall_write ((int)argv[0], (void *)argv[1], argv[2]);
      break;
    
    case SYS_SEEK:
      load_arguments (f -> esp, argv, 2);
      syscall_seek (argv[0], argv[1]);
      break;
    
    case SYS_TELL:
      load_arguments (f -> esp, argv, 1);
      f -> eax = syscall_tell (argv[0]);
      break;
    
    case SYS_CLOSE:
      load_arguments (f -> esp, argv, 1);
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
  thread -> pcb -> exitcode = status;

  printf ("%s: exit(%d)\n", thread -> name, status);
  thread_exit ();
}

pid_t
syscall_exec (const char *command)
{
  return process_execute (command);
}

int
syscall_wait (pid_t pid)
{
  return process_wait (pid);
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
  if (!is_valid_vaddr (filename))
    syscall_exit (-1);
  
  struct file *file = filesys_open (filename);
  if(file == NULL)
    return -1;
  
  struct pcb *pcb = thread_current () -> pcb;
  pcb -> fdtable[pcb -> fdcount] = file;
  
  return pcb -> fdcount++;
}

int
syscall_filesize (int fd)
{
  struct file *file = thread_current () -> pcb -> fdtable[fd];
  return (file == NULL) ? -1 : file_length (file);
}

int
syscall_read (int fd, void *buffer, size_t size)
{
  struct pcb *pcb = thread_current () -> pcb;
  int fdcount = pcb -> fdcount;
  
  if (!is_valid_vaddr(buffer) || fd < 0 || fd >= fdcount)
    syscall_exit (-1);
  
  struct file *file = pcb -> fdtable[fd];
  if (file == NULL)
    syscall_exit (-1);
  
  return file_read (file, buffer, size);

}

int
syscall_write (int fd, void *buffer, size_t size)
{
  struct pcb *pcb = thread_current () -> pcb;
  int fdcount = pcb -> fdcount;

  /* fd=0 cannot be used to write on */
  if (!is_valid_vaddr (buffer) || fd < 1 || fd >= fdcount)
    syscall_exit (-1);
  
  if (fd == 1)
  {
    putbuf (buffer, size);
    return size;
  }
  else
  {
    struct file *file = pcb -> fdtable[fd];

    if (file == NULL)
      syscall_exit (-1);
    
    return file_write (file, buffer, size);
  }
}

void
syscall_seek (int fd, size_t pos)
{
  struct pcb *pcb = thread_current () -> pcb;
  int fdcount = pcb -> fdcount;

  if (fd >= 0 && fd < fdcount)
  {
    struct file *file = pcb -> fdtable[fd];
    if (file != NULL)
      file_seek (file, pos);
  }
}

size_t
syscall_tell (int fd)
{
  struct pcb *pcb = thread_current () -> pcb;
  int fdcount = pcb -> fdcount;

  if (fd < 0 || fd >= fdcount)
    return -1;
  
  struct file *file = pcb -> fdtable[fd];
  if (file == NULL)
    return -1;
  
  return file_tell (file);
}

void
syscall_close (int fd)
{
  struct pcb *pcb = thread_current () -> pcb;
  int fdcount = pcb -> fdcount,i;

  if (fd >= fdcount || fd < 2)
    syscall_exit (-1);
  
  struct file * file = pcb -> fdtable[fd];
  if (file == NULL)
    return;
  
  for (i = fd; i < fdcount; i++)
    pcb -> fdtable[i] = pcb -> fdtable[i + 1];
  pcb -> fdcount --;

  file_close (file);
}