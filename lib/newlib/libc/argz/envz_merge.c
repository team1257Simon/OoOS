typedef int               error_t;
typedef long unsigned int size_t;
void                      free(void*);
char*                     strdup(const char*);
char*                     strchr(const char*, int);
error_t                   envz_add(char** envz, size_t* envz_len, const char* name, const char* value);
char*                     envz_entry(const char* envz, size_t envz_len, const char* name);
char*                     argz_next(char* argz, size_t argz_len, const char* entry);
error_t                   argz_add(char** argz, size_t* argz_len, const char* str);
error_t                   envz_merge(char** envz, size_t* envz_len, const char* envz2, size_t envz2_len, int override)
{
    char* entry     = ((void*)0);
    char* name_str  = ((void*)0);
    char* val_str   = ((void*)0);
    char* name_iter = ((void*)0);
    int   retval    = 0;
    while((entry = argz_next((char*)envz2, envz2_len, entry)) && !retval)
    {
        if(!override)
        {
            name_str  = strdup(entry);
            name_iter = strchr(name_str, '=');
            if(name_iter) *name_iter = '\0';
            if(!envz_entry(*envz, *envz_len, name_str)) { retval = argz_add(envz, envz_len, entry); }
            free(name_str);
        }
        else
        {
            name_str  = strdup(entry);
            name_iter = strchr(name_str, '=');
            if(name_iter)
            {
                *name_iter = '\0';
                val_str    = name_iter + 1;
            }
            else { val_str = ((void*)0); }
            retval = envz_add(envz, envz_len, name_str, val_str);
            free(name_str);
        }
    }
    return retval;
}
