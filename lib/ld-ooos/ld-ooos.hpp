#ifndef __LD_OOOS
#define __LD_OOOS
#include <stdint.h>
#include <stddef.h>
#define SCV_DLINIT      24
#define SCV_DLOPEN      25
#define SCV_DLCLOSE     26
#define SCV_DLSYM       27
#define SCV_DLRESOLVE   28
#define SCV_DLPATH      29
#define SCV_DLORIGIN    30
#define SCV_DEPENDS     31
#define SCV_DLFINI      32
#define RTLD_LAZY       0x0001
#define RTLD_NOW        0x0002
#define RTLD_NOLOAD	    0x0004
#define RTLD_DEEPBIND   0x0008
#define RTLD_GLOBAL	    0x0100
#define RTLD_NODELETE   0x1000
#define exit(code) asm volatile("syscall" :: "a"(0), "D"(code) : "memory")
typedef void(*init_fn)(int argc, char** argv, char** env);
typedef void(*fini_fn)();
extern "C"
{
    init_fn* dlinit(void* handle);
    fini_fn* dlfini(void* handle);
    char** depends(void* handle);
    int dlpath(const char* path_str);
}
#endif