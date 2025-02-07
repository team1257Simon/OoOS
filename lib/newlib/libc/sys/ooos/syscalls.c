#include "syscalls.h"
#ifdef __KERNEL__
#undef __KERNEL__
#endif
extern void _fini();
void _exit(int code) { _fini(); asm volatile("syscall" :: "a"(0) : "memory"); __builtin_unreachable(); }
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