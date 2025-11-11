typedef unsigned long size_t;
int strcmp(const char* s1, const char* s2)
{
    unsigned long* a1;
    unsigned long* a2;
    if(!(((long)s1 & (sizeof(long) - 1)) | ((long)s2 & (sizeof(long) - 1))))
    {
        a1 = (unsigned long*)s1;
        a2 = (unsigned long*)s2;
        while(*a1 == *a2)
        {
            if((((*a1) - 0x0101010101010101) & ~(*a1) & 0x8080808080808080)) return 0;
            a1++;
            a2++;
        }
        s1 = (char*)a1;
        s2 = (char*)a2;
    }
    while(*s1 != '\0' && *s1 == *s2)
    {
        s1++;
        s2++;
    }
    return (*(unsigned char*)s1) - (*(unsigned char*)s2);
}
char* strcpy(char* dst0, const char* src0)
{
    char*       dst = dst0;
    const char* src = src0;
    long*       aligned_dst;
    const long* aligned_src;
    if(!(((long)src & (sizeof(long) - 1)) | ((long)dst & (sizeof(long) - 1))))
    {
        aligned_dst = (long*)dst;
        aligned_src = (long*)src;
        while(!(((*aligned_src) - 0x0101010101010101) & ~(*aligned_src) & 0x8080808080808080)) { *aligned_dst++ = *aligned_src++; }
        dst = (char*)aligned_dst;
        src = (char*)aligned_src;
    }
    while((*dst++ = *src++));
    return dst0;
}
unsigned long strnlen(const char* str, size_t n)
{
    const char* start = str;
    while(n-- > 0 && *str) str++;
    return str - start;
}
void* __attribute__((__optimize__("-fno-tree-loop-distribute-patterns"))) memmove(void* dst_void, const void* src_void, size_t length)
{
    char*       dst = dst_void;
    const char* src = src_void;
    long*       aligned_dst;
    const long* aligned_src;
    if(src < dst && dst < src + length)
    {
        src += length;
        dst += length;
        while(length--) { *--dst = *--src; }
    }
    else
    {
        if(!((length) < (sizeof(long) << 2)) && !(((long)src & (sizeof(long) - 1)) | ((long)dst & (sizeof(long) - 1))))
        {
            aligned_dst = (long*)dst;
            aligned_src = (long*)src;
            while(length >= (sizeof(long) << 2))
            {
                *aligned_dst++ = *aligned_src++;
                *aligned_dst++ = *aligned_src++;
                *aligned_dst++ = *aligned_src++;
                *aligned_dst++ = *aligned_src++;
                length -= (sizeof(long) << 2);
            }
            while(length >= (sizeof(long)))
            {
                *aligned_dst++ = *aligned_src++;
                length -= (sizeof(long));
            }
            dst = (char*)aligned_dst;
            src = (char*)aligned_src;
        }
        while(length--) { *dst++ = *src++; }
    }
    return dst_void;
}