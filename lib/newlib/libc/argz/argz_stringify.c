
typedef long unsigned int size_t;
void                      argz_stringify(char* argz, size_t argz_len, int sep)
{
    size_t i;
    if(argz_len > 1)
        for(i = 0; i < argz_len - 1; i++)
        {
            if(argz[i] == '\0') argz[i] = sep;
        }
}
