/* lab3 - swap table */
#ifndef VM_SWAP_H
#define VM_SWAP_H

void init_swap ();
void swap_in (struct spte *entry, void *kvaddr);
int swap_out (void *kvaddr);

#endif