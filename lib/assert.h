#ifndef __LIB_ASSERT_H
#define __LIB_ASSERT_H

#include <stdio.h>
#include "debug.h"

void halt(void);

#undef assert
#ifdef NDEBUG
#define ASSERT(e) ((void) 0)
#else
#define ASSERT(e) ((void) ((e) || (printf("%s:%d: Assertion failed: %s\n", __FILE__, __LINE__, #e), \
                                   debug_backtrace(), halt(), 0)))
#endif

#endif
