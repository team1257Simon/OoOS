#include <sys/stat.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/errno.h>
typedef void (*_sig_func_ptr)(int);
typedef unsigned long sigset_t;
#define SYSCVEC_N_exit			0
#define SYSCVEC_N_sleep			1
#define SYSCVEC_N_wait			2
#define SYSCVEC_N_fork			3
#define SYSCVEC_N_times			4
#define SYSCVEC_N_gettimeofday	5
#define SYSCVEC_N_sbrk			6
#define SYSCVEC_N_open			7
#define SYSCVEC_N_close			8
#define SYSCVEC_N_read			9
#define SYSCVEC_N_write			10
#define SYSCVEC_N_link			11
#define SYSCVEC_N_lseek			12
#define SYSCVEC_N_unlink		13
#define SYSCVEC_N_getpid		14
#define SYSCVEC_N_fstat			15
#define SYSCVEC_N_stat			16
#define SYSCVEC_N_fchmod		17
#define SYSCVEC_N_chmod			18
#define SYSCVEC_N_isatty		19
#define SYSCVEC_N_execve		20
#define SYSCVEC_N_kill			21
#define SYSCVEC_N_mmap			22
#define SYSCVEC_N_munmap		23
#define SYSCVEC_N_signal		36
#define SYSCVEC_N_sigprocmask	38
#define SYSCVEC_N_mkdir			39
#define SYSCVEC_N_opendir		40
#define SYSCVEC_N_fdopendir		41
#define SYSCVEC_N_closedir		42
#define SYSCVEC_N_lstat			43
#define SYSCVEC_N_mknod			44
#define SYSCVEC_N_mknodat		45
#define SYSCVEC_N_vfork			46
#define SYSCVEC_N_spawn			47
#define SYSCVEC_N_pipe			48
#define SYSCVEC_N_getvpwent		49
#define SYSCVEC_N_getvpwuid		50
#define SYSCVEC_N_getvpwnam		51
#define SYSCVEC_N_threadexit	54
#define SYSCVEC_N_tfork			55
#define SYSCVEC_N_threadcreate	56
#define SYSCVEC_N_threadcount	57
#define SYSCVEC_N_threadjoin	58
#define SYSCVEC_N_threaddetach	59
#define SYSCVEC_N_fexecve		60
#define SYSCVEC_N_fspawn		61
#define SYSCVEC_N_getuid		62
#define SYSCVEC_N_getgid		63
#define SYSCVEC_N_login			64
#define SYSCVEC_N_impersonate	65
#define SYSCVEC_N_escalate		66
#define SYSCVEC_N_urevert		67
#define SYSCVEC_N_setuid		68
#define SYSCVEC_N_setgid		69
#ifdef __cplusplus
#ifndef restrict
#define restrict __restrict__
#endif
extern "C" 
{
#endif
void _exit(int code);
int close(int file);
int execve(char* restrict name, char** restrict argv, char** restrict env);
int fexecve(int fd, char** restrict argv, char** restrict env);
pid_t spawn(char* restrict name, char** restrict argv, char** restrict env);
pid_t fspawn(int fd, char** restrict argv, char** restrict env);
pid_t fork();
pid_t vfork();
int fstat(int file, struct stat* st);
int stat(const char* restrict name, struct stat* restrict st);
int getpid();
int isatty(int file);
int kill(int pid, int sig);
int link(char* restrict old, char* restrict __new);
int lseek(int file, int ptr, int dir);
int open(const char* name, int flags, ...);
int read(int file, char* ptr, int len);
void* sbrk(int incr);
clock_t times(struct tms* buf);
int unlink(char* name);
int wait(int* status);
int write(int file, char* ptr, int len);
int gettimeofday(struct timeval* restrict p, void* restrict z);
_sig_func_ptr signal(int sig, _sig_func_ptr func);
int sigprocmask(int how, sigset_t const* restrict set, sigset_t* restrict oset);
int mkdir(const char* path, mode_t mode);
int pipe(int out[2]);
uid_t getuid();
gid_t getgid();
typedef struct {
	uid_t uid;
	gid_t gid;
} login_result;
int login(const char* restrict user, const char* restrict pass, login_result* restrict result_out);
int impersonate(const char* restrict user, const char* restrict pass, login_result* restrict result_out);
int escalate(const char* pw);
login_result urevert();
int setuid(uid_t uid);
int setgid(gid_t gid);
size_t threadcount();
#define SYSCVEC_ARG(name) "0"(SYSCVEC_N_##name)
#define XSYSCALL0(name, ret) asm volatile("syscall" : "=a"(ret) : SYSCVEC_ARG(name) : "memory", "%r11", "%rcx")
#define XSYSCALL1(name, ret, arg0) asm volatile("syscall" : "=a"(ret) : SYSCVEC_ARG(name), "D"(arg0) : "memory", "%r11", "%rcx")
#define XSYSCALL2(name, ret, arg0, arg1) asm volatile("syscall" : "=a"(ret) : SYSCVEC_ARG(name), "D"(arg0), "S"(arg1) : "memory", "%r11", "%rcx")
#define XSYSCALL3(name, ret, arg0, arg1, arg2) asm volatile("syscall" : "=a"(ret) : SYSCVEC_ARG(name), "D"(arg0), "S"(arg1), "d"(arg2) : "memory", "%r11", "%rcx")
#define XSYSCALL6(name, ret, arg0, arg1, arg2, arg3, arg4, arg5) asm volatile("syscall" : "=a"(ret) : SYSCVEC_ARG(name), "D"(arg0), "S"(arg1), "d"(arg2), "r"(arg3), "r"(arg4), "r"(arg5) : "memory", "%r11", "%rcx")
#define SYSCALL_RETVAL(type, ret) do \
{ \
	if(__builtin_expect((signed long)(ret) < 0L && (signed long)(ret) > -4096L, 0)) { errno = -(int)(ret); return (type)(-1); } \
	else return (type)(ret); \
} while(0)
#define DEF_SYSCALL0(rt, name) rt name() \
{ \
	rt ret; \
	XSYSCALL0(name, ret); \
	SYSCALL_RETVAL(rt, ret); \
}
#define DEF_SYSCALL1(rt, name, t1, n1) rt name(t1 n1) \
{ \
	rt ret; \
	XSYSCALL1(name, ret, n1); \
	SYSCALL_RETVAL(rt, ret); \
}
#define DEF_SYSCALL2(rt, name, t1, n1, t2, n2) rt name(t1 n1, t2 n2) \
{ \
	rt ret; \
	XSYSCALL2(name, ret, n1, n2); \
	SYSCALL_RETVAL(rt, ret); \
}
#define DEF_SYSCALL3(rt, name, t1, n1, t2, n2, t3, n3) rt name(t1 n1, t2 n2, t3 n3) \
{ \
	rt ret; \
	XSYSCALL3(name, ret, n1, n2, n3); \
	SYSCALL_RETVAL(rt, ret); \
}
#define DEF_SYSCALL6(rt, name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6) rt name(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6) \
{ \
	rt ret; \
	register t4 r8 asm("r8") = n4; \
	register t5 r9 asm("r9") = n5; \
	register t6 r10 asm("r10") = n6; \
	XSYSCALL6(name, ret, n1, n2, n3, r8, r9, r10); \
	SYSCALL_RETVAL(rt, ret); \
}
#ifdef __cplusplus
}
#endif