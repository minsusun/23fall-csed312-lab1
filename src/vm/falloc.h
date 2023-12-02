/* lab3 - frame table */

#ifndef VM_FALLOC_H
#define VM_FALLOC_H

#include <list.h>

#include "threads/thread.h"

// fte: frame table entry
struct fte
{
    void *kpage;
    void *upage;

    struct thread *t;
    struct list_elem list_elem;
};

void frame_table_init (void);
void *falloc_get_page (enum palloc_flags flag, void *upage);
void falloc_free_page (void *kpage);
struct fte *get_fte (void *kpage);

#endif