#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "filesys/file.h"
#include "filesys/off_t.h"
#include "userprog/syscall.h"

enum spt_status{
    ZERO_PAGE,
    FRAME_PAGE,
    FILE_PAGE,
    SWAP_PAGE
};

struct spte
{
    void *upage;
    void *kpage;
    enum spt_status status;

    struct hash_elem hash_elem;

    struct file *file;
    off_t file_offset;
    uint32_t read_bytes, zero_bytes;
    bool writable;
    int swap_id;
};

void init_spt (struct hash *);
void destroy_spt (struct hash *);
void init_frame_spte (struct hash *, void *, void *);
struct spte *init_file_spte (struct hash *, void *, struct file *, off_t, uint32_t, uint32_t, bool);
void init_zero_spte (struct hash *spt, void *upage);
bool load_page (struct hash *, void *);
struct spte *get_spte (struct hash *, void *);
void page_delete (struct hash *spt, struct spte *entry);

#endif