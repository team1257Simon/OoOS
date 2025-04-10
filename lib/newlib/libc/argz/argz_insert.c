typedef long int          ptrdiff_t;
typedef int               error_t;
typedef long unsigned int size_t;
size_t                    strlen(const char*);
void*                     memcpy(void* restrict, const void* restrict, size_t);
void*                     memmove(void*, const void*, size_t);
void*                     realloc(void*, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));
error_t                   argz_add(char** argz, size_t* argz_len, const char* str);
error_t                   argz_insert(char** argz, size_t* argz_len, char* before, const char* entry)
{
    int       len = 0;
    ptrdiff_t delta;
    if(before == ((void*)0)) return argz_add(argz, argz_len, entry);
    if(before < *argz || before >= *argz + *argz_len) return 22;
    while(before != *argz && before[-1]) before--;
    delta = before - *argz;
    len   = strlen(entry) + 1;
    if(!(*argz = (char*)realloc(*argz, *argz_len + len))) return 12;
    memmove(*argz + delta + len, *argz + delta, *argz_len - delta);
    memcpy(*argz + delta, entry, len);
    *argz_len += len;
    return 0;
}
