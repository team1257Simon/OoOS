#include "syscalls.h"
#ifdef __KERNEL__
#undef __KERNEL__
#endif
DEF_SYSCALL1(int, close, int, fd)
DEF_SYSCALL3(int, execve, char* restrict, name, char** restrict, argv, char** restrict, env)
DEF_SYSCALL3(pid_t, spawn, char* restrict, name, char** restrict, argv, char** restrict, env)
DEF_SYSCALL3(int, fexecve, int, fd, char** restrict, argv, char** restrict, env)
DEF_SYSCALL3(pid_t, fspawn, int, fd, char** restrict, argv, char** restrict, env)
DEF_SYSCALL0(pid_t, fork)
DEF_SYSCALL0(pid_t, vfork)
DEF_SYSCALL2(int, fstat, int, fd, struct stat*, st)
DEF_SYSCALL2(int, stat, const char* restrict, name, struct stat* restrict, st)
DEF_SYSCALL0(int, getpid)
DEF_SYSCALL1(int, isatty, int, fd)
DEF_SYSCALL2(int, kill, int, pid, int, sig)
DEF_SYSCALL2(int, link, char* restrict, old, char* restrict, new)
DEF_SYSCALL3(int, lseek, int, fd, int, ptr, int, dir)
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
DEF_SYSCALL1(int, escalate, const char*, pw)
DEF_SYSCALL1(int, setuid, uid_t, id)
DEF_SYSCALL1(int, setgid, gid_t, id)
DEF_SYSCALL0(size_t, threadcount)
void _exit(int code) {
	asm volatile("syscall" :: "a"(SYSCVEC_N_exit), "D"(code) : "memory", "%r11", "%rcx");
	__builtin_unreachable();
}
void exit_thread(void* retval) {
	asm volatile("syscall" :: "a"(SYSCVEC_N_threadexit), "D"(retval) : "memory", "%r11", "%rcx");
	__builtin_unreachable();
}
int open(const char* name, int flags, ...)
{
	int ret;
	asm volatile("syscall" : "=a"(ret) : "0"(SYSCVEC_N_open), "D"(name), "S"(flags) : "memory", "%r11", "%rcx");
	if(__builtin_expect(ret < 0 && ret > -4096, 0))  {
		errno = -ret;
		return -1;
	}
	else return ret;
}
int pipe(int out[2])
{
	int ret;
	asm volatile("syscall" : "=a"(ret): "0"(SYSCVEC_N_pipe), "D"(&out[0]) : "memory", "%r11", "%rcx");
	if(__builtin_expect(ret < 0 && ret > -4096, 0)) {
		errno = -ret;
		return -1;
	}
	return ret;
}
int login(const char* restrict user, const char* restrict pass, login_result* restrict result_out)
{
	asm volatile("syscall" : "=a"(result_out->uid), "=d"(result_out->gid) : "0"(SYSCVEC_N_login), "D"(user), "S"(pass) : "memory", "%r11", "%rcx");
	int ret = (int)result_out->uid;
	if(__builtin_expect(ret < 0 && ret > -4096, 0)) {
		errno = -ret;
		return -1;
	}
	return 0;
}
int impersonate(const char* restrict user, const char* restrict pass, login_result* restrict result_out)
{
	asm volatile("syscall" : "=a"(result_out->uid), "=d"(result_out->gid) : "0"(SYSCVEC_N_impersonate), "D"(user), "S"(pass) : "memory", "%r11", "%rcx");
	int ret = (int)result_out->uid;
	if(__builtin_expect(ret < 0 && ret > -4096, 0)) {
		errno = -ret;
		return -1;
	}
	return 0;
}
login_result urevert()
{
	login_result result;
	asm volatile("syscall" : "=a"(result.uid), "=d"(result.gid) : "0"(SYSCVEC_N_urevert) : "memory", "%r11", "%rcx");
	return result;
}