typedef int               error_t;
typedef long unsigned int size_t;
void*                     memcpy(void* restrict, const void* restrict, size_t);
void*                     realloc(void*, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));
error_t                   argz_append(char** argz, size_t* argz_len, const char* buf, size_t buf_len)
{
    if(buf_len)
    {
        size_t last = *argz_len;
        *argz_len += buf_len;
        if(!(*argz = (char*)realloc(*argz, *argz_len))) return 12;
        memcpy(*argz + last, buf, buf_len);
    }
    return 0;
}
