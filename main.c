#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <uart.h>

void main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    uart_init();
    printf("Hello World!\n");
}
