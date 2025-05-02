#include "syscalls.h"
#ifdef __KERNEL__
#undef __KERNEL__
#endif
void _exit(int code) { asm volatile("syscall" :: "a"(0), "D"(code) : "memory", "%r11", "%rcx"); __builtin_unreachable(); }
DEF_SYSCALL1(int, close, int, fd)
DEF_SYSCALL3(int, execve, char* restrict, name, char** restrict, argv, char** restrict, env)
DEF_SYSCALL0(pid_t, fork)
DEF_SYSCALL0(pid_t, vfork)
DEF_SYSCALL2(int, fstat, int, fd, struct stat*, st)
DEF_SYSCALL2(int, stat, const char* restrict, name, struct stat* restrict, st)
DEF_SYSCALL0(int, getpid)
DEF_SYSCALL1(int, isatty, int, fd)
DEF_SYSCALL2(int, kill, int, pid, int, sig)
DEF_SYSCALL2(int, link, char* restrict, old, char* restrict, new)
DEF_SYSCALL3(int, lseek, int, fd, int, ptr, int, dir)
int open(const char* name, int flags, ...) { int ret; asm volatile("syscall" : "=a"(ret) : "0"(SYSCVEC_N_open), "D"(name), "S"(flags) : "memory", "%r11", "%rcx"); do { if((signed long)(ret) < 0L && (signed long)(ret) > -4095L) { *(__errno()) = -(int)(ret); return (int)(-1); } else return (int)(ret); } while(0); }
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
DEF_SYSCALL2(int, lstat, const char* restrict, name, struct stat* restrict, st)
DEF_SYSCALL3(int, mknod, const char*, path, mode_t, mode, dev_t, dev)
int pipe(int out[2]) { int ret; asm volatile("syscall" : "=a"(ret): "0"(SYSCVEC_N_pipe), "D"(&out[0]) : "memory", "%r11", "%rcx"); if(ret < 0 && ret > -4095) { *__errno() = -ret; return -1; } return ret; }