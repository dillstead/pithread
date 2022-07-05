#ifndef __PAGE_H
#define __PAGE_H

#include <stdint.h>

#define BITMASK(SHIFT, CNT) (((1ul << (CNT)) - 1) << (SHIFT))

/* Page offset (bits 0:12). */
#define PGSHIFT 0                          /* Index of first offset bit. */
#define PGBITS  12                         /* Number of offset bits. */
#define PGSIZE  (1 << PGBITS)              /* Bytes in a page. */
#define PGMASK  BITMASK(PGSHIFT, PGBITS)   /* Page offset bits (0:12). */

/* Offset within a page. */
static inline unsigned pg_ofs(const void *va)
{
  return (uintptr_t) va & PGMASK;
}

/* Virtual page number. */
static inline uintptr_t pg_no(const void *va)
{
  return (uintptr_t) va >> PGBITS;
}

/* Round up to nearest page boundary. */
static inline void *pg_round_up(const void *va)
{
  return (void *) (((uintptr_t) va + PGSIZE - 1) & ~PGMASK);
}

/* Round down to nearest page boundary. */
static inline void *pg_round_down(const void *va)
{
  return (void *) ((uintptr_t) va & ~PGMASK);
}

/* Round up to the next page. */
static inline void *pg_next_page(const void *va)
{
  return (void *) (((uintptr_t) va + PGSIZE) & ~PGMASK);
}

#endif
