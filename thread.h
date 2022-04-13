#ifndef __THREAD_H
#define __THREAD_H

#include <inttypes.h>
#include <stdbool.h>
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
    uint32_t magic;
};

int thread_init(bool preempt);
struct thread *thread_new(int apply(void *),
                          void *args, size_t nbytes);
void thread_exit(int code);
struct thread *thread_self(void);
int thread_join(struct thread *thr);
void thread_pause(void);

struct semaphore 
{
    unsigned int value;         /* Current value. */
    struct list waiters;        /* List of waiting threads. */
};

void sem_init(struct semaphore *sem, unsigned int value);
void sem_down(struct semaphore *sem);
bool sem_try_down(struct semaphore *sem);
void sem_up(struct semaphore *sem);

struct lock 
{
    struct thread *holder;      /* Thread holding lock (for debugging). */
    struct semaphore sem;       /* Binary semaphore controlling access. */
    struct list_elem elem;      /* List element for thread's lock list. */
};

void lock_init(struct lock *lock);
void lock_acquire(struct lock *lock);
bool lock_try_acquire(struct lock *lock);
void lock_release(struct lock *lock);

struct condition 
{
    struct list waiters;        /* List of waiting threads. */
};

void cond_init(struct condition *cond);
void cond_wait(struct condition *cond, struct lock *lock);
void cond_signal(struct condition *cond, struct lock *lock);
void cond_broadcast(struct condition *cond, struct lock *lock);

struct channel
{
    const void *ptr;
    size_t *size;
    struct semaphore send;
    struct semaphore recv;
    struct semaphore sync;
};

void chan_init(struct channel *chan);
size_t chan_send(struct channel *chan, const void *ptr, size_t size);
size_t chan_recv(struct channel *chan, void *ptr, size_t size);

#endif
