#ifndef __KERNEL_STDIO
#define __KERNEL_STDIO
#include "kernel/kernel_defs.h"

#ifdef __cplusplus
extern "C" 
{
#endif
typedef int FILE;
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
int printf(const char* restrict fmt, ...);
int asprintf(char** restrict strp, const char* restrict fmt, ...);
int fprintf(FILE* stream, const char* restrict fmt, ...);
int sprintf(char* restrict buffer, const char* restrict fmt, ...);
int snprintf(char* restrict buffer, size_t bufsz, const char* restrict fmt, ...);
#ifdef __cplusplus
}
#endif
#endif