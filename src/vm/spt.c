/* lab3 - supplemental page table */
#include <hash.h>
#include <string.h>

#include "filesys/file.h"
#include "filesys/off_t.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "userprog/syscall.h"
#include "vm/falloc.h"
#include "vm/spt.h"

unsigned spt_hash_func (struct hash_elem *hash_elem, void *aux);
bool spt_less_func (const struct hash_elem *e1, const struct hash_elem *e2, void *aux);
void spt_destroy_func (struct hash_elem *elem, void *aux);

extern struct lock file_lock;

void
init_spt (struct hash *spt)
{   
    hash_init (spt, spt_hash_func, spt_less_func, NULL);
}

void
destroy_spt (struct hash *spt)
{
    hash_destroy (spt, spt_destroy_func);
}

unsigned
// typedef unsigned hash_hash_func (const struct hash_elem *e, void *aux);
spt_hash_func (struct hash_elem *hash_elem, void *aux)
{
    struct spte *entry = hash_entry (hash_elem, struct spte, hash_elem);   
    return hash_bytes (&(entry -> kpage), sizeof (entry -> kpage));
}

bool
// typedef bool hash_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux);
spt_less_func (const struct hash_elem *e1, const struct hash_elem *e2, void *aux)
{
    void *p1 = hash_entry (e1, struct spte, hash_elem) -> upage;
    void *p2 = hash_entry (e2, struct spte, hash_elem) -> upage;
    return p1 < p2;
}

void
// typedef void hash_action_func (struct hash_elem *e, void *aux);
spt_destroy_func (struct hash_elem *elem, void *aux)
{
    struct spte *entry = hash_entry (elem, struct spte, hash_elem);
    free(entry);
}

struct spte *
spalloc (struct hash *spt, void *upage, void *kpage, enum spage_type type)
{
    struct spte *entry = (struct spte *)malloc (sizeof (struct spte));
    entry -> type = type;
    entry -> upage = upage;
    entry -> kpage = kpage;
    hash_insert (spt, &(entry -> hash_elem));
    return entry;
}

void
spalloc_zero (struct hash *spt, void *upage)
{
    struct spte *entry = spalloc(spt, upage, NULL, SPAGE_ZERO);
}

void
spalloc_frame (struct hash *spt, void *upage, void *kpage)
{
    struct spte *entry = spalloc(spt, upage, kpage, SPAGE_FRAME);
}

void
spalloc_file (struct hash *spt, void *upage, struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
    struct spte *entry = spalloc(spt, upage, NULL, SPAGE_FILE);
    entry -> file = file;
    entry -> ofs = ofs;
    entry -> read_bytes = read_bytes;
    entry -> zero_bytes = zero_bytes;
    entry -> writable = writable;
}

bool
load_page (struct hash *spt, void *upage)
{
    struct spte *entry = get_spte (spt, upage);
    if (entry == NULL)
        syscall_exit (-1);

    void *kpage = falloc_get_page (PAL_USER, upage);
    if (kpage == NULL)
        syscall_exit (-1);
    
    bool flag = lock_held_by_current_thread (&file_lock);

    switch (entry -> type)
    {
        case SPAGE_ZERO:
            memset (kpage, 0, PGSIZE);
            break;
        case SPAGE_FRAME:
            break;
        case SPAGE_FILE:
            if (!flag)
                lock_acquire (&file_lock);
            if (file_read_at (entry -> file, kpage, entry -> read_bytes, entry -> ofs) != entry -> read_bytes)
            {
                falloc_free_page (kpage);
                lock_release (&file_lock);
                syscall_exit (-1);
            }

            memset (kpage + (entry -> read_bytes), 0, entry -> zero_bytes);
            if(!flag)
                lock_release (&file_lock);
            break;
        default:
            syscall_exit (-1);
    }

    uint32_t *pagedir = thread_current () -> pagedir;
    if (!pagedir_set_page (pagedir, upage, kpage, entry -> writable))
    {
        falloc_free_page (kpage);
        syscall_exit (-1);
    }

    entry -> kpage = kpage;
    entry -> type = SPAGE_FRAME;

    return true;
}

struct spte *
get_spte (struct hash *spt, void *upage)
{
    struct spte entry;
    entry.upage = upage;
    
    struct hash_elem *elem = hash_find (spt, &(entry.hash_elem));

    if (elem == NULL)
        return NULL;
    else
        return elem;
}