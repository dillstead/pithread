#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include "stdio.h"
#include "ctype.h"
#include "debug.h"
#include "round.h"

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
    debug_backtrace();
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

/* Dumps the SIZE bytes in BUF to the console as hex bytes
   arranged 16 per line.  Numeric offsets are also included,
   starting at OFS for the first byte in BUF.  If ASCII is true
   then the corresponding ASCII characters are also rendered
   alongside. */   
void hex_dump(size_t ofs, const void *buf_, size_t size, bool ascii)
{
    const uint8_t *buf = buf_;
    const size_t per_line = 16; /* Maximum bytes per line. */

    while (size > 0)
    {
        size_t start, end, n;
        size_t i;
      
        /* Number of bytes on this line. */
        start = ofs % per_line;
        end = per_line;
        if (end - start > size)
        {
            end = start + size;
        }
        n = end - start;

        /* Print line. */
        printf ("%08jx  ", (uintmax_t) ROUND_DOWN (ofs, per_line));
        for (i = 0; i < start; i++)
        {
            printf ("   ");
        }
        for (; i < end; i++)
        {
            printf ("%02hhx%c",
                    buf[i - start], i == per_line / 2 - 1? '-' : ' ');
        }
        if (ascii) 
        {
            for (; i < per_line; i++)
            {
                printf ("   ");
            }
            printf ("|");
            for (i = 0; i < start; i++)
            {
                printf (" ");
            }
            for (; i < end; i++)
            {
                printf ("%c",
                        isprint (buf[i - start]) ? buf[i - start] : '.');
            }
            for (; i < per_line; i++)
            {
                printf (" ");
            }
            printf ("|");
        }
        printf ("\n");

        ofs += n;
        buf += n;
        size -= n;
    }
}
