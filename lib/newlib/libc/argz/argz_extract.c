typedef long unsigned int size_t;
size_t                    argz_count(const char* argz, size_t argz_len);
void                      argz_extract(char* argz, size_t argz_len, char** argv)
{
    size_t       i     = 0;
    int          j     = 0;
    const size_t count = argz_count(argz, argz_len);
    if(argz_len > 1)
        for(i = argz_len - 2; i > 0; i--)
        {
            if(argz[i] == '\0')
            {
                j++;
                argv[count - j] = &argz[i + 1];
            }
        }
    argv[0]     = &argz[0];
    argv[count] = ((void*)0);
}
