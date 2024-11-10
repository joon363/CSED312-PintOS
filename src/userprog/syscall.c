#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Use is_user_vaddr from src/threads/vaddr.h */
void
check_vaddr (const void *vaddr)
{
  if (!is_user_vaddr (vaddr) || vaddr <= 0) {
    sys_exit(-1);
  }
}

/* syscall handler implementation. */
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  /* exit when address is invalid. */
  check_vaddr(f->esp);
  void *esp = f->esp;

  /* getting syscall number and arguments from frame. */
  uint32_t syscall_number =  *(uint32_t *)esp
  uint32_t argv[3];
  argv[0] = *(uint32_t *)(sp + 4);
  argv[1] = *(uint32_t *)(sp + 8);
  argv[2] = *(uint32_t *)(sp + 12);

  /* call syscall functions according to syscall number. */
  switch (syscall_number) {
    case SYS_HALT:                    // syscall0
      sys_halt();
      break;

    case SYS_EXIT: {                  // syscall1: int status
      int status = (int)argv[0];
      sys_exit(status);
      break;
    }

    case SYS_EXEC: {                  // syscall1: const char *file
      const char *file = (const char *)argv[0];
      f->eax = sys_exec(file);
      break;
    }

    case SYS_WAIT: {                  // syscall1: pid_t pid
      pid_t pid = (pid_t)argv[0];
      f->eax = sys_wait(pid);
      break;
    }

    case SYS_CREATE: {                // syscall2: const char *file, unsigned initial_size
      const char *file = (const char *)argv[0];
      unsigned initial_size = (unsigned)argv[1];
      f->eax = sys_create(file, initial_size);
      break;
    }

    case SYS_REMOVE: {                // syscall1: const char *file
      const char *file = (const char *)argv[0];
      f->eax = sys_remove(file);
      break;
    }

    case SYS_OPEN: {                  // syscall1: const char *file
      const char *file = (const char *)argv[0];
      f->eax = sys_open(file);
      break;
    }

    case SYS_FILESIZE: {              // syscall1: int fd
      int fd = (int)argv[0];
      f->eax = sys_filesize(fd);
      break;
    }

    case SYS_READ: {                  // syscall3: int fd, void *buffer, unsigned size
      int fd = (int)argv[0];
      void *buffer = (void *)argv[1];
      unsigned size = (unsigned)argv[2];
      f->eax = sys_read(fd, buffer, size);
      break;
    }

    case SYS_WRITE: {                 // syscall3: int fd, const void *buffer, unsigned size
      int fd = (int)argv[0];
      const void *buffer = (const void *)argv[1];
      unsigned size = (unsigned)argv[2];
      f->eax = sys_write(fd, buffer, size);
      break;
    }

    case SYS_SEEK: {                  // syscall2: int fd, unsigned position
      int fd = (int)argv[0];
      unsigned position = (unsigned)argv[1];
      sys_seek(fd, position);
      break;
    }

    case SYS_TELL: {                  // syscall1: int fd
      int fd = (int)argv[0];
      f->eax = sys_tell(fd);
      break;
    }

    case SYS_CLOSE: {                 // syscall1: int fd
      int fd = (int)argv[0];
      sys_close(fd);
      break;
    }
  }
}

