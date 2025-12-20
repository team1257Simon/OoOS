#ifndef __PTHREAD_H
#define __PTHREAD_H
#include <sys/pthread.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct __attribute__((aligned(16))) { private_data_qword data[64]; } thread_fp_state_t;
typedef struct __attribute__((packed, aligned(8)))
{
	uintptr_t		rax;
	uintptr_t		rbx;
	uintptr_t		rcx;
	uintptr_t		rdx;
	uintptr_t		rdi;
	uintptr_t		rsi;
	uintptr_t		r8;
	uintptr_t		r9;
	uintptr_t		r10;
	uintptr_t		r11;
	uintptr_t		r12;
	uintptr_t		r13;
	uintptr_t		r14;
	uintptr_t		r15;
	uintptr_t		rbp;
	uintptr_t		rsp;
	uintptr_t		rip;
	const uintptr_t	cr3;
	uintptr_t		rflags;
	const uint16_t	ds;
	const uint16_t	ss;
	const uint16_t	cs;
} thread_cpu_state_t;
struct __attribute__((aligned(16))) pthread
{
	struct pthread* self_ptr;
	private_data_qword dtv_ptr;
	thread_fp_state_t fp_state;
	thread_cpu_state_t cpu_state;
	struct ooos_pthread_ctl ctl_info;
	size_t stack_size;
	private_data_qword stack_base_ptr;
	private_data_qword tls_base_ptr;
};
int pthread_attr_init(pthread_attr_t*);
int pthread_create(pthread_t*, pthread_attr_t const*, void* (*)(void*), void*);
int pthread_detach(pthread_t);
int pthread_join(pthread_t, void**);
void pthread_exit(void*);
struct pthread* pthread_self();
#ifdef __cplusplus
}
#endif
#endif