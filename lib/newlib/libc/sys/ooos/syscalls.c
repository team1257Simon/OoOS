#include "syscalls.h"
#ifdef __KERNEL__
#undef __KERNEL__
#endif
void _exit(int code) { asm volatile("syscall" :: "a"(0), "D"(code) : "memory"); __builtin_unreachable(); }
DEF_SYSCALL1(int, close, int, fd)
DEF_SYSCALL3(int, execve, char*, name, char**, argv, char**, env)
DEF_SYSCALL0(int, fork)
DEF_SYSCALL2(int, fstat, int, fd, struct stat*, st)
DEF_SYSCALL2(int, stat, const char* restrict, name, struct stat* restrict, st)
DEF_SYSCALL0(int, getpid)
DEF_SYSCALL1(int, isatty, int, fd)
DEF_SYSCALL2(int, kill, int, pid, int, sig)
DEF_SYSCALL2(int, link, char* restrict, old, char* restrict, new)
DEF_SYSCALL3(int, lseek, int, fd, int, ptr, int, dir)
int open(const char* name, int flags, ...) { int ret; asm volatile("syscall" : "=a"(ret) : "0"(SYSCVEC_N_open), "D"(name), "S"(flags) : "memory"); do { if((signed long)(ret) < 0L) { *(__errno()) = -(int)(ret); return (int)(-1); } else return (int)(ret); } while(0); }
DEF_SYSCALL3(int, read, int, fd, char*, buf, int, len)
DEF_SYSCALL1(void*, sbrk, int, incr)
DEF_SYSCALL1(clock_t, times, struct tms*, buf)
DEF_SYSCALL1(int, unlink, char*, name)
DEF_SYSCALL1(int, wait, int*, status)
DEF_SYSCALL3(int, write, int, fd, char*, ptr, int, len)
DEF_SYSCALL2(int, gettimeofday, struct timeval* restrict, ts, void*, tz)
DEF_SYSCALL6(void*, mmap, void*, addr, size_t, len, int, prot, int, flags, int, fd, off_t, offset)
DEF_SYSCALL2(int, munmap, void*, addr, size_t, len)
DEF_SYSCALL2(_sig_func_ptr, signal, int, sig, _sig_func_ptr, func)
DEF_SYSCALL3(int, sigprocmask, int, how, sigset_t const* restrict, set, sigset_t* restrict, oset)
DEF_SYSCALL2(int, mkdir, const char*, path, mode_t, mode)
extern void *memalign(size_t alignment, size_t size);
extern int rand();
int posix_memalign(void** memptr, size_t alignment, size_t size)
{
    if(alignment % sizeof(void*) != 0 || __builtin_popcount(alignment) > 1) { *memptr = NULL; return EINVAL; }
    void* result = memalign(alignment, size);
    if(!result) return ENOMEM;
    *memptr = result;
    return 0;
}
int fcntl(int, int, ...)
{
    // NYI
    errno = ENOSYS;
    return -1;
}
int getentropy(void* buffer, unsigned long length)
{
    if(length > 256) { errno = EIO; return -1; }
    void* buf_end = ((unsigned char*)buffer) + length;
    while(buffer < buf_end) { *((unsigned char*)buffer++) = rand() % 256; }
    return 0;
}