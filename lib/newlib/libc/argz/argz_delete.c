typedef int               error_t;
typedef long unsigned int size_t;
size_t                    strlen(const char*);
void*                     realloc(void*, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));
void                      free(void*);
void*                     memmove(void*, const void*, size_t);
error_t                   argz_delete(char** argz, size_t* argz_len, char* entry)
{
    int   len    = 0;
    char* moveto = entry;
    if(entry)
    {
        len = strlen(entry) + 1;
        entry += len;
        memmove(moveto, entry, *argz + *argz_len - entry);
        *argz_len -= len;
        if(!(*argz = (char*)realloc(*argz, *argz_len))) return 12;
        if(*argz_len <= 0)
        {
            free(*argz);
            *argz = ((void*)0);
        }
    }
    return 0;
}
