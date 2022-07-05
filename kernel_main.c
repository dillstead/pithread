#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <palloc.h>
#include <malloc.h>
#include <assert.h>
#include "atags.h"
#include "uart.h"
#include "thread.h"
#include "rand.h"

extern bool spin_test(void);
extern int sieve_test(void);
extern int sort_test(void);
    
void kernel_main(uint32_t r0, uint32_t r1, void *atags)
{
    (void) r0;
    (void) r1;

    uart_init();
    atags_init(atags);
    rand_init();
    palloc_init();
    malloc_init();
    thread_init(false);

    printf("Running tests...\n");
    if (spin_test())
    {
        printf("spin_test passed\n");
    }
    else
    {
        printf("spin_test failed\n");        
    }
    if (sieve_test())
    {
        printf("sieve_test passed\n");
    }
    else
    {
        printf("sieve_test failed\n");
    }
    if (sort_test())
    {
        printf("sort_test passed\n");
    }
    else
    {
        printf("sort_test failed\n");
    }
    printf("Tests finished\n");
    thread_exit(0);
}
