#ifndef __LIB_DEBUG_H
#define __LIB_DEBUG_H

#include <stddef.h>
#include <stdbool.h>

/* GCC lets us add "attributes" to functions, function
   parameters, etc. to indicate their properties.
   See the GCC manual for details. */
#define UNUSED                    __attribute__ ((unused))
#define NO_RETURN                 __attribute__ ((noreturn))
#define NO_INLINE                 __attribute__ ((noinline))
#define PRINTF_FORMAT(FMT, FIRST) __attribute__ ((format (printf, FMT, FIRST)))

/* Halts the OS, printing the source file name, line number, and
   function name, plus a user-specific message. */
#define PANIC(...)                debug_panic(__FILE__, __LINE__, __func__, __VA_ARGS__)

void debug_panic(const char *file, int line, const char *function,
                 const char *message, ...) PRINTF_FORMAT(4, 5) NO_RETURN;
void debug_backtrace(void);
void hex_dump(size_t ofs, const void *buf_, size_t size, bool ascii);

#undef NOT_REACHED
#ifndef NDEBUG
#define NOT_REACHED() PANIC("executed an unreachable statement");
#else
#define NOT_REACHED() for (;;)
#endif

#endif
