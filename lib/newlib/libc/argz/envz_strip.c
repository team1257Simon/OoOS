typedef long unsigned int size_t;
char*                     strchr(const char*, int);
size_t                    strlen(const char*);
void*                     realloc(void*, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));
void*                     memmove(void*, const void*, size_t);
char*                     argz_next(char* argz, size_t argz_len, const char* entry);
void                      envz_strip(char** envz, size_t* envz_len)
{
    char* entry      = 0;
    int   len        = 0;
    int   null_found = 0;
    while((entry = argz_next(*envz, *envz_len, entry)))
    {
        if(!strchr(entry, '='))
        {
            null_found = 1;
            len        = strlen(entry) + 1;
            if(*envz + *envz_len != entry + len - 1) { memmove(entry, entry + len, *envz + *envz_len - entry - len); }
            *envz_len -= len;
        }
    }
    if(null_found) { *envz = (char*)realloc(*envz, *envz_len); }
}
