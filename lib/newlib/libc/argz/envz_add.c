typedef int               error_t;
typedef long int          ptrdiff_t;
typedef long unsigned int size_t;
size_t                    strlen(const char*);
void*                     memcpy(void* restrict, const void* restrict, size_t);
void*                     malloc(size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(1)));
void                      free(void*);
error_t                   argz_add(char** argz, size_t* argz_len, const char* str);
void                      envz_remove(char** envz, size_t* envz_len, const char* name);
error_t                   envz_add(char** envz, size_t* envz_len, const char* name, const char* value)
{
    char* concat   = ((void*)0);
    int   name_len = 0;
    int   val_len  = 0;
    int   retval   = 0;
    envz_remove(envz, envz_len, name);
    if(value)
    {
        name_len = strlen(name);
        val_len  = strlen(value);
        if(!(concat = (char*)malloc(name_len + val_len + 2))) return 12;
        memcpy(concat, name, name_len);
        concat[name_len] = '=';
        memcpy(concat + name_len + 1, value, val_len + 1);
        retval = argz_add(envz, envz_len, concat);
        free(concat);
    }
    else { retval = argz_add(envz, envz_len, name); }
    return retval;
}
