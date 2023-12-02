/* lab3 - supplemental page table */

#ifndef VM_SPT_H
#define VM_SPT_H

#include <hash.h>

struct spte
{
    void *upage;
    void *kpage;

    struct hash_elem hash_elem;

    struct file *file;
    off_t ofs;
    uint32_t read_bytes;
    uint32_t zero_bytes;
    bool writable;
};

void init_spt (struct hash *spt);
void destroy_spt (struct hash *spt);
struct spte *spalloc_get_page (struct hash *spt, void *upage, struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable);

#endif