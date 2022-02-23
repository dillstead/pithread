#ifndef __THREAD_H
#define __THREAD_H

#include <inttypes.h>
#include <list.h>

struct thread
{
    /* Must be first */
    uint32_t *sp;
    struct thread *handle;
    struct list_elem link;
    struct list *in_list;
    struct list join_list;
    int code;
};

int thread_init(int preempt);
struct thread *thread_new(int apply(void *),
                          void *args, size_t nbytes);
void thread_exit(int code);
struct thread *thread_self(void);
int thread_join(struct thread *thr);
void thread_pause(void);
#endif
