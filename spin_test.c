#include <stdio.h>
#include <assert.h>
#include "thread.h"

#define NBUMP        30000
#define NUM_THREADS  5

struct args
{
    struct lock *lock;
    int *ip;
};

static int n;

int unsafe(void *cl)
{
    int *ip = cl;

    for (int i = 0; i < NBUMP; i++)
    {
        *ip = *ip + 1;
    }
    return 0;
}

int safe(void *cl)
{
    struct args *p = cl;
    
    for (int i = 0; i < NBUMP; i++)
    {
        lock_acquire(p->lock);
        *p->ip = *p->ip + 1;
        lock_release(p->lock);
    }
    return 0;
}

bool spin_test(void)
{
    struct lock lock;
    struct args args;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_new(unsafe, &n, 0);
    }
    thread_join(NULL);
    if (n != NBUMP * NUM_THREADS)
    {
        printf("spin_test: n, expected %d, actual %d\n",
               NBUMP * NUM_THREADS, n);
        return false;
    }

    n = 0;
    lock_init(&lock);
    args.lock = &lock;
    args.ip = &n;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_new(safe, &args, sizeof args);
    }
    thread_join(NULL);
    if (n != NBUMP * NUM_THREADS)
    {
        printf("spin_test: n, expected %d, actual %d\n",
               NBUMP * NUM_THREADS, n);
        return false;
    }
    return true;
}
