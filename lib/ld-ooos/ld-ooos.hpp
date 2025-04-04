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
#define SCV_DLMAP       30
#define SCV_DEPENDS     31
#define SCV_DLFINI      32
#define SCV_DLPREINIT   33
#define RTLD_LAZY       0x0001
#define RTLD_NOW        0x0002
#define RTLD_NOLOAD	    0x0004
#define RTLD_PREINIT    0x0010
#define RTLD_DEEPBIND   0x0008
#define RTLD_GLOBAL	    0x0100
#define RTLD_NODELETE   0x1000
#define exit(code) asm volatile("syscall" :: "a"(0), "D"(code) : "memory")
typedef void(*init_fn)(int argc, char** argv, char** env);
typedef void(*fini_fn)();
struct elf64_dyn
{
    int64_t d_tag;
    union [[gnu::may_alias]]
    {
        uint64_t d_val;
        uintptr_t d_ptr;
    };
};
struct link_map
{
	uintptr_t l_addr;
    char* l_name;
    elf64_dyn* l_ld;
    link_map* l_next;
    link_map* l_prev;
    size_t __ld_len;
    void* __so_handle;
};
enum dl_action : int
{
    DLA_NONE    = 0,
    DLA_GETDEP  = 1,
    DLA_SETPATH = 2,
    DLA_PREINIT = 3,
    DLA_OPEN    = 4,
    DLA_INIT    = 5,
    DLA_RESOLVE = 6,
    DLA_LMAP    = 7,
    DLA_FINI    = 8,
    DLA_CLOSE   = 9
};
extern "C"
{
    extern dl_action last_error_action;
    extern int errno;
    init_fn* dlinit(void* handle);
    init_fn* dlpreinit(void* handle);
    fini_fn* dlfini(void* handle);
    char** depends(void* handle);
    int dlpath(const char* path_str);
    int dlmap(void* handle, link_map* lm);
    void* __copy(void* dest, const void* src, size_t n);
    void __zero(void*, size_t);
}
#endif