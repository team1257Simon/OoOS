#include <pwd.h>
#include "syscalls.h"
static struct passwd local_pwent;
static struct passwd local_pwd;
__attribute__((visibility("hidden"))) DEF_SYSCALL1(int, getvpwent, struct passwd*, out)
__attribute__((visibility("hidden"))) DEF_SYSCALL2(int, getvpwnam, const char* restrict, name, struct passwd*, out)
__attribute__((visibility("hidden"))) DEF_SYSCALL2(int, getvpwuid, uid_t, uid, struct passwd*, out)
void setpwent(void) { __builtin_memset(&local_pwent, 0, sizeof(struct passwd)); }
void endpwent(void) {}
struct passwd* getpwent(void)
{
	if(getvpwent(&local_pwent) == 0)
		return &local_pwent;
	return NULL;
}
struct passwd* getpwuid(uid_t uid)
{
	if(getvpwuid(uid, &local_pwd) == 0)
		return &local_pwd;
	return NULL;
}
struct passwd* getpwnam(const char* name)
{
	if(getvpwnam(name, &local_pwd) == 0)
		return &local_pwd;
	return NULL;    
}