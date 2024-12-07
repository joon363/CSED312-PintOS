#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"
#include "threads/malloc.h"


/* Frame Table Entry*/
struct fte
{
    void *kernal_page;
    struct list_elem elem;

    void *user_page;
    struct thread *t;
};

/* Frame Table functions*/
void frame_init (void);
void *falloc_get_page(enum palloc_flags flags, void *upage);
void  falloc_free_page (void *);
void *try_page_allocation(enum palloc_flags flags);
struct fte *create_frame_entry(void *kpage, void *upage);
void evict_page();
struct fte *get_fte (void* );


#endif