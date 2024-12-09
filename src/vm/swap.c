#include "vm/swap.h"
#include <bitmap.h>
#include "devices/block.h"
#include "threads/vaddr.h"

#define SECTORS_PER_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)

static struct lock swap_lock;
static struct bitmap *swap_valid_table;
static struct block *swap_disk;

void init_swap_table() {
  swap_disk = block_get_role(BLOCK_SWAP);
  swap_valid_table = bitmap_create(block_size(swap_disk) / SECTORS_PER_PAGE);
  bitmap_set_all(swap_valid_table, true);

  lock_init(&swap_lock);
}

void swap_in(int swap_index, void *kpage) {
  lock_acquire(&swap_lock);

  for (int i = 0; i < SECTORS_PER_PAGE; i++) {
    block_read(swap_disk, swap_index * SECTORS_PER_PAGE + i, kpage + i * BLOCK_SECTOR_SIZE);
  }

  bitmap_set(swap_valid_table, swap_index, true);

  lock_release(&swap_lock);
}

int swap_out(void *kpage) {
  lock_acquire(&swap_lock);

  int swap_index = bitmap_scan_and_flip(swap_valid_table, 0, 1, true);

  for (int i = 0; i < SECTORS_PER_PAGE; i++) {
    block_write(swap_disk, swap_index * SECTORS_PER_PAGE + i, kpage + i * BLOCK_SECTOR_SIZE);
  }

  lock_release(&swap_lock);
  return swap_index;
}
