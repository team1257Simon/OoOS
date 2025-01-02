#ifndef __STDLIB_H
#define __STDLIB_H
#ifndef _GCC_STDINT_H
#include "stdint.h"
#endif
#include "stddef.h"
#ifndef	NULL
#define NULL		0
#endif

#ifdef __cplusplus
extern "C" 
{
#endif 
void* malloc(size_t);
void* realloc(void *, size_t);
void* calloc(size_t, size_t);
void free(void *);
[[noreturn]] void abort();
// More to come...
#ifdef __cplusplus
}
#endif
#endif