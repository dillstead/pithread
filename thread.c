#include <stdlib.h>
#include <round.h>
#include <assert.h>
#include <string.h>
#include "thread.h"

#define THREAD_MAGIC 0xDEADBEEF

struct thread_start_frame
{
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t fp;
    uint32_t lr;
};

/* One semaphore in a list. */
struct semaphore_elem 
{
  struct list_elem elem;              /* List element. */
  struct semaphore semaphore;         /* This semaphore. */
};

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

    thr = list_entry(list_pop_front(list), struct thread, link);
    ASSERT(thr->in_list == list);
    ASSERT(thr->magic == THREAD_MAGIC);
    thr->in_list = NULL;
    return thr;
}

static void put(struct thread *thr, struct list *list)
{
    ASSERT(thr);
    ASSERT(thr->in_list == NULL);
    ASSERT(thr->magic == THREAD_MAGIC);

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

int thread_init(bool preempt)
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
    root.magic = THREAD_MAGIC;
    current = &root;
    num_thrs = 1;
    return 1;
}

struct thread *thread_new(int apply(void *),
                          void *args, size_t nbytes)
{
    struct thread *thr;
    size_t stack_sz;
    struct thread_start_frame *frame;
    
    ASSERT(current);
    ASSERT(current->magic == THREAD_MAGIC);
    ASSERT(apply);

    if (!args)
    {
        nbytes = 0;
    }
    stack_sz = ROUND_UP(16 * 1024 + sizeof *thr + nbytes, 15);
    release();
    thr = malloc(stack_sz);
    if (!thr)
    {
        return NULL;
    }
    memset(thr, 0, stack_sz);
    thr->magic = THREAD_MAGIC;
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
    thr->sp -= sizeof *frame / sizeof *thr->sp;
    frame = (struct thread_start_frame *) thr->sp;
    frame->r9 = (int32_t) args;
    frame->r10 = (int32_t) apply;
    frame->lr = (int32_t) thread_start;
    
    num_thrs++;
    put(thr, &ready_list);
    return thr;
}

void thread_exit(int code)
{
    struct thread *thr;
    
    ASSERT(current);
    ASSERT(current->magic == THREAD_MAGIC);

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
    ASSERT(current->magic == THREAD_MAGIC);
    
    return current;
}

int thread_join(struct thread *thr)
{
    ASSERT(current && thr != current);
    ASSERT(current->magic == THREAD_MAGIC);

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
    ASSERT(current->magic == THREAD_MAGIC);

    put(current, &ready_list);
    run();
}

/* Initializes semaphore SEMA to VALUE.  A semaphore is a
   nonnegative integer along with two atomic operators for
   manipulating it:

   - down or "P": wait for the value to become positive, then
   decrement it.

   - up or "V": increment the value (and wake up one waiting
   thread, if any). */
void sem_init(struct semaphore *sem, unsigned int value)
{
    ASSERT(sem != NULL);

    sem->value = value;
    list_init(&sem->waiters);   
}

/* Down or "P" operation on a semaphore.  Waits for SEMA's value
   to become positive and then atomically decrements it. */
void sem_down(struct semaphore *sem)
{
    ASSERT(sem != NULL);
    ASSERT(current);

    while (sem->value == 0) 
    {
        put(current, &sem->waiters);
        run();
    }
    sem->value--;
}

/* Down or "P" operation on a semaphore, but only if the
   semaphore is not already 0.  Returns true if the semaphore is
   decremented, false otherwise.

   This function may be called from an interrupt handler. */
bool sem_try_down(struct semaphore *sem)
{
    ASSERT(sem != NULL);

    bool success;
        
    if (sem->value > 0) 
    {
        sem->value--;
        success = true; 
    }
    else
    {
        success = false;
    }
    return success;   
}

/* Up or "V" operation on a semaphore.  Increments SEMA's value
   and wakes up the highest priority thread of those waiting for SEMA, 
   if any. */
void sem_up(struct semaphore *sem)
{
    struct thread *thr;
        
    ASSERT(sem != NULL);
    
    sem->value++;
    if (!list_empty(&sem->waiters))
    {
        thr = get(&sem->waiters);
        put(thr, &ready_list);
    }
}

/* Returns true if the current thread holds LOCK, false
   otherwise.  (Note that testing whether some other thread holds
   a lock would be racy.) */
bool
lock_held_by_current_thread (const struct lock *lock) 
{
    ASSERT(lock != NULL);

    return lock->holder == current;
}

/* Initializes LOCK.  A lock can be held by at most a single
   thread at any given time.  Our locks are not "recursive", that
   is, it is an error for the thread currently holding a lock to
   try to acquire that lock.

   A lock is a specialization of a semaphore with an initial
   value of 1.  The difference between a lock and such a
   semaphore is twofold.  First, a semaphore can have a value
   greater than 1, but a lock can only be owned by a single
   thread at a time.  Second, a semaphore does not have an owner,
   meaning that one thread can "down" the semaphore and then
   another one "up" it, but with a lock the same thread must both
   acquire and release it.  When these restrictions prove
   onerous, it's a good sign that a semaphore should be used,
   instead of a lock. */
void lock_init(struct lock *lock)
{
    ASSERT(lock != NULL);

    lock->holder = NULL;
    sem_init(&lock->sem, 1);
}

/* Acquires LOCK, sleeping until it becomes available if
   necessary.  The lock must not already be held by the current
   thread. */
void lock_acquire(struct lock *lock)
{
    ASSERT(lock != NULL);
    ASSERT(!lock_held_by_current_thread(lock));

    sem_down(&lock->sem);
    lock->holder = current;
}

/* Tries to acquires LOCK and returns true if successful or false
   on failure.  The lock must not already be held by the current
   thread. */
bool lock_try_acquire(struct lock *lock)
{
    bool success;

    ASSERT(lock != NULL);
    ASSERT(!lock_held_by_current_thread(lock));

    success = sem_try_down(&lock->sem);
    if (success)
    {
        lock->holder = current;
    }
    return success; 
}

/* Releases LOCK, which must be owned by the current thread 
   and wakes up a thread waiting on the lock. */
void lock_release(struct lock *lock)
{
    ASSERT(lock != NULL);
    ASSERT(lock_held_by_current_thread(lock));

    lock->holder = NULL;
    sem_up (&lock->sem);
}

/* Initializes condition variable COND.  A condition variable
   allows one piece of code to signal a condition and cooperating
   code to receive the signal and act upon it. */
void cond_init(struct condition *cond)
{
    ASSERT(cond != NULL);

    list_init (&cond->waiters);
    
}

/* Atomically releases LOCK and waits for COND to be signaled by
   some other piece of code.  After COND is signaled, LOCK is
   reacquired before returning.  LOCK must be held before calling
   this function.

   The monitor implemented by this function is "Mesa" style, not
   "Hoare" style, that is, sending and receiving a signal are not
   an atomic operation.  Thus, typically the caller must recheck
   the condition after the wait completes and, if necessary, wait
   again.

   A given condition variable is associated with only a single
   lock, but one lock may be associated with any number of
   condition variables.  That is, there is a one-to-many mapping
   from locks to condition variables. */
void cond_wait(struct condition *cond, struct lock *lock)
{
    struct semaphore_elem waiter;

    ASSERT(cond != NULL);
    ASSERT(lock != NULL);
    ASSERT(lock_held_by_current_thread(lock));
  
    sem_init(&waiter.semaphore, 0);
    list_push_back(&cond->waiters, &waiter.elem);
    lock_release(lock);
    sem_down(&waiter.semaphore);
    lock_acquire(lock);   
}

/* If any threads are waiting on COND (protected by LOCK), then
   this function signals one of the threads to wake up. 
   LOCK must be held before calling this function. */
void cond_signal(struct condition *cond, struct lock *lock)
{
  struct list_elem *e;
  
  ASSERT(cond != NULL);
  ASSERT(lock != NULL);
  ASSERT(lock_held_by_current_thread(lock));

  if (!list_empty(&cond->waiters))
  {
      e = list_pop_front(&cond->waiters);
      sem_up(&list_entry(e, struct semaphore_elem, elem)->semaphore);
  }
}

/* Wakes up all threads, if any, waiting on COND (protected by
   LOCK).  LOCK must be held before calling this function. */
void cond_broadcast(struct condition *cond, struct lock *lock)
{
    ASSERT(cond != NULL);
    ASSERT(lock != NULL);
    ASSERT(lock_held_by_current_thread(lock));

    while (!list_empty (&cond->waiters))
    {
        cond_signal(cond, lock);
    }
}

/* Initalizes channel CHAN.  A channel is used to synchronously send "messages" from
   one thread to another. */
void chan_init(struct channel *chan)
{
    ASSERT(chan != NULL);

    chan->ptr = NULL;
    chan->size = NULL;
    sem_init(&chan->send, 1);
    sem_init(&chan->recv, 0);
    sem_init(&chan->sync, 0);
}

/* Send a message of size SIZE through channel CHAN to a receiver.  Waits 
   for the receiver to receive the message before returning the number 
   of bytes copied by the receiver. */
size_t chan_send(struct channel *chan, const void *ptr, size_t size)
{
    ASSERT(chan != NULL);
    ASSERT(ptr != NULL);

    sem_down(&chan->send);
    chan->ptr = ptr;
    chan->size = &size;
    sem_up(&chan->recv);
    sem_down(&chan->sync);
    return size;
}

/* Waits until a message is avaible in channel CHAN, copies the message
   to a buffer pointed to by PTR.  If the message is too large to fit
   in SIZE size, it's truncated.  The number of bytes copied is returned. */
size_t chan_recv(struct channel *chan, void *ptr, size_t size)
{
    size_t n;
    
    ASSERT(chan != NULL);
    ASSERT(ptr != NULL);

    sem_down(&chan->recv);
    n = *chan->size;
    if (size < n)
    {
        n = size;
    }
    *chan->size = n;
    if (n > 0)
    {
        memcpy(ptr, chan->ptr, n);
    }
    sem_up(&chan->sync);
    sem_up(&chan->send);
    return n;    
}

