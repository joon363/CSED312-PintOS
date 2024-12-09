#include "userprog/syscall.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/off_t.h"
#include "devices/block.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "threads/synch.h"
#include "vm/spt.h"

struct lock file_lock;
struct file 
{
  struct inode *inode;        /* File's inode. */
  off_t pos;                  /* Current position. */
  bool deny_write;            /* Has file_deny_write() been called? */
};

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init (&file_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Use is_user_vaddr from src/threads/vaddr.h */
void
check_vaddr (const void *vaddr)
{
  if (!is_user_vaddr (vaddr)) {
    //printf("not user vaddr");
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
  uint32_t syscall_number =  *(uint32_t *)esp;
  uint32_t argv[3];
  argv[0] = *(uint32_t *)(esp + 4);
  argv[1] = *(uint32_t *)(esp + 8);
  argv[2] = *(uint32_t *)(esp + 12);
  
  check_vaddr(esp + 4);
  check_vaddr(esp + 8);
  check_vaddr(esp + 12);

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
      if(file==NULL) {
        //printf("SYS_EXEC file null");
        sys_exit(-1);
      }
      f->eax = sys_exec(file);
      break;
    }

    case SYS_WAIT: {                  // syscall1: pid_t pid
      int pid = (int)argv[0];
      f->eax = sys_wait(pid);
      break;
    }

    case SYS_CREATE: {                // syscall2: const char *file, unsigned initial_size
      const char *file = (const char *)argv[0];
      unsigned initial_size = (unsigned)argv[1];
      if(file==NULL) {
        //printf("SYS_CREATE file null");
        sys_exit(-1);
      }
      f->eax = sys_create(file, initial_size);
      break;
    }

    case SYS_REMOVE: {                // syscall1: const char *file
      const char *file = (const char *)argv[0];
      if(file==NULL) {
        //printf("SYS_REMOVE file null");
        sys_exit(-1);
      }
      f->eax = sys_remove(file);
      break;
    }

    case SYS_OPEN: {                  // syscall1: const char *file
      const char *file = (const char *)argv[0];
      if(file==NULL) {
        //printf("SYS_OPEN file null");
        sys_exit(-1);
      }
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
      if(buffer==NULL) {
        //printf("SYS_READ buffer null");
        sys_exit(-1);
      }
      f->eax = sys_read(fd, buffer, size);
      break;
    }

    case SYS_WRITE: {                 // syscall3: int fd, const void *buffer, unsigned size
      int fd = (int)argv[0];
      const void *buffer = (const void *)argv[1];
      unsigned size = (unsigned)argv[2];
      if(buffer==NULL) {
        //printf("sys_write buffer null");
        sys_exit(-1);
      }
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

    case SYS_MMAP: {                 // syscall2: int fd, void * addr
      int fd = (int)argv[0];
      void * addr = (void *)argv[1];
      f->eax = sys_mmap(fd,addr);
      break;
    }

    case SYS_MUNMAP: {                 // syscall1: mapid_t mapping
      mapid_t mapping = (mapid_t)argv[0];
      sys_munmap(mapping);
      break;
    }
  }
}

/* syscall logic implementations. */

/* call shutdown_power_off() from src/devices/shutdown.c. */
void
sys_halt (void)
{
  shutdown_power_off();
}

/* set exit code, call thread_exit(). */
void
sys_exit (int status)
{
  /* Problem 1: Process Termination Messages */
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current() -> exit_code = status;
  thread_exit();
}

/* call process_execute from userprog/process.c. */
int
sys_exec (const char *cmd_line)
{
  /* process_execute returns -1 if program fails for some reason. */
  lock_acquire (&file_lock);
  int pid = process_execute (cmd_line);
  lock_release (&file_lock);
  return pid;
}

/* Waits for a child process pid and retrieves the child's exit status. 
- If pid is still alive, waits until it terminates. Then, returns the status that pid passed to `exit`.
- If pid did not call `exit()`, but was terminated by the kernel (e.g. killed due to an exception), `wait(pid)` must return -1.*/
int 
sys_wait(int pid)
{
  return process_wait (pid);
}

/* Return file pointer according to fd number. */
struct file *
fd_to_file(int fd){
  if(fd<0 || fd>128) {
    //printf("fd_to_file fd error null");
    sys_exit(-1);
  }
  struct file * f= thread_current()->fd[fd];
  if(f==NULL) {
    //printf("fd_to_file file null");
    sys_exit(-1);
  }
  return f;
}

/* Creates a new file called file initially initial_size bytes in size.*/
bool
sys_create(const char *file, unsigned initial_size)
{
  lock_acquire (&file_lock);
  bool res = filesys_create (file, initial_size);
  lock_release (&file_lock);
  return res;
}

/* Deletes the file called file.*/
bool
sys_remove (const char *file)
{
  lock_acquire (&file_lock);
  bool res= filesys_remove (file);
  lock_release (&file_lock);
  return res;
}

/* Opens the file called file.
  - Returns a nonnegative integer handle called a "file descriptor" (fd), 
    or -1 if the file could not be opened.
  - File descriptors numbered 0 and 1 are reserved for the console: 
    fd 0 (`STDIN_FILENO`) is standard input, fd 1 (`STDOUT_FILENO`) is standard output.
  - The `open` system call will never return either of these file descriptors, 
    which are valid as system call arguments only as explicitly described below.
    - When a single file is opened more than once, whether by a single process or 
      different processes, each `open` returns a new file descriptor.
    - Different file descriptors for a single file are closed independently 
      in separate calls to `close` and they do not share a file position.
  - Each process has an independent set of file descriptors. 
    File descriptors are not inherited by child processes. */
int
sys_open (const char *file)
{
  lock_acquire (&file_lock);
  struct file *return_file = filesys_open (file);
  lock_release(&file_lock);
  if (return_file == NULL) {
    return -1;
  }
  for (int i=2; i<128; i++)
  {
    if (thread_current()->fd[i] == NULL)
    {
      if(thread_current()->executing_file !=NULL && strcmp(thread_current()->name, file)==0){
        lock_acquire (&file_lock);
        file_deny_write(return_file);
        lock_release(&file_lock);   
      }
      thread_current()->fd[i] = return_file;
      return i;
    }
  }
  return -1;
}

/* Returns the size, in bytes, of the file open as fd. */
int
sys_filesize (int fd)
{
  struct file *f = fd_to_file(fd);
  lock_acquire (&file_lock);
  int length = file_length (f);
  lock_release (&file_lock);
  return length;
}

/* Reads from the keyboard using input_getc(). */
int
keyboard_read(void *buffer, unsigned size){
  unsigned count;
    char key;
    for (count=0; count<size; count++)
    {
      key = input_getc();
      ((char*)buffer)[count]=key;
      if (key == '\0') break;
      else buffer+=1;
    }
    return count;
  }

/* Reads size bytes from the file open as fd into buffer. */
int
sys_read (int fd, void *buffer, unsigned size)
{
  check_vaddr (buffer);
  check_vaddr (buffer+size-1);
  switch (fd)
  {
  case 0:   // STDIN
    return keyboard_read(buffer, size);
  case 1:   // STDOUT
    return 0;
  default:  // File
    {
      struct file *f = fd_to_file(fd);
      lock_acquire (&file_lock);
      int bytes_read = file_read (f, buffer, size);
      lock_release (&file_lock);
      return bytes_read;
    }
  }
}

/* Writes size bytes from buffer to the open file fd.
  - Returns the number of bytes actually written, 
    which may be less than size if some bytes could not be written.
  - The expected behavior is to write as many bytes as possible up to end-of-file 
    and return the actual number written, or 0 if no bytes could be written at all.
  - Fd 1 writes to the console.  */
int
sys_write (int fd, const void *buffer, unsigned size)
{
  if (fd == 1){
    lock_acquire (&file_lock);
    putbuf (buffer, size);
    lock_release (&file_lock);
    return size;
  }
  else{
    struct file *f = fd_to_file(fd);
    lock_acquire (&file_lock);
    int res =  file_write (f, buffer, size);
    lock_release (&file_lock);
    return res;
  }
}

/* Changes the next byte to be read or written in open file fd to position, 
   expressed in bytes from the beginning of the file. (Thus, a position of 0 is the file's start.)
  - A seek past the current end of a file is not an error. 
    A later read obtains 0 bytes, indicating end of file. 
    A later write extends the file, filling any unwritten gap with zeros. 
    (However, in Pintos files have a fixed length until project 4 is complete, so writes past end of file will return an error.)
  - These semantics are implemented in the file system and 
    do not require any special effort in system call implementation. */
void
sys_seek (int fd, unsigned position)
{
  struct file *f = fd_to_file(fd);
  lock_acquire (&file_lock);
  file_seek (f, position);
  lock_release (&file_lock);
  return;
}

/* Returns the position of the next byte to be read or written in open file fd, 
   expressed in bytes from the beginning of the file. */
unsigned
sys_tell (int fd)
{
  struct file *f = fd_to_file(fd);
  lock_acquire (&file_lock);
  unsigned res = file_tell(f);
  lock_release (&file_lock);
  return res;
}

/* Closes file descriptor fd.
- Exiting or terminating a process implicitly closes all its open file descriptors, 
  as if by calling this function for each one. */
void
sys_close (int fd)
{
  struct file *f = fd_to_file(fd);
  lock_acquire (&file_lock);
  f = NULL;
  file_close (f);
  lock_release (&file_lock);
  return;
}

/* Maps the file open as fd into the process's virtual address space. 
 The entire file is mapped into consecutive virtual pages 
 starting at addr. */
mapid_t
sys_mmap (int fd, void *addr)
{
  // mmap-misalign, mmap-null test case
  if (addr == NULL ||(int) addr % PGSIZE != 0) {
	  return -1;
  }

  struct file *f = fd_to_file(fd);
  lock_acquire (&file_lock);
  struct file *opened_file = file_reopen (f);
  if(opened_file==NULL) {
	  lock_release (&file_lock);
	  return -1;
  }

  struct mmap_file *mmf;
  mmf = (struct mmap_file *) malloc(sizeof *mmf);
  mmf->id = new_mmapid(thread_current());
  mmf->file = opened_file;
  mmf->upage = addr;

  off_t ofs;
  int size = file_length(opened_file);
  struct hash *spt = &thread_current()->spt;

  /* check if all pages do not exist.*/
  for (ofs = 0; ofs < size; ofs += PGSIZE){
    if (get_spte(spt, addr + ofs)) {
	    lock_release (&file_lock);
      return -1;
    }
  }

  /* map each page */
  for (ofs = 0; ofs < size; ofs += PGSIZE) {
      uint32_t read_bytes = ofs + PGSIZE < size ? PGSIZE : size - ofs;
      init_file_spte(spt, addr, opened_file, ofs, read_bytes, PGSIZE - read_bytes, true);
      addr += PGSIZE;
  }

  /* add to list */
  list_push_back(&thread_current()->mmap_list, &mmf->mmap_file_elem);
  lock_release (&file_lock);
  return mmf->id;
}

/* allocate mapid */
mapid_t 
new_mmapid(struct thread* t){
  mapid_t mapid=1;
  if (! list_empty(&t->mmap_list)) {
    mapid = list_entry(list_back(&t->mmap_list), struct mmap_file, mmap_file_elem)->id + 1;
  }
  return mapid;
}

/* Unmaps the mapping designated by mapping, which must be a 
mapping ID returned by a previous call to mmap by the same process 
that has not yet been unmapped. */
void
sys_munmap (mapid_t mapping)
{
  struct thread *t = thread_current();
  struct mmap_file *mmf = get_mmf(t, mapping);
  if(mmf==NULL) return; // invalid mapping id
  lock_acquire (&file_lock);

  off_t ofs;
  void *upage;
  for (ofs = 0; ofs < file_length(mmf->file); ofs += PGSIZE) {
    upage = mmf->upage + ofs;
    struct spte *entry = get_spte(&t->spt, upage);

    // dirty page check
    if (pagedir_is_dirty(t->pagedir, upage)) {
        void *kpage = pagedir_get_page(t->pagedir, upage);
        file_write_at(entry->file, kpage, entry->read_bytes, entry->file_offset);
    }

    // remove page
    page_delete(&t->spt, entry);
  }

  // remove from list
  list_remove(&mmf->mmap_file_elem);
  lock_release (&file_lock);
  return;
}

/* find mmap file based on mapping id. */
struct mmap_file *
get_mmf(struct thread *t, mapid_t mapping) {
  struct list_elem *e;
  struct mmap_file *res_mmf;
  if (!list_empty(&t->mmap_list)) {
    for(e = list_begin(&t->mmap_list); e != list_end(&t->mmap_list); e = list_next(e))
    {
      res_mmf = list_entry(e, struct mmap_file, mmap_file_elem);
      if(res_mmf->id == mapping) {
        return res_mmf;
      }
    }
  }
  return NULL;
}