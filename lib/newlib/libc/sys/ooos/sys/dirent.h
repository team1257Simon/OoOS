#ifndef __DIRENT
#define __DIRENT
#include <sys/types.h>
#ifdef __cplusplus
extern "C" 
{
#endif
struct dirent
{
    long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[256];
};
typedef struct
{
    int fd;
    void* buf_start;
    void* buf_pos;
    void* buf_end;
} DIR;
enum
{
    DT_UNKNOWN = 0,
    DT_FIFO = 1,
    DT_CHR = 2,
    DT_DIR = 4,
    DT_BLK = 6,
    DT_REG = 8,
    DT_LNK = 10,
    DT_SOCK = 12,
    DT_WHT = 14
};
typedef int (*scan_filter_fn)(const struct dirent*);
typedef int (*scan_compare_fn)(const struct dirent**, const struct dirent**);
int alphasort(const struct dirent** a, const struct dirent** b);
DIR* fdopendir(int fd);
DIR* opendir(const char* dirname);
int closedir(DIR* dir);
struct dirent* readdir(DIR* dirp);
void rewinddir(DIR* dirp);
int dirfd(DIR* dirp);
int scandir(const char* dirp, struct dirent*** namelist, scan_filter_fn scan, scan_compare_fn compare);
void seekdir(DIR *dirp, long loc);
long telldir(DIR *dirp);
#ifdef __cplusplus
}
#endif
#endif