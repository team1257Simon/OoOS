#include "ld-ooos.hpp"
extern "C"
{
    init_fn* dlinit(void* handle)
    {
        long result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DLINIT), "D"(handle) : "memory", "%r11", "%rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); }
        else { return reinterpret_cast<init_fn*>(result); }
        return nullptr;
    }
    fini_fn* dlfini(void* handle)
    {
        long result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DLFINI), "D"(handle) : "memory", "%r11", "%rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); }
        else { return reinterpret_cast<fini_fn*>(result); }
        return nullptr;
    }
    char** depends(void* handle)
    {
        long result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DEPENDS), "D"(handle) : "memory", "%r11", "%rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); }
        else { return reinterpret_cast<char**>(result); }
        return nullptr;
    }
    int dlpath(const char* path_str)
    {
        int result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DLPATH), "D"(path_str) : "memory", "%r11", "%rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); }
        return result;
    }
    int dlmap(void* handle, link_map* lm)
    {
        int result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DLMAP), "D"(handle), "S"(lm) : "memory", "%r11", "%rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); }
        return result;
    }
    init_fn* dlpreinit(void* handle)
    {
        long result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DLPREINIT), "D"(handle) : "memory", "%r11", "%rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); }
        else { return reinterpret_cast<init_fn*>(result); }
        return nullptr;
    }
    int dladdr(const void* addr, dl_info* info)
    {
        int result;
        asm volatile("syscall" : "=a"(result) : "0"(SCV_DLADDR), "D"(addr), "S"(info) : "memory", "%r11", "%rcx");
        if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); }
        return result;
    }
}