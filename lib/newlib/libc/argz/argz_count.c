typedef long unsigned int size_t;
size_t                    argz_count(const char* argz, size_t argz_len)
{
    int    i;
    size_t count = 0;
    for(i = 0; i < argz_len; i++)
    {
        if(argz[i] == '\0') count++;
    }
    return count;
}
