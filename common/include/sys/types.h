#ifndef __TYPES
#define __TYPES
#ifdef __KERNEL__
#include "kernel/kernel_defs.h"
typedef vaddr_t caddr_t;
#else
#include "stdint.h"
#include "stddef.h"
typedef void* caddr_t;
#endif
typedef long ssize_t;
typedef	unsigned short	ushort;		/* System V compatibility */
typedef	unsigned int	uint;		/* System V compatibility */
typedef	unsigned long	ulong;		/* System V compatibility */
typedef long register_t;
typedef __int128_t int128_t;
typedef uint64_t time_t, clock_t, suseconds_t, nlink_t, uid_t, gid_t, blksize_t, blkcnt_t, ino_t;
typedef uint32_t dev_t;
typedef ptrdiff_t off_t;
typedef uint32_t mode_t;
typedef uint64_t pid_t;
#endif