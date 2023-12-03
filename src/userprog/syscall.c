#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

/* Lab2 - userProcess */
#include <string.h>
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

/* Lab2 - fileSystem */
#include "threads/synch.h"

struct lock file_lock;

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  /* Lab2 - fileSystem */
  lock_init (&file_lock);
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

  /* lab3 - stack growth */
  thread_current () -> esp = f -> esp;

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
    
    /* lab3 - MMF */
    case SYS_MMAP:
      get_argument (f -> esp, argv, 1);
      syscall_mmap ((int) argv[0], (void *)argv[1]);
      break;
    
    case SYS_MUNMAP:
      get_argument (f -> esp, argv, 2);
      syscall_munmap ((int) argv[0]);
      break;

    default:
      /* temporary handling */
      printf ("default syscall handling!!\n");
      // thread_exit ();
  }
}

/* Lab2 - userProcess */
/* Store n arguments to the stack frame */
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

/* Validate the given virtual address is in user space */
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

  /* Lab2 - fileSystem */
  /* sync load & wait process */
  // sema_up (&(thread -> pcb -> load));
  // sema_up (&(thread -> pcb -> wait));

  if (!thread -> pcb -> isloaded)
    sema_up (&(thread -> pcb -> load));

  /* termination message */
  printf ("%s: exit(%d)\n", thread -> name, status);
  thread_exit ();
}

pid_t
syscall_exec (const char *cmd_line)
{
  pid_t pid = process_execute (cmd_line);
  struct pcb *pcb = thread_get_child_pcb (pid);
  
  if (pid == -1 || !pcb -> isloaded)
    return -1;
  
  return pid;

  // return process_execute (cmd_line);
}

int
syscall_wait (pid_t pid)
{
  return process_wait (pid);
}

bool
syscall_create (const char *file, unsigned initial_size)
{
  if (!is_valid_vaddr (file))
    syscall_exit (-1);
  
  return filesys_create (file, initial_size);
}

bool
syscall_remove (const char *file)
{
  if (!is_valid_vaddr (file))
    syscall_exit (-1);
  
  return filesys_remove (file);
}

int
syscall_open (const char *file)
{
  /* Lab2 - fileSystem */
  lock_acquire (&file_lock);

  if (!is_valid_vaddr (file))
  {
    lock_release (&file_lock);
    syscall_exit (-1);
  }
  
  struct file *file_ = filesys_open (file);
  
  if(file_ == NULL)
  {
    lock_release (&file_lock);
    return -1;
  }
  
  struct thread *thread = thread_current ();
  struct pcb *pcb = thread -> pcb;

  /* ROX, Deny writes to executables */
  if (pcb -> _file != NULL && strcmp (thread -> name, file) == 0)
    file_deny_write (file_);

  pcb -> fdtable[pcb -> fdcount] = file_;

  lock_release (&file_lock);
  
  return pcb -> fdcount++;
}

int
syscall_filesize (int fd)
{
  struct file *file = thread_current () -> pcb -> fdtable[fd];
  return (file == NULL) ? -1 : file_length (file);
}

int
syscall_read (int fd, void *buffer, unsigned size)
{
  struct pcb *pcb = thread_current () -> pcb;
  int fdcount = pcb -> fdcount;
  
  if (!is_valid_vaddr(buffer) || fd < 0 || fd >= fdcount)
    syscall_exit (-1);
  
  struct file *file = pcb -> fdtable[fd];
  if (file == NULL)
    syscall_exit (-1);
  
  /* Lab2 - fileSystem */
  lock_acquire (&file_lock);
  
  int read_size = file_read (file, buffer, size);
  
  lock_release (&file_lock);
  
  return read_size;

  //return file_read (file, buffer, size);
}

int
syscall_write (int fd, void *buffer, unsigned size)
{
  struct pcb *pcb = thread_current () -> pcb;
  int fdcount = pcb -> fdcount;

  /* fd=0(stdin) cannot be used to write on */
  if (!is_valid_vaddr (buffer) || fd < 1 || fd >= fdcount)
    syscall_exit (-1);
  
  /* stdout */
  if (fd == 1)
  {
    lock_acquire (&file_lock);
    
    putbuf (buffer, size);
    
    lock_release(&file_lock);
    
    return size;
  }
  else
  {
    struct file *file = pcb -> fdtable[fd];

    if (file == NULL)
      syscall_exit (-1);
    
    /* Lab2 - fileSystem */
    lock_acquire (&file_lock);
    
    int write_size = file_write (file, buffer, size);
    
    lock_release (&file_lock);
    
    return write_size;
    
    //return file_write (file, buffer, size);
  }
}

void
syscall_seek (int fd, unsigned position)
{
  struct pcb *pcb = thread_current () -> pcb;
  int fdcount = pcb -> fdcount;

  if (fd >= 0 && fd < fdcount)
  {
    struct file *file = pcb -> fdtable[fd];
    
    if (file != NULL)
      file_seek (file, position);
  }
}

unsigned
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
  
  /* Reorder file descriptor table */
  for (i = fd; i < fdcount; i++)
    pcb -> fdtable[i] = pcb -> fdtable[i + 1];
  
  pcb -> fdcount --;

  file_close (file);
}

/* lab3 - MMF */
int
syscall_mmap (int fd, void *vaddr)
{
  
}

int
syscall_munmap (int mapid)
{

}