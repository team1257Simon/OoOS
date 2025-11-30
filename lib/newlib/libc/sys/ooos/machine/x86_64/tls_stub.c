#include <stddef.h>
__attribute__((weak, visibility("hidden"))) void* ____tls_get_addr(void*) { return NULL; }
void* __tls_get_addr(void*) __attribute__((weak, alias("____tls_get_addr")));