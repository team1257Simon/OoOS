#include "ld-ooos.hpp"
#include <sys/errno.h>
typedef void(*init_fini_fn)();
typedef init_fini_fn* init_fini_vec;
extern "C"
{
    void (**dlinit(void* handle))()
    {
        long result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DLINIT), "D"(handle) : "memory, %r11, %rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); return nullptr; }
        return reinterpret_cast<init_fini_vec>(result);
    }
    void (**dlfini(void* handle))()
    {
        long result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DLFINI), "D"(handle) : "memory, %r11, %rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); return nullptr; }
        return reinterpret_cast<init_fini_vec>(result);
    }
    char** depends(void* handle)
    {
        long result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DEPENDS), "D"(handle) : "memory, %r11, %rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); return nullptr; }
        return reinterpret_cast<char**>(result);
    }
}