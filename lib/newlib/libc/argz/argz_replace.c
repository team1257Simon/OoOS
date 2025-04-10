typedef int               error_t;
typedef long unsigned int size_t;
size_t                    strlen(const char*);
void*                     memcpy(void* restrict, const void* restrict, size_t);
void*                     realloc(void*, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));
void*                     malloc(size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(1)));
void                      free(void*);
int                       _buf_findstr(const char* str, char** buf, size_t* buf_len);
error_t                   argz_replace(char** argz, size_t* argz_len, const char* str, const char* with, unsigned* replace_count)
{
    const int str_len       = strlen(str);
    const int with_len      = strlen(with);
    const int len_diff      = with_len - str_len;
    char*     buf_iter      = *argz;
    size_t    buf_len       = *argz_len;
    char*     last_iter     = ((void*)0);
    char*     new_argz      = ((void*)0);
    size_t    new_argz_len  = 0;
    char*     new_argz_iter = ((void*)0);
    char*     argz_realloc  = ((void*)0);
    *replace_count          = 0;
    new_argz_len            = *argz_len;
    while(buf_len)
    {
        if(_buf_findstr(str, &buf_iter, &buf_len))
        {
            *replace_count += 1;
            new_argz_len += len_diff;
        }
    }
    if(*replace_count)
    {
        if(!(new_argz = (char*)malloc(new_argz_len))) return 12;
        buf_iter      = *argz;
        buf_len       = *argz_len;
        last_iter     = buf_iter;
        new_argz_iter = new_argz;
        while(buf_len)
        {
            if(_buf_findstr(str, &buf_iter, &buf_len))
            {
                memcpy(new_argz_iter, last_iter, buf_iter - last_iter - str_len);
                new_argz_iter += (buf_iter - last_iter - str_len);
                memcpy(new_argz_iter, with, with_len);
                new_argz_iter += with_len;
                last_iter = buf_iter;
            }
        }
        memcpy(new_argz_iter, last_iter, *argz + *argz_len - last_iter);
        if(!(argz_realloc = (char*)realloc(*argz, new_argz_len)))
        {
            free(new_argz);
            return 12;
        }
        *argz = argz_realloc;
        memcpy(*argz, new_argz, new_argz_len);
        *argz_len = new_argz_len;
        if(*argz_len == 0)
        {
            free(*argz);
            *argz = ((void*)0);
        }
        free(new_argz);
    }
    return 0;
}
