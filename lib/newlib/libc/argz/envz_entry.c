
typedef long unsigned int size_t;
int                       _buf_findstr(const char* str, char** buf, size_t* buf_len);
char*                     envz_entry(const char* envz, size_t envz_len, const char* name)
{
    char*  buf_ptr = (char*)envz;
    size_t buf_len = envz_len;
    while(buf_len)
    {
        if(_buf_findstr(name, &buf_ptr, &buf_len))
        {
            if(buf_ptr)
            {
                if(*buf_ptr == '=' || *buf_ptr == '\0')
                {
                    buf_ptr--;
                    while(*buf_ptr != '\0' && buf_ptr != envz) buf_ptr--;
                    if(*buf_ptr == '\0') buf_ptr++;
                    return (char*)buf_ptr;
                }
            }
        }
    }
    return 0;
}
