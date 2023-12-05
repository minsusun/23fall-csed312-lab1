#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "vm/falloc.h"
#include "vm/spt.h"

static struct list frame_table;
static struct lock frame_table_lock;
static struct list_elem *clock;

void
frame_table_init ()
{
    list_init (&frame_table);
    lock_init (&frame_table_lock);
    clock = NULL;
}

void *
falloc_get_page (enum palloc_flags flag, void *upage)
{
    void *kpage;
    struct fte *entry;
    lock_acquire (&frame_table_lock);
    kpage = palloc_get_page (flag);
    if (kpage == NULL)
    {
        evict_frame ();
        kpage = palloc_get_page (flag);
        if (kpage == NULL)
            return NULL;
    }
    entry = (struct fte *) malloc (sizeof *entry);
    entry -> kpage = kpage;
    entry -> upage = upage;
    entry -> thread = thread_current ();
    list_push_back (&frame_table, &(entry -> list_elem));
    lock_release (&frame_table_lock);
    return kpage;
}

void
falloc_free_page (void *kpage)
{
    lock_acquire (&frame_table_lock);
    struct fte *entry = get_fte (kpage);
    if (entry == NULL)
        syscall_exit (-1);
    list_remove (&(entry -> list_elem));
    palloc_free_page (entry -> kpage);
    pagedir_clear_page(entry -> thread -> pagedir, entry -> upage);
    free (entry);
    lock_release (&frame_table_lock);
}

struct fte *
get_fte (void *kpage)
{
    struct list_elem *elem;
    for (elem = list_begin (&frame_table); elem != list_end (&frame_table); elem = list_next(elem))
    {
        struct fte *entry = list_entry (elem, struct fte, list_elem);
        if (entry -> kpage == kpage)
            return entry;
    }
    return NULL;
}

void
evict_frame ()
{
    struct fte *entry;
    struct thread *thread = thread_current ();

    if (clock == NULL)
        clock = list_begin (&frame_table);

    for (clock; clock != list_end (&frame_table); clock = list_next (clock))
    {
        entry = list_entry (clock, struct fte, list_elem);

        if (pagedir_is_accessed (thread -> pagedir, entry -> upage))
            pagedir_set_accessed (thread -> pagedir, entry -> upage, false);
        else
            break;
    }

    if (clock == list_end (&frame_table))
    {
        for (clock = list_begin (&frame_table); clock != list_end (&frame_table); clock = list_next (clock))
        {
            entry = list_entry (clock, struct fte, list_elem);

            if (pagedir_is_accessed (thread -> pagedir, entry -> upage))
                pagedir_set_accessed (thread -> pagedir, entry -> upage, false);
            else
                break;
        }
    }

    struct spte *spte = get_spte (&(thread_current () -> spt), entry ->upage);
    spte -> type = SPAGE_SWAP;
    spte -> swap_id = swap_out (entry -> kpage);

    lock_release (&frame_table_lock);
    falloc_free_page (entry -> kpage);
    lock_acquire (&frame_table_lock);
}