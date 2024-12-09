#ifndef VM_SWAP_H
#define VM_SWAP_H

void init_swap_table();
void swap_in(int swap_index, void *kpage);
int swap_out(void *kpage);

#endif