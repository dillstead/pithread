#include <stddef.h>
#include <stdint.h>
#include <atags.h>
#include <page.h>
#include <thread.h>
#include "assert.h"
#include "debug.h"
#include "round.h"
#include "stdio.h"
#include "string.h"
#include "bitmap.h"
#include "palloc.h"

extern void *__start;
extern void *__end;

/* Page allocator.  Hands out memory in page-size (or
   page-multiple) chunks.  See malloc.h for an allocator that
   hands out smaller chunks. */

/* A memory pool. */
struct pool
{
    struct lock lock;                   /* Mutual exclusion. */
    struct bitmap *used_map;            /* Bitmap of free pages. */
    uint8_t *base;                      /* Base of pool. */
};

static struct pool pool;

static void init_pool(struct pool *pool, void *base, size_t page_cnt,
                       const char *name);

/* Initializes the page allocator.  At most USER_PAGE_LIMIT
   pages are put into the user pool. */
void palloc_init(void)
{
    struct atag *tag;
    uint8_t *free_start;
    uint8_t *free_end;
    size_t free_pages;
    
    /* Assumption is that there's only one memory chunk which also
       contains the kernel.  If there are multiple chunks, add a 
       pool for each chunk. */
    tag = get_atag(ATAG_MEM, NULL);
    if (!tag)
    {
        PANIC("palloc_init: no memory");
    }
    /* If this is the chunk that contains the kernel it needs
       to be adjusted in size to account for that. */
    if ((uintptr_t) &__start >= tag->u.mem.start
        && (uintptr_t) &__start < tag->u.mem.start + tag->u.mem.size)
    {
        tag->u.mem.start = (uintptr_t) &__end;
        tag->u.mem.size = tag->u.mem.start + tag->u.mem.size 
            - (uintptr_t) &__end;
    }
    if (tag->u.mem.size < PGSIZE)
    {
        PANIC("palloc_init: no memory");
    }
    free_start = (uint8_t *) tag->u.mem.start;
    free_end = (uint8_t *) (tag->u.mem.start + tag->u.mem.size);
    free_pages = (free_end - free_start) / PGSIZE;
    init_pool(&pool, free_start, free_pages, "pool");
}

/* Obtains and returns a group of PAGE_CNT contiguous free pages.
   If PAL_USER is set, the pages are obtained from the user pool,
   otherwise from the kernel pool.  If PAL_ZERO is set in FLAGS,
   then the pages are filled with zeros.  If too few pages are
   available, returns a null pointer, unless PAL_ASSERT is set in
   FLAGS, in which case the kernel panics. */
void *palloc_get_multiple(enum palloc_flags flags, size_t page_cnt)
{
    void *pages;
    size_t page_idx;

    if (page_cnt == 0)
    {
        return NULL;
    }

    lock_acquire(&pool.lock);
    page_idx = bitmap_scan_and_flip(pool.used_map, 0, page_cnt, false);
    lock_release(&pool.lock);

    if (page_idx != BITMAP_ERROR)
    {
        pages = pool.base + PGSIZE * page_idx;
    }
    else
    {
        pages = NULL;
    }

    if (pages != NULL) 
    {
        if (flags & PAL_ZERO)
        {
            memset(pages, 0, PGSIZE * page_cnt);
        }
    }
    else 
    {
        if (flags & PAL_ASSERT)
        {
            PANIC("palloc_get: out of pages");
        }
    }

    return pages;
}

/* Obtains a single free page and returns its address.

   If PAL_ZERO is set in FLAGS,
   then the page is filled with zeros.  If no pages are
   available, returns a null pointer, unless PAL_ASSERT is set in
   FLAGS, in which case the kernel panics. */
void *palloc_get_page(enum palloc_flags flags) 
{
    return palloc_get_multiple(flags, 1);
}

/* Frees the PAGE_CNT pages starting at PAGES. */
void palloc_free_multiple(void *pages, size_t page_cnt) 
{
    size_t page_idx;

    ASSERT(pg_ofs(pages) == 0);
    if (pages == NULL || page_cnt == 0)
    {
        return;
    }

    page_idx = pg_no(pages) - pg_no(pool.base);

#ifndef NDEBUG
    memset(pages, 0xcc, PGSIZE * page_cnt);
#endif

    ASSERT(bitmap_all(pool.used_map, page_idx, page_cnt));
    bitmap_set_multiple(pool.used_map, page_idx, page_cnt, false);
}

/* Frees the page at PAGE. */
void palloc_free_page(void *page) 
{
    palloc_free_multiple(page, 1);
}

/* Initializes pool P as starting at START and ending at END,
   naming it NAME for debugging purposes. */
static void init_pool(struct pool *p, void *base, size_t page_cnt, const char *name) 
{
    /* We'll put the pool's used_map at its base.
       Calculate the space needed for the bitmap
       and subtract it from the pool's size. */
    size_t bm_pages = DIV_ROUND_UP(bitmap_buf_size(page_cnt), PGSIZE);
    if (bm_pages > page_cnt)
        PANIC("Not enough memory in %s for bitmap.", name);
    page_cnt -= bm_pages;

    printf("%zu pages available in %s.\n", page_cnt, name);

    /* Initialize the pool. */
    lock_init(&p->lock);
    p->used_map = bitmap_create_in_buf(page_cnt, base, bm_pages * PGSIZE);
    p->base = base + bm_pages * PGSIZE;
}
