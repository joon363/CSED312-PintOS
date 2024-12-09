#include "vm/frame.h"
#include "threads/synch.h"
#include "threads/palloc.h"
#include "frame.h"

/* Syncronization*/
static struct lock frame_lock;

/* Frame Table*/
static struct list frame_table;

/* Frame Initialization*/
void
frame_init ()
{
    list_init (&frame_table);
    lock_init (&frame_lock);
}

/* Allocate page. in setup_stack in process.c, palloc is replaced by falloc. */
void *
falloc_get_page(enum palloc_flags flags, void *upage) {
    void *kpage = NULL;
    struct fte *e = NULL;

    // Synchronize access
    lock_acquire(&frame_lock);

    // Allocate a kernel page
    kpage = try_page_allocation(flags);
    if (kpage == NULL) {
        lock_release(&frame_lock);
        return NULL;  // Allocation failed even after eviction
    }

    // Initialize and add a frame table entry
    e = create_frame_entry(kpage, upage);
    if (e == NULL) {
        palloc_free_page(kpage);
        lock_release(&frame_lock);
        return NULL;
    }
    list_push_back(&frame_table, &e->elem);

    lock_release(&frame_lock);
    return kpage;
}

/* Allocate a page, handle eviction */
void *
try_page_allocation(enum palloc_flags flags) {
    void *kpage = palloc_get_page(flags);
    if (kpage == NULL) {
        evict_page();  // Evict a page if allocation fails
        kpage = palloc_get_page(flags);
    }
    return kpage;
}

/* Creates a frame table entry */
struct fte *
create_frame_entry(void *kpage, void *upage) {
    struct fte *entry = (struct fte *) malloc(sizeof(struct fte));
    if (entry == NULL) {
        return NULL;  // Allocation failed
    }
    entry->kernal_page = kpage;
    entry->user_page = upage;
    entry->t = thread_current();
    return entry;
}

/* Frees a page */
void
falloc_free_page (void *kpage)
{
  struct fte *e;
  lock_acquire (&frame_lock);

  // find fte
  e = get_fte (kpage);
  if (e == NULL) PANIC ("Failed to free page. No such page found");
  
  // free it
  list_remove (&e->elem);
  palloc_free_page (e);
  pagedir_clear_page (e->t->pagedir, e->user_page);
  lock_release (&frame_lock);
}

struct fte *
get_fte (void* kpage)
{
  struct list_elem *e;
  for (e = list_begin (&frame_table); e != list_end (&frame_table); e = list_next (e))
    if (list_entry (e, struct fte, elem)->kernal_page == kpage)
      return list_entry (e, struct fte, elem);
  return NULL;
}

/* Evict Page */
void evict_page(){
    printf("eviction!");
    return;
}