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
// Some of these functions are used in various support code in libk. Others are just nice to have available.
// The memory-managing functions effectively just call ::operator new(size_t) and ::operator delete(void*).
// Obviously, they don't throw (they return a null pointer if the allocation fails somehow).
extern "C" 
{
#endif 
void* malloc(size_t);
void* realloc(void*, size_t);
void* calloc(size_t, size_t);
void free(void *);
[[noreturn]] void abort();
long atol(const char* str);
long strtol(const char* str, char** endptr, int base);
long long strtoll(const char* str, char** endptr, int base);
unsigned long strtoul(const char* str, char** endptr, int base);
unsigned long long strtoull(const char* str, char** endptr, int base);
void srand(unsigned int);
int rand();
// More as needed
#ifdef __cplusplus
}
#endif
#endif