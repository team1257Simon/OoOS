#ifndef __KERNEL_STDIO
#define __KERNEL_STDIO
#include "kernel/kernel_defs.h"
// Internal pseudo-cstdio implementations for use with GDB stubs. The stderr pointer resolves to an integer of 1; the others give 0.
// The only purpose of this is to be able to indicate that the target is stderr by marking the text with an error indicator [!].
// They all target the same sink (the serial driver). Sprintf instead uses an untied string buffer.
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