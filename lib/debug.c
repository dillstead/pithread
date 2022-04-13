#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "debug.h"

void halt(void);

/* Halts, printing the source file name, line number, and
   function name, plus a user-specific message. */
void debug_panic(const char *file, int line, const char *function,
                 const char *message, ...)
{
    va_list args;

    printf("Kernel PANIC at %s:%d in %s(): ", file, line, function);
    va_start(args, message);
    vprintf(message, args);
    printf("\n");
    va_end(args);
    halt();
    for (;;);
}

/* Prints the call stack, that is, a list of addresses, one in
   each of the functions we are nested within. */
void debug_backtrace(void)
{
    printf("backtrace: ");
    for (uint32_t *frame = __builtin_frame_address(0);
         frame; frame = (uint32_t *) frame[-1])
    {
        printf("%p ", frame[0]);
    }
    printf("\n");
}
