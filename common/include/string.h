#ifndef __STRING_H
#define __STRING_H
#include "stddef.h"
#ifdef __cplusplus
#ifndef restrict
#define restrict
#endif
extern "C" 
{
#endif
void* memcpy(void* restrict src, const void* restrict dst, size_t n);
int memcmp(const void* restrict a, const void* restrict b, size_t n);
void* memset(void*, int, size_t);
size_t strlen(const char*);
size_t strnlen(const char*, size_t);
int strcmp(const char* restrict lhs, const char* restrict rhs);
int strncmp(const char* restrict lhs, const char* restrict rhs, size_t);
char* strdup(const char*);
char* strndup(const char*, size_t);
char* strcpy(char* restrict src, const char* restrict dst);
char* strncpy(char* restrict src, const char* restrict dst, size_t);
char* stpcpy(char* restrict src, const char* restrict dst);
char* strstr(const char*, const char*);
char* strchr(const char*, int);
char* strnchr(const char*, size_t, int);
void* memchr(const void*, int, size_t);
#ifdef __cplusplus
}
#endif
#endif