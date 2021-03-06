#ifndef __LIB_PALLOC_H
#define __LIB_PALLOC_H

#include <stddef.h>

/* How to allocate pages. */
enum palloc_flags
{
    PAL_ASSERT = 001,           /* Panic on failure. */
    PAL_ZERO = 002,             /* Zero page contents. */
};

void palloc_init(void);
void *palloc_get_page(enum palloc_flags);
void *palloc_get_multiple(enum palloc_flags, size_t page_cnt);
void palloc_free_page(void *);
void palloc_free_multiple(void *, size_t page_cnt);

#endif
