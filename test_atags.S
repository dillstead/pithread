#include "atags.h"
        
.section ".rodata"

.globl  test_atags

test_atags:
        .word   2
        .word   ATAG_CORE

        .word   4
        .word   ATAG_MEM
        .word   1024 * 1024 * 8
        .word   __end

        .word   4
        .word   ATAG_MEM
        .word   1024 * 1024 * 8
        .word   __end + 1024 * 1024 * 8
        
        .word   0
        .word   ATAG_NONE



 
