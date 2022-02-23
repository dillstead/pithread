/*#include <console.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/switch.h"
#include "threads/vaddr.h"
#include "devices/serial.h"
#include "devices/shutdown.h"*/

#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

extern void halt(void);

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
