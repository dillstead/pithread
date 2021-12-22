#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <uart.h>

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void) r0;
    (void) r1;
    (void) atags;
    
    uart_init();
    printf("Hello World!\n");
}
