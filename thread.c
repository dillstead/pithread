#include <stdlib.h>
#include <round.h>
#include <assert.h>
#include <string.h>
#include "thread.h"

static struct thread *current;
static struct thread root;
static unsigned int num_thrs;
static struct list ready_list;
static struct list free_list;
static struct list join_list;

extern void swtch(struct thread *from, struct thread *to);
extern void thread_start(void);

static struct thread *get(struct list *list)
{
    struct thread *thr;

    thr = list_entry(list_pop_back(list), struct thread, link);
    ASSERT(thr->in_list == list);
    thr->in_list = NULL;
    return thr;
}

static void put(struct thread *thr, struct list *list)
{
    ASSERT(thr);
    ASSERT(thr->in_list == NULL);

    thr->in_list = list;
    list_push_back(list, &thr->link);
}

static void run(void)
{
    struct thread *thr = current;

    current = get(&ready_list);
    swtch(thr, current);
}

static void release(void)
{
    struct thread *thr;

    while (!list_empty(&free_list))
    {
        thr = list_entry(list_pop_front(&free_list), struct thread, link);
        free(thr);
    }
}

int thread_init(int preempt)
{
    ASSERT(preempt == 0);
    ASSERT(current == NULL);

    if (preempt)
    {
        return 0;
    }
    list_init(&ready_list);
    list_init(&free_list);
    list_init(&join_list);
    list_init(&root.join_list);
    root.handle = &root;
    current = &root;
    num_thrs = 1;
    return 1;
}

struct thread *thread_new(int apply(void *),
                          void *args, size_t nbytes)
{    
    ASSERT(current);
    ASSERT(apply);
    ASSERT((args && nbytes > 0) || args == NULL);
    
    struct thread *thr;
    size_t stack_sz;

    if (!args)
    {
        nbytes = 0;
    }
    stack_sz = ROUND_UP(16 + 1024 + sizeof *thr + nbytes, 15);
    release();
    thr = malloc(stack_sz);
    if (!thr)
    {
        return NULL;
    }
    memset(thr, 0, stack_sz);
    list_init(&thr->join_list);
    thr->sp = (uint32_t *) ROUND_DOWN((uintptr_t) ((uint8_t *) thr + stack_sz), 16);
    thr->handle = thr;
    if (nbytes > 0)
    {
        thr->sp = (uint32_t *) ((uint8_t *) thr->sp - ROUND_UP(nbytes, 16));
        memcpy(thr->sp, args, nbytes);
        args = thr->sp;
    }
    
    /* At this point, the stack is aligned at 16 bytes.  Setup the stack so 
       that when the thread returns from swtch for the first time, it will run 
       thread_start with apply and args in registers. As per the ARM-AAPCS 
       (5.2.1 Stack) the stack must be double word aligned upon function entry.  
       Registers r4 - r8, r10, and r11 must be preserved (5.1.1 Core registers).  
       r9 is designated as a platform specific register and in this case it's 
       used as a callee-saved register variable so it must also be preserved.
       Stack contents:

       00               sp[10] 
       00               sp[9]
       lr  thread_start sp[8] <-- double word aligned
       fp               sp[7]
       r10 apply        sp[6]
       r9  args         sp[5]
       r8               sp[4]
       r7               sp[3]
       r6               sp[2]
       r5               sp[1]
       r4               sp[0]
    */
    thr->sp -= 11;
    thr->sp[5] = (int32_t) args;
    thr->sp[6] = (int32_t) apply;
    thr->sp[7] = (int32_t) thr->sp + 9;
    thr->sp[8] = (int32_t) thread_start;
    
    num_thrs++;
    put(thr, &ready_list);
    return thr;
}

void thread_exit(int code)
{
    ASSERT(current);

    struct thread *thr;
    
    release();

    if (current != &root)
    {
        put(current, &free_list);
    }
    current->handle = NULL;
    while (!list_empty(&current->join_list))
    {
        thr = get(&current->join_list);
        thr->code = code;
        put(thr, &ready_list);
    }
    if (!list_empty(&join_list) && num_thrs == 2)
    {
        ASSERT(list_empty(&ready_list));
        put(get(&join_list), &ready_list);
    }
    if (--num_thrs == 0)
    {
        halt();
    }
    else
    {
        run();
    }
}

struct thread *thread_self(void)
{
    ASSERT(current);
    
    return current;
}

int thread_join(struct thread *thr)
{
    ASSERT(current && thr != current);

    if (thr)
    {
        if (thr->handle == thr)
        {
            put(current, &thr->join_list);
            run();
            return current->code;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        ASSERT(list_empty(&join_list));
        if (num_thrs > 1)
        {
            put(current, &join_list);
            run();
        }
        return 0;
    }
    return 0;
}

void thread_pause(void)
{
    ASSERT(current);

    put(current, &ready_list);
    run();
}
