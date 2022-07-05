#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#ifndef __ASSEMBLER__

#define CPSR_I_FLAG      0x00000080

#include <stdint.h>
#include <stdbool.h>

void interrupts_init(void);
void register_process_interrupt(int interrupt_num, void (*process_interrupt)(void));
void register_clear_interrupt(int interrupt_num, void (*clear_interrupt)(void));

static inline bool interrupts_get_level(void)
{
    uint32_t flags;
    
    asm volatile("mrs %0, cpsr" : "=r" (flags));
    return !(flags & CPSR_I_FLAG);
}

static inline bool interrupts_enable(void)
{
    bool old_level;

    old_level = interrupts_get_level();
    asm volatile("cpsie i");
    return old_level;
}

static inline bool interrupts_disable(void)
{
    bool old_level;

    old_level = interrupts_get_level();
    asm volatile("cpsid i");
    return old_level;
}

static inline bool interrupts_set_level(bool enable)
{
    return enable ? interrupts_enable() : interrupts_disable();
}

#endif

#include "peripheral.h"

#define INT_BASE       (PERIPHERAL_BASE + 0xB000)
#define INT_IRQ0_PEND  (INT_BASE + 0x0200)
#define INT_IRQ1_PEND  (INT_BASE + 0x0204)
#define INT_IRQ2_PEND  (INT_BASE + 0x0208)
#define INT_FIQ_CTRL   (INT_BASE + 0x020C)
#define INT_IRQ1_ENB   (INT_BASE + 0x0210)
#define INT_IRQ2_ENB   (INT_BASE + 0x0214)
#define INT_IRQ0_ENB   (INT_BASE + 0x0218)
#define INT_IRQ1_DIS   (INT_BASE + 0x021C)
#define INT_IRQ2_DIS   (INT_BASE + 0x0220)
#define INT_IRQ0_DIS   (INT_BASE + 0x0224)

// Interrupts range from 0 to MAX_INTERRUPTS - 1: 
//   00...63 correspond to IRQ 0 - IRQ 63 
//   64...MAX_INTERRUPTS - 1 correspond to the ARM peripherals table
// See 7 Interrupts in the BCM2835 ARM Peripherals manual
#define MAX_INTERRUPTS 72
// Base interrupt number for each register.
#define INT_IRQ1_BASE  0
#define INT_IRQ2_BASE  32
#define INT_IRQ0_BASE  64

#endif


