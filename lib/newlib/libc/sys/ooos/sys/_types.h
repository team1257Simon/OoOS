#ifndef __OOOS_TYPES
#define __OOOS_TYPES
#include <machine/_types.h>
typedef unsigned long __blkcnt_t;
typedef unsigned long __blksize_t;
typedef unsigned long __fsblkcnt_t;
typedef unsigned int __fsfilcnt_t;
typedef long _off_t;
typedef int __pid_t;
typedef int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned int __id_t;
typedef unsigned long __ino_t;
typedef unsigned int __mode_t;
__extension__ typedef long long _off64_t;
typedef _off_t __off_t;
typedef _off64_t __loff_t;
typedef long __key_t;
typedef long _fpos_t;
#ifndef __cplusplus
typedef unsigned long __size_t;
#endif
typedef signed long _ssize_t;
struct __lock;
typedef struct __lock *_flock_t;
typedef void *_iconv_t;
typedef	unsigned long	__clock_t;
typedef	long __time_t;
#define _TIME_T_ long
#define _CLOCK_T_ unsigned long
typedef	unsigned long __clockid_t;
typedef	unsigned long	__timer_t;
typedef	unsigned char __sa_family_t;
typedef	unsigned int	__socklen_t;
typedef	int		__nl_item;
typedef	unsigned long	__nlink_t;
typedef	long		__suseconds_t;
typedef	unsigned long	__useconds_t;
#ifdef __GNUCLIKE_BUILTIN_VARARGS
typedef	__builtin_va_list	__va_list;
#else
typedef	char *			__va_list;
#endif /* __GNUCLIKE_BUILTIN_VARARGS */
#define __need_wint_t
#include <stddef.h>
#ifndef __machine_mbstate_t_defined
typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    unsigned char __wchb[4];
  } __value;
} _mbstate_t;
#endif
#endif