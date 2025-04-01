#include "ld-ooos.hpp"
#include <sys/errno.h>
extern "C"
{
    init_fn* dlinit(void* handle)
    {
        long result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DLINIT), "D"(handle) : "memory, %r11, %rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); return nullptr; }
        return reinterpret_cast<init_fn*>(result);
    }
    fini_fn* dlfini(void* handle)
    {
        long result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DLFINI), "D"(handle) : "memory, %r11, %rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); return nullptr; }
        return reinterpret_cast<fini_fn*>(result);
    }
    char** depends(void* handle)
    {
        long result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DEPENDS), "D"(handle) : "memory, %r11, %rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); return nullptr; }
        return reinterpret_cast<char**>(result);
    }
    int dlpath(const char* path_str)
    {
        int result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DLPATH), "D"(path_str) : "memory, %r11, %rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); }
        return result;
    }
}