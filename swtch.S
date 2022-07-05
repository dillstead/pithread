.globl swtch
.globl thread_start
        
swtch:
        push    { r4-r10, fp, lr }
        // r0 is arg1 (thread currently running)
        // r1 is arg2 (next thread to run)
        str     sp, [r0]
        ldr     sp, [r1]
        pop     { r4-r10, fp, lr }
        mov     pc, lr

thread_start:
        // The stack pointer is now double word aligned as per the
        // ARM-AAPCS (5.2.1 Stack) because the return address to this
        // function was double word aligned in thread_new.
        
        // r10 is apply
        // r9 is args
        // The first argument to a function is passed in r0
        mov     r0, r9
        blx     r10
        // The exit code from apply is passed to thread_exit as the first and
        // only parameter.
        bl      thread_exit
