#include <stddef.h>
#include <reent.h>
#ifndef __ATTRIBUTE_IMPURE_DATA__
#define __ATTRIBUTE_IMPURE_DATA__
#endif
__attribute__((weak, visibility("hidden"))) void* ____tls_get_addr(void*) { return NULL; }
void* __tls_get_addr(void*) __attribute__((weak, alias("____tls_get_addr")));
static __thread struct _reent __ATTRIBUTE_IMPURE_DATA__ __reent_data;
static __thread int __reent_have_init	= 0;
struct _reent* __getreent()
{
	if(!__reent_have_init) {
		__reent_data		= (struct _reent) _REENT_INIT(__reent_data);
		__reent_have_init	= 1;
	}
	return &__reent_data;
}