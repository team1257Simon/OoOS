#include <stddef.h>
static char dlfcn_error[] = "Service unavailable";
void* dlopen(const char* name, int mode) __attribute__((weak, alias("__dlopen")));
int dlclose(void* fd) __attribute__((weak, alias("__dlclose")));
void* dlsym(void* handle, const char* name) __attribute__((weak, alias("__dlsym")));
char* dlerror(void) __attribute__((weak, alias("__dlerror")));
int dladdr(const void* addr, void* dli) __attribute__((weak, alias("__dladdr")));
int dlinfo(void* handle, int req, void* out) __attribute__((weak, alias("__dlinfo")));
__attribute__((weak, visibility("hidden"))) void* __dlopen(const char* name, int mode) { return NULL; }
__attribute__((weak, visibility("hidden"))) int __dlclose(void* fd) { return -1; }
__attribute__((weak, visibility("hidden"))) void* __dlsym(void* handle, const char* name) { return NULL; }
__attribute__((weak, visibility("hidden"))) char* __dlerror(void) { return dlfcn_error; }
__attribute__((weak, visibility("hidden"))) int __dladdr(const void* addr, void* dli) { return 0; }
__attribute__((weak, visibility("hidden"))) int __dlinfo(void* handle, int req, void* out) { return -1; }