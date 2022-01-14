#ifndef __LIB_MALLOC_H
#define __LIB_MALLOC_H

#include <stddef.h>

void malloc_init(void);
void *malloc(size_t size) __attribute__ ((malloc));
void *calloc(size_t nmemb, size_t size) __attribute__ ((malloc));
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#endif
