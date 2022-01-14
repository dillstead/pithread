#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include "atags.h"

#define TAG_NEXT(t) ((struct atag *)((uint32_t *)(t) + (t)->hdr.size))

static struct atag *atags;

static void print_atags(struct atag *tags)
{
    while (tags->hdr.value != ATAG_NONE)
    {
        switch (tags->hdr.value)
        {
        case ATAG_CORE:
        {
            printf("ATAG_CORE\n");
            break;
        }
        case ATAG_MEM:
        {
            printf("ATAG_MEM\n");
            printf("sz: %u, start: %x\n", tags->u.mem.size, tags->u.mem.start);
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

void atags_init(void *tags)
{
    atags = (struct atag *) tags;
    print_atags(atags);
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

