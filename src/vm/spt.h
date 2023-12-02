/* lab3 - supplemental page table */

#ifndef VM_SPT_H
#define VM_SPT_H

#include "filesys/off_t.h"

enum spage_type {
    SPAGE_ZERO,
    SPAGE_FRAME,
    SPAGE_FILE
};

struct spte
{
    enum spage_type type;

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

struct spte *spalloc (struct hash *spt, void *upage, void *kpage, enum spage_type type);
void spalloc_zero (struct hash *spt, void *upage);
void spalloc_frame (struct hash *pst, void *upage, void *kpage);
void spalloc_file (struct hash *spt, void *upage, struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable);

#endif