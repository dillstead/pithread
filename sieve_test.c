#include <stdio.h>
#include <malloc.h>
#include "thread.h"

#define N    3
#define LAST 1000

static int cnt;
static int actual[LAST];
static int expected[LAST] =
{
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31,
    37, 41, 43, 47, 53, 59, 61, 67, 71, 73,
    79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163,
    167, 173, 179, 181, 191, 193, 197, 199,
    211, 223, 227, 229, 233, 239, 241, 251,
    257, 263, 269, 271, 277, 281, 283, 293,
    307, 311, 313, 317, 331, 337, 347, 349,
    353, 359, 367, 373, 379, 383, 389, 397,
    401, 409, 419, 421, 431, 433, 439, 443,
    449, 457, 461, 463, 467, 479, 487, 491,
    499, 503, 509, 521, 523, 541, 547, 557,
    563, 569, 571, 577, 587, 593, 599, 601,
    607, 613, 617, 619, 631, 641, 643, 647,
    653, 659, 661, 673, 677, 683, 691, 701,
    709, 719, 727, 733, 739, 743, 751, 757,
    761, 769, 773, 787, 797, 809, 811, 821,
    823, 827, 829, 839, 853, 857, 859, 863,
    877, 881, 883, 887, 907, 911, 919, 929,
    937, 941, 947, 953, 967, 971, 977, 983,
    991, 997
};

int source(void *cl)
{
    struct channel *chan = cl;
    int i = 2;
    
    if (chan_send(chan, &i, sizeof i))
    {
        for (i = 3; chan_send(chan, &i, sizeof i); i += 2);
    }
    return 0;
}

void filter(int *primes, struct channel *input, struct channel *output)
{
    int j, x;
    
    for (;;)
    {
        chan_recv(input, &x, sizeof x);
        for (j = 0; primes[j] != 0 && x%primes[j] != 0; j++);
        if (primes[j] == 0)
        {
            if (chan_send(output, &x, sizeof x) == 0)
            {
                break;
            }
        }
    }
    chan_recv(input, &x, 0);
 }

int sink(void *cl)
{
    struct channel *input = cl;
    struct channel chan;
    struct thread *thread;
    int i = 0, j, x, primes[256];
    primes[0] = 0;
    
    for (;;)
    {
        chan_recv(input, &x, sizeof x);
        for (j = 0; primes[j] != 0 && x % primes[j] != 0; j++);
        if (primes[j] == 0)
        {
            if (x > LAST)
            {
                break;
            }
            actual[cnt++] = x;
            primes[i++] = x;
            primes[i] = 0;
            if (i == N)
            {
                chan_init(&chan);
                thread = thread_new(sink, &chan, 0);
                filter(primes, input, &chan);
                thread_join(thread);
                return 0;
            }
        }
    }
    chan_recv(input, &x, 0);
    return 0;
}

bool sieve_test(void)
{
    struct channel chan;

    chan_init(&chan);
    thread_new(source, &chan, 0);
    thread_new(sink, &chan, 0);
    thread_join(NULL);
    for (int i = 0; i < LAST; i++)
    {
        if (actual[i] != expected[i])
        {
            printf("sieve_test: pos %d expected %d, actual %d\n",
                   i, expected[i], actual[i]);
            return false;
        }
    }
    return true;
}
