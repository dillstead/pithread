#include "atags.h"

.equ KERNEL_STACK_SIZE, 4096
// To keep this in the first portion of the binary.
.section ".text.boot"

// Make _start global.
.globl  _start

// Entry point for the kernel.
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for kernel_main
_start:
        // This set of instructions sets 3 of the 4 cores to go to halt.
        // Model 1 only has 1 cpu and does not have this instruction, so don't include it if building for model 1
        mrc     p15, #0, r1, c0, c0, #5
        and     r1, r1, #3
        cmp     r1, #0
        bne     halt

        // Setup the stack.  Stack will work down from the start of the kernel.
        ldr     sp, =__start
	//mov     r4, #(KERNEL_STACK_SIZE)
	//add     sp, sp, r4

#ifdef COPY_ATAGS
        // Copy atags to the correct location
        mov     r4, #0x100
        ldr     r5, =test_atags

1:              
        // Size in words
        ldr     r6, [r5], #4
        str     r6, [r4], #4

        // Value
        ldr     r7, [r5], #4
        str     r7, [r4], #4
        
        cmp     r6, #0
        beq     3f
        
        subs    r6, #2
2:      
        beq     1b
        ldr     r7, [r5], #4
        str     r7, [r4], #4
        subs    r6, #1
        b       2b

3:
#endif        
        // Clear out bss.
        ldr     r4, =__bss_start
        ldr     r9, =__bss_end
        mov     r5, #0
        mov     r6, #0
        mov     r7, #0
        mov     r8, #0
        b       2f

1:
        // store multiple at r4.
        stmia   r4!, {r5-r8}

        // If we are still below bss_end, loop.
2:
        cmp     r4, r9
        blo     1b

        // Call main
	mov     r2, #0x100
        bl      kernel_main
        b       halt

