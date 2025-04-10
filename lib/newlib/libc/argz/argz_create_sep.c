typedef int               error_t;
typedef long unsigned int size_t;
size_t                    strlen(const char*);
void*                     memcpy(void* restrict, const void* restrict, size_t);
void*                     realloc(void*, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));
void*                     malloc(size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(1)));
char*                     strsep(char**, const char*);
void                      free(void*);
char*                     strdup(const char*);
error_t                   argz_create_sep(const char* string, int sep, char** argz, size_t* argz_len)
{
    int   len         = 0;
    int   i           = 0;
    int   num_strings = 0;
    char  delim[2];
    char* running     = 0;
    char* old_running = 0;
    char* token       = 0;
    char* iter        = 0;
    *argz_len         = 0;
    if(!string || string[0] == '\0')
    {
        *argz = ((void*)0);
        return 0;
    }
    delim[0]    = sep;
    delim[1]    = '\0';
    running     = strdup(string);
    old_running = running;
    while((token = strsep(&running, delim)))
    {
        len = strlen(token);
        *argz_len += (len + 1);
        num_strings++;
    }
    if(!(*argz = (char*)malloc(*argz_len))) return 12;
    free(old_running);
    running     = strdup(string);
    old_running = running;
    iter        = *argz;
    for(i = 0; i < num_strings; i++)
    {
        token = strsep(&running, delim);
        len   = strlen(token) + 1;
        memcpy(iter, token, len);
        iter += len;
    }
    free(old_running);
    return 0;
}
