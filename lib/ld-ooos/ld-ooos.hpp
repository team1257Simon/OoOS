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
#define exit(code) asm volatile("syscall" :: "a"(0), "D"(code) : "memory")
extern "C"
{
    void (**dlinit(void* handle))();
    void (**dlfini(void* handle))();
    char** depends(void* handle);
}
#endif