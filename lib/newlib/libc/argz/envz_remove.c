
typedef unsigned long size_t;
size_t                strlen(const char*);
void*                 memmove(void*, const void*, size_t);
void*                 realloc(void*, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));
char*                 envz_entry(const char* envz, size_t envz_len, const char* name);
void                  envz_remove(char** envz, size_t* envz_len, const char* name)
{
    char* entry = ((void*)0);
    int   len   = 0;
    entry       = envz_entry(*envz, *envz_len, name);
    if(entry)
    {
        len = strlen(entry) + 1;
        if(*envz + *envz_len != entry + len - 1) { memmove(entry, entry + len, *envz + *envz_len - entry - len); }
        *envz = (char*)realloc(*envz, *envz_len - len);
        *envz_len -= len;
    }
}
