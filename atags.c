#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include "atags.h"

#define TAG_NEXT(t) ((struct atag *)((uint32_t *)(t) + (t)->hdr.size))

static struct atag *atags;

#ifndef NDEBUG
static void print_atags(struct atag *tags)
{
    while (tags->hdr.value != ATAG_NONE)
    {
        switch (tags->hdr.value)
        {
        case ATAG_CORE:
        {
            printf("ATAG_CORE\n");
            if (tags->hdr.size > 2)
            {
                printf("flags: %p, page size: %u, dev: %u\n", tags->u.core.flags,
                       tags->u.core.pagesize, tags->u.core.rootdev);
            }
            break;
        }
        case ATAG_MEM:
        {
            printf("ATAG_MEM\n");
            printf("size: %u, start: %x\n", tags->u.mem.size, tags->u.mem.start);
            break;
        }
        case ATAG_CMDLINE:
        {
            printf("ATAG_CMDLINE\n");
            if (tags->hdr.size > 2)
            {
                printf("%s\n", tags->u.cmdline.cmdline);
            }
            break;
        }
        default:
        {
            printf("Unknown %u\n", tags->hdr.value);
            break;
        }
        }
        tags = TAG_NEXT(tags);
    }
    printf("ATAG_NONE\n");
}
#endif

void atags_init(void *tags)
{
    atags = (struct atag *) tags;
#ifndef NDEBUG
    print_atags(atags);
#endif
}

struct atag *get_atag(unsigned int value, struct atag **next)
{
    struct atag *tags = (!next || !*next) ? atags : *next;
    
    while (tags->hdr.value != ATAG_NONE)
    {
        if (value == tags->hdr.value)
        {
            if (next)
            {
                *next = TAG_NEXT(tags);
            }
            return tags;
        }
        tags = TAG_NEXT(tags);
    }
    return NULL;
}
