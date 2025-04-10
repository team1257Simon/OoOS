typedef int               error_t;
typedef long unsigned int size_t;
size_t                    strlen(const char*);
void*                     memcpy(void* restrict, const void* restrict, size_t);
void*                     realloc(void*, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));
error_t                   argz_add(char** argz, size_t* argz_len, const char* str)
{
    int    len_to_add = 0;
    size_t last       = *argz_len;
    if(str == ((void*)0)) return 0;
    len_to_add = strlen(str) + 1;
    *argz_len += len_to_add;
    if(!(*argz = (char*)realloc(*argz, *argz_len))) return 12;
    memcpy(*argz + last, str, len_to_add);
    return 0;
}
