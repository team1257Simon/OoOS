int                        ffsl(long i) { return (__builtin_ffsl(i)); }
int                        ffsll(long long i) { return (__builtin_ffsll(i)); }
int                        fls(int i)
{
    if(i == 0) return 0;
    return (sizeof(i) * 8 - __builtin_clz(i));
}
int                        flsll(long long i)
{
    if(i == 0) return 0;
    return (sizeof(i) * 8 - __builtin_clzll(i));
}
int                        flsl(long i)
{
    if(i == 0) return 0;
    return (sizeof(i) * 8 - __builtin_clzl(i));
}
