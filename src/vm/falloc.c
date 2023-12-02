#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "userprog/syscall.h"
#include "vm/falloc.h"

static struct list frame_table;
static struct lock frame_table_lock;

void
frame_table_init ()
{
    list_init (&frame_table);
    lock_init (&frame_table_lock);
}

void *
falloc_get_page (enum palloc_flags flag, void *upage)
{
    void *kpage;
    struct fte *entry;
    lock_acquire (&frame_table_lock);
    kpage = palloc_get_page (flag);
    if (kpage != NULL)
    {
        entry = (struct fte *) malloc (sizeof (struct fte));
        entry -> kpage = kpage;
        entry -> upage = upage;
        entry -> t = thread_current ();
        list_push_back (&frame_table, &(entry -> list_elem));
    }
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
    pagedir_clear_page(entry -> t -> pagedir, entry -> upage);
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