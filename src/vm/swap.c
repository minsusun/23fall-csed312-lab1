/* lab3 - swap table */
#include <bitmap.h>

#include "devices/block.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "userprog/syscall.h"
#include "vm/spt.h"
#include "vm/swap.h"

#define SECTORS_PER_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)

struct lock swap_lock;
struct block *swap;
struct bitmap *swap_table;

void
init_swap ()
{
    swap = block_get_role (BLOCK_SWAP);
    swap_table = bitmap_create(block_size(swap) / SECTORS_PER_PAGE);
    bitmap_set_all (swap_table, true);
    lock_init (&swap_lock);
}

void
swap_in (struct spte *entry, void *kvaddr)
{
    int swap_id = entry -> swap_id;

    lock_acquire (&swap_lock);

    if (swap_id > bitmap_size(swap_table) || swap_id < 0 || bitmap_test(swap_table, swap_id))
        syscall_exit (-1);
    
    bitmap_set (swap_table, swap_id, true);

    lock_release (&swap_lock);

    for (int i = 0; i < SECTORS_PER_PAGE; i++)
        block_read (swap, swap_id * SECTORS_PER_PAGE + i, kvaddr + BLOCK_SECTOR_SIZE * i);
}

int
swap_out (void *kvaddr)
{
    lock_acquire (&swap_lock);
    
    int swap_id = bitmap_scan_and_flip (swap_table, 0, 1, true);
    
    lock_release (&swap_lock);
    
    for (int i = 0; i < SECTORS_PER_PAGE; i++)
        block_write (swap, swap_id * SECTORS_PER_PAGE + i, kvaddr + BLOCK_SECTOR_SIZE * i);
    
    return swap_id;
}