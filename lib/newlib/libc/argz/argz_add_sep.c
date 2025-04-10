typedef unsigned long size_t;
typedef int           error_t;
error_t               argz_create_sep(const char* string, int sep, char** argz, size_t* argz_len);
void*                 memcpy(void* restrict, const void* restrict, size_t);
void*                 realloc(void*, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));
error_t               argz_add_sep(char** argz, size_t* argz_len, const char* str, int sep)
{
    char*  str_argz     = 0;
    size_t str_argz_len = 0;
    size_t last         = *argz_len;
    argz_create_sep(str, sep, &str_argz, &str_argz_len);
    if(str_argz_len)
    {
        *argz_len += str_argz_len;
        if(!(*argz = (char*)realloc(*argz, *argz_len))) return 12;
        memcpy(*argz + last, str_argz, str_argz_len);
    }
    return 0;
}
