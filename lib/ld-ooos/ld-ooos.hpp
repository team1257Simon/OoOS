#ifndef __LD_OOOS
#define __LD_OOOS
#include <stdint.h>
#include <stddef.h>
#define __local __attribute__((visibility("hidden")))
#define SCV_DLPREINIT   24
#define SCV_DLINIT      25
#define SCV_DLOPEN      26
#define SCV_DLCLOSE     27
#define SCV_DLSYM       28
#define SCV_DLRESOLVE   29
#define SCV_DLPATH      30
#define SCV_DLMAP       31
#define SCV_DEPENDS     32
#define SCV_DLADDR      33
#define SCV_DLFINI      34
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
struct dl_info // I refuse to use the ugly caps-with-underscores in my internal headers, sue me
{
    const char* dli_fname;
    void*       dli_fbase;
    const char* dli_sname;
    void*       dli_saddr;
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
    void** __global_offset_table;
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
    DLA_GETINFO = 8,
    DLA_FINI    = 9,
    DLA_CLOSE   = 10
};
enum dlinfo_req : int
{
    RTLD_DI_LMID        = 1,
    RTLD_DI_LINKMAP     = 2,
    RTLD_DI_CONFIGADDR  = 3,
    RTLD_DI_SERINFO     = 4,
    RTLD_DI_SERINFOSIZE = 5,
    RTLD_DI_ORIGIN      = 6,
    RTLD_DI_PROFILENAME = 7,
    RTLD_DI_PROFILEOUT  = 8,
    RTLD_DI_TLS_MODID   = 9,
    RTLD_DI_TLS_DATA    = 10,
    RTLD_DI_MAX         = 10
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
    char* __strterm(const char*);
}
__local void deallocate(void* ptr, size_t al);
__local void* allocate(size_t count, size_t al);
#endif