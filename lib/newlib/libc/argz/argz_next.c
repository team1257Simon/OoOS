
typedef long unsigned int size_t;
char*                     argz_next(char* argz, size_t argz_len, const char* entry)
{
    if(entry)
    {
        while(*entry != '\0') entry++;
        entry++;
        if(entry >= argz + argz_len)
            return ((void*)0);
        else
            return (char*)entry;
    }
    else
    {
        if(argz_len > 0)
            return (char*)argz;
        else
            return ((void*)0);
    }
}
