#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <palloc.h>
#include "atags.h"
#include "uart.h"

void kernel_main(uint32_t r0, uint32_t r1, void *atags)
{
    (void) r0;
    (void) r1;

    uart_init();
    atags_init(atags);
    palloc_init();
    printf("Hello World!\n");
}
