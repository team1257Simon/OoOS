#ifndef __PWD_H
#define __PWD_H
#include "sys/types.h"
#ifdef __cplusplus
extern "C"
{
#endif
#ifdef __KERNEL__
typedef
#else
#include <sys/types.h>
#endif
struct passwd
{
	char*   pw_name;		/* user name */
	char*   pw_passwd;		/* encrypted password */
	uid_t	pw_uid;			/* user uid */
	gid_t	pw_gid;			/* user gid */
	char*   pw_comment;		/* comment */
	char*   pw_gecos;		/* Honeywell login info */
	char*   pw_dir;		    /* home directory */
	char*   pw_shell;		/* default shell */
}
#ifdef __KERNEL__
unix_pwd;
#else
;
struct passwd*  getpwent(void);
void		 setpwent(void);
void		 endpwent(void);
struct passwd*  getpwuid(uid_t);
struct passwd*  getpwnam(const char *);
#endif
#ifdef __cplusplus
}
#endif
#endif