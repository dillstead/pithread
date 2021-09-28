#ifndef __MMIO_H
#define __MMIO_H

#include <stdint.h>

static inline void mmio_write(uint32_t reg, uint32_t data)
{
    *(volatile uint32_t *) reg = data;
}

static inline uint32_t mmio_read(uint32_t reg)
{
    return *(volatile uint32_t *) reg;
}

static inline void delay(int32_t count)
{
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
            : "=r"(count): [count]"0"(count) : "cc");
}

#endif
