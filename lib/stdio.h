#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H

#include <stdarg.h>

int printf(const char *format, ...);
int vprintf(const char *format, va_list args);

#endif
