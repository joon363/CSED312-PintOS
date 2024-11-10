#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
void set_args_stack (char **argv, int argc, void **esp);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
int parse_args(char **argv, char* args);

#endif /* userprog/process.h */
