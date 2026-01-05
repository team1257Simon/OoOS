#ifndef __STRING_H
#define __STRING_H
#include <stddef.h>
#ifdef __cplusplus
#ifndef restrict
#define restrict
#endif
extern "C"
{
#endif
void* memcpy(void* restrict dest, const void* restrict src, size_t n);
int memcmp(const void* restrict a, const void* restrict b, size_t n);
void* memset(void*, int, size_t);
size_t strlen(const char*);
size_t strnlen(const char*, size_t);
int strcmp(const char* restrict lhs, const char* restrict rhs);
int strncmp(const char* restrict lhs, const char* restrict rhs, size_t n);
char* strdup(const char*);
char* strndup(const char*, size_t);
char* strcpy(char* restrict dest, const char* restrict src);
char* strncpy(char* restrict dest, const char* restrict src, size_t n);
char* stpcpy(char* restrict dest, const char* restrict src);
char* strstr(const char* haystack, const char* needle);
char* strchr(const char* str, int chr);
char* strnchr(const char* str, size_t n, int chr);
void* memchr(const void* mem, int chr, size_t n);
void* wmemchr(const void* mem, wchar_t chr, size_t n);
#ifdef __cplusplus
__extension__ void* u16memchr(const void* mem, char16_t chr, size_t n);
}
#endif
#endif