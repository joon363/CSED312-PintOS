#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdio.h>
#include "threads/thread.h"

void syscall_init (void);
void check_vaddr (const void *vaddr);

void sys_halt(void);
void sys_exit(int status);
int sys_exec(const char *cmd_line);
int sys_wait(int pid);
bool sys_create(const char *file, unsigned initial_size);
bool sys_remove(const char *file);
int sys_open(const char *file);
int sys_filesize(int fd);
int sys_read(int fd, void *buffer, unsigned size);
int sys_write(int fd, const void *buffer, unsigned size);
void sys_seek(int fd, unsigned position);
unsigned sys_tell(int fd);
void sys_close(int fd);
struct file *fd_to_file(int fd);
int keyboard_read(void *buffer, unsigned size);

typedef int mapid_t;
mapid_t sys_mmap(int fd, void *addr);
mapid_t new_mmapid(struct thread * t);
void sys_munmap(mapid_t mapping);
struct mmap_file *get_mmf(struct thread *t, mapid_t mapping);
#endif /* userprog/syscall.h */
