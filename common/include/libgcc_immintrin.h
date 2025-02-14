#ifndef __IMM_INTRIN_GCC
#define __IMM_INTRIN_GCC
#include "stddef.h"
#include "stdint.h"
#ifdef __KERNEL__
// Use the kernel-backed aligned malloc in kernel mode
#define _MM_MALLOC_H_INCLUDED
#ifdef __cplusplus
extern "C"
{
#endif
void* _mm_malloc(size_t, size_t);
void* _mm_free(void*);
#ifdef __cplusplus
}
#endif
#endif
#include <immintrin.h>
#endif