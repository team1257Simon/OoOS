typedef int               error_t;
typedef long unsigned int size_t;
size_t                    strlen(const char*);
void*                     memcpy(void* restrict, const void* restrict, size_t);
void*                     malloc(size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(1)));
error_t                   argz_create(char* const argv[], char** argz, size_t* argz_len)
{
    int   argc = 0;
    int   i    = 0;
    int   len  = 0;
    char* iter;
    *argz_len = 0;
    if(*argv == ((void*)0))
    {
        *argz = ((void*)0);
        return 0;
    }
    while(argv[argc])
    {
        *argz_len += (strlen(argv[argc]) + 1);
        argc++;
    }
    if(!(*argz = (char*)malloc(*argz_len))) return 12;
    iter = *argz;
    for(i = 0; i < argc; i++)
    {
        len = strlen(argv[i]) + 1;
        memcpy(iter, argv[i], len);
        iter += len;
    }
    return 0;
}
