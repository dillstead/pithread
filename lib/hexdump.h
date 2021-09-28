#ifndef __LIB_HEXDUMP_H
#define __LIB_HEXDUMP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void hex_dump (uintptr_t ofs, const void *buf_, size_t size, bool ascii);

#endif
