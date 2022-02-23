#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <palloc.h>
#include <malloc.h>
#include "atags.h"
#include "uart.h"
#include "thread.h"

int count(void *arg)
{
    int *i = (int *) arg;

    printf("%d\n", *i);
    *i = *i + 1;
    printf("%d\n", *i);
    *i = *i + 1;
    printf("%d\n", *i);
    *i = *i + 1;
    return *i;
}

void kernel_main(uint32_t r0, uint32_t r1, void *atags)
{
    (void) r0;
    (void) r1;
    int i = 0;
    struct thread *thr;

    uart_init();
    atags_init(atags);
    palloc_init();
    malloc_init();
    printf("Hello World!\n");

    printf("%d\n", i++);
    printf("%d\n", i++);
    printf("%d\n", i++);
    thread_init(false);
    thr = thread_new(count, &i, sizeof i);
    i = thread_join(thr);
    printf("%d\n", i++);
    printf("%d\n", i++);
    printf("%d\n", i++);
    thread_exit(0);
}
