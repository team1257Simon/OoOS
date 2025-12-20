#ifndef _SYS__PTHREADTYPES_H_
#define	_SYS__PTHREADTYPES_H_
#include <sys/types.h>
#ifdef __cplusplus
extern "C"
{
#endif
typedef pid_t pthread_t;
typedef struct { volatile unsigned char : 8; } pthread_spinlock_t;
typedef const struct { volatile unsigned long : 64; } private_data_qword;
typedef struct {
	size_t stack_size;
	unsigned char detach_state;
} pthread_attr_t;
typedef struct
{
	void* current_holder;
	pthread_spinlock_t lock;
	unsigned char type;
	unsigned char recursive;
} pthread_mutex_t;
typedef enum
{
	TS_STOPPED		= 0,
	TS_RUNNING		= 1,
	TS_CANCELLED	= 2,
	TS_TERMINATED	= 3,
} thread_state_t;
struct ooos_pthread_ctl
{
	volatile struct
	{
		thread_state_t execution_state				: 2;
		unsigned char park							: 1;
		unsigned char untimed_park					: 1;
		unsigned char detach_state					: 1;
		unsigned char cancel_state					: 1;
		unsigned char cancel_type					: 1;
		const unsigned char callback_reset_type		: 1;
	};
	struct
	{
		pthread_spinlock_t thread_mutex;
		pthread_t thread_id;
		private_data_qword wait_time_info;
		private_data_qword callback_reset_info;
		void* const callback_handle;
	};
};
#ifdef __cplusplus
}
#endif
#endif