#include "syscalls.h"
#include "./sys/dirent.h"
#include <string.h>
#include <stdlib.h>
typedef int cmp_t(const void*, const void*);
extern void qsort(void* a, size_t n, size_t es, cmp_t* cmp);
void rewinddir(DIR* dirp) { if(dirp != NULL) dirp->buf_pos = dirp->buf_start; }
int alphasort(const struct dirent** a, const struct dirent** b) { return strncmp((*a)->d_name, (*b)->d_name, 255); }
DIR* opendir(const char* dirname)
{
    DIR* result;
    asm volatile("syscall" : "=a"(result) : "0"(SYSCVEC_N_opendir), "D"(dirname) : "memory", "%r11", "%rcx");
    if((long)result < 0 && (long)result > -4095) { errno = (int)(-1*(long)result); return NULL; }
    return result;
}
DIR* fdopendir(int fd)
{
    DIR* result;
    asm volatile("syscall" : "=a"(result) : "0"(SYSCVEC_N_fdopendir), "D"(fd)  : "memory", "%r11", "%rcx");
    if((long)result < 0 && (long)result > -4095) { errno = (int)(-1*(long)result); return NULL; }
    return result;
}
int closedir(DIR* dir)
{
    if(dir == NULL) { errno = EINVAL; return -1; }
    int result;
    asm volatile("syscall" : "=a"(result) : "0"(SYSCVEC_N_closedir), "D"(dir) : "memory", "%r11", "%rcx");
    if(result < 0 && result > -4095) { errno = -result; return -1; }
    return 0;
}
struct dirent* readdir(DIR* dirp)
{
    if(dirp != NULL)
    {
        if(dirp->buf_pos < dirp->buf_end)
        {
            struct dirent* result = (struct dirent*)dirp->buf_pos;
            dirp->buf_pos = ((char*)dirp->buf_pos) + sizeof(struct dirent);
            return result;
        }
    }
    return NULL;
}
int dirfd(DIR* dirp)
{
    if(dirp)
        return dirp->fd;
    else
        return -1;
}
static int scan_eval(scan_filter_fn scan, struct dirent* ent)
{
    if(!scan) 
        return 1;
    else
        return (*scan)(ent);
}
int scandir(const char* dirp, struct dirent*** namelist, scan_filter_fn scan, scan_compare_fn compare)
{
    int n = 0;
    DIR* dir = opendir(dirp);
    if(!dir) return -1;
    int result = 0;
    struct dirent** names = NULL;
    for(struct dirent* cur = (struct dirent*)dir->buf_start; cur < (struct dirent*)dir->buf_end; ++cur)
    {
        if(scan_eval(scan, cur))
        {
            struct dirent** rnames = (struct dirent**)realloc(names, (n + 1) * sizeof(struct dirent*));
            if(!rnames)
            {
                errno = ENOMEM;
                result = -1;
                goto cleanup;
            }
            names = rnames;
            struct dirent* allocated = (struct dirent*)malloc(sizeof(struct dirent));
            if(!allocated)
            {
                errno = ENOMEM;
                result = -1;
                goto cleanup;
            }
            names[n++] = (struct dirent*)(memcpy(allocated, cur, sizeof(struct dirent)));
        }
    }
    if(result < 0)
    {
        if(names)
        {
            for(int i = 0; i < n; i++)
                free(names[i]);
            free(names);
        }
    }
    else
    {
        if(n && compare)
            qsort(names, n, sizeof(struct dirent*), (cmp_t*)compare);
        *namelist = names;
        result = n;
    }
cleanup:
    closedir(dir);
    return result;
}
void seekdir(DIR* dirp, long loc)
{
    if(dirp)
    {
        struct dirent* st = (struct dirent*)dirp->buf_start;
        if(st + loc < dirp->buf_end) 
            dirp->buf_pos = st + loc;
    }
}
long telldir(DIR* dirp)
{
    if(dirp)
    {
        struct dirent* st = (struct dirent*)dirp->buf_start;
        struct dirent* cur = (struct dirent*)dirp->buf_pos;
        return cur - st;
    }
    return -1L;
}