#include "stdio.h"
#include "stdarg.h"
extern size_t kvfprintf(FILE* fd, const char* fmt, va_list args);
extern size_t kvsprintf(char* restrict buffer, const char* restrict fmt, va_list args);
extern size_t kvsnprintf(char* restrict buffer, size_t n, const char* restrict fmt, va_list args);
extern size_t kvasprintf(char** restrict strp, const char* restrict fmt, va_list args);
attribute(__weak__) int printf(const char* restrict fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t n = kvfprintf(stdout, fmt, args);
    va_end(args);
    return (int)n;
}
attribute(__weak__) int fprintf(FILE* fd, const char* restrict fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t n = kvfprintf(fd, fmt, args);
    va_end(args);
    return (int)n;
}
attribute(__weak__) int sprintf(char* restrict buffer, const char* restrict fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t n = kvsprintf(buffer, fmt, args);
    va_end(args);
    return (int)n;
}
attribute(__weak__) int snprintf(char* restrict buffer, size_t bufsz, const char* restrict fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t n = kvsnprintf(buffer, bufsz, fmt, args);
    va_end(args);
    return (int)n;
}
attribute(__weak__) int asprintf(char** restrict strp, const char* restrict fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = kvasprintf(strp, fmt, args);
    va_end(args);
    return (int)n;
}