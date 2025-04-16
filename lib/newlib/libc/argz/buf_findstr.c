typedef long unsigned int size_t;
int                       _buf_findstr(const char* str, char** buf, size_t* buf_len)
{
    int i = 0;
    int j = 0;
    for(i = 0; i < *buf_len; i++)
    {
        if(str[0] == (*buf)[i])
        {
            j = i;
            while(str[j - i] && (str[j - i] == (*buf)[j])) j++;
            if(str[j - i] == '\0')
            {
                *buf += j;
                *buf_len -= j;
                return 1;
            }
        }
    }
    if(i == *buf_len)
    {
        *buf += *buf_len;
        *buf_len = 0;
    }
    return 0;
}
