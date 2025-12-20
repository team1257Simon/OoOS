#include <assert.h>
#include "./include/pthread.h"
#include "syscalls.h"
extern void thread_landing();
extern void exit_thread(void*);
void pthread_exit(void* retval) { exit_thread(retval); }
static const pthread_attr_t attr_default =
{
	.stack_size		= 4096 * 4,
	.detach_state	= 0
};
struct pthread* pthread_self()
{
	struct pthread* result = NULL;
	asm volatile("movq %%fs:0, %0" : "=r"(result) :: "memory");
	return result;
}
int pthread_attr_init(pthread_attr_t* attr) {
	*attr	= attr_default;
	return 0;
}
static pthread_t __sys_threadcreate(void* (*entry)(void*), void* restrict arg, pthread_attr_t const* restrict attr)
{
	if(!attr) attr = &attr_default;
	long result;
	register int __r8 asm("r8")		= attr->detach_state;
	register void* __r9 asm("r9")	= arg;
	asm volatile("syscall" : "=a"(result) : "0"(SYSCVEC_N_threadcreate), "D"(entry), "S"(thread_landing), "d"(attr->stack_size), "r"(__r8), "r"(__r9) : "%rcx", "%r11", "memory");
	return (pthread_t)(result);
}
int pthread_create(pthread_t* restrict out_ptr, pthread_attr_t const* restrict attr, void* (*entry)(void*), void* restrict arg)
{
	assert(out_ptr != NULL);
	assert(entry != NULL);
	pthread_t result	= __sys_threadcreate(entry, arg, attr);
	long sresult		= (long)result;
	if(__builtin_expect(sresult < 0 && sresult > -4096L, 0))
		return (errno	= (int)(-sresult));
	*out_ptr			= result;
	return 0;
}
int pthread_detach(pthread_t id)
{
	// The call to threaddetach will return a positive value if the thread is locked/busy, or 0 if the detach is successful.
	// This positive return result signals us to keep trying until the thread is available, simulating a spinlock.
	int result;
	do {
		asm volatile("syscall" : "=a"(result) : "0"(SYSCVEC_N_threaddetach), "D"(id) : "%rcx", "%r11", "memory");
		if(__builtin_expect(result < 0 && result > -4096, 0)) {
			errno = -result;
			return -1;
		}
	} while(result > 0);
	return 0;
}
int pthread_join(pthread_t id, void** out)
{
	long result;
	asm volatile("syscall" : "=a"(result) : "0"(SYSCVEC_N_threadjoin), "D"(id) : "%rcx", "%r11", "memory");
	if(__builtin_expect(result < 0L && result > -4096L, 0))
		return (errno	= (int)(-result));
	if(out) *out		= (void*)result;
	return 0;
}
DEF_SYSCALL0(pthread_t, tfork)