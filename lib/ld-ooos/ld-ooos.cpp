#include "ld-ooos.hpp"
#include <sys/errno.h>
#include <string.h>
const char path_var_str[] = "LD_LIBRARY_PATH=";
constexpr size_t name_str_size = sizeof(path_var_str);
std::set<void*> __loaded_obj_handles;
std::vector<link_map*> __map_chain;
std::unordered_map<void*, link_map> __obj_map;
template<typename T> concept boolean_testable = std::convertible_to<T, bool> && std::convertible_to<decltype(!std::declval<T&&>()), bool>;
template<boolean_testable T> constexpr std::add_pointer_t<T> end_of(std::add_pointer_t<T> vec) { size_t i; for(i = 0; vec[i]; i++); return vec + i; }
extern "C"
{
    int argc;
    char** argv;
    char** env;
}
static inline void* __so_open(char* name, int flags)
{
    long result;
    asm volatile("syscall" : "=a"(result) : "0"(SCV_DLOPEN), "D"(name), "S"(flags) : "memory, %r11, %rcx");
    if(result < 0 && result > -4096) { return nullptr; }
    else { return reinterpret_cast<void*>(result); }
}
static bool __load_deps(void* handle)
{
    char** deps = depends(handle);
    if(!deps) { return false; }
    if(*deps)
    {
        for(char* dep : std::vector<char*>(deps, end_of<char*>(deps)))
        {
            void* so = __so_open(dep, RTLD_LAZY | RTLD_PREINIT);
            if(!so) return false;
            auto result = __loaded_obj_handles.insert(so);
            if(!result.second) continue; // already loaded
            void* dhandle = *result.first;
            auto map_iter = __obj_map.emplace(dhandle, link_map{}).first;
            link_map* lm = std::addressof(map_iter->second);
            if(dlmap(dhandle, lm) < 0) { return false; }
            if(!__map_chain.empty()) { lm->l_prev = __map_chain.back(); }
            __map_chain.push_back(lm);
            if(!__load_deps(dhandle)) return false;
        }
    }
    init_fn* ini = dlinit(handle);
    if(!ini) { return false; }
    if(*ini) { for(init_fn f : std::vector<init_fn>(ini, end_of<init_fn>(ini))) f(argc, argv, env); }
    return true;
}
extern "C"
{
    void dlbegin(void* phandle, char** __argv, char** __env)
    {
        argc = end_of<char*>(__argv) - __argv;
        argv = __argv;
        env = __env;
        for(char** str = env; *str; str++) { if(!strncmp(*str, path_var_str, name_str_size)) { int result = dlpath(*str + name_str_size); if(result < 0) { exit(errno); __builtin_unreachable(); } break; } }
        // The kernel will call this function to invoke the dynamic linker. The handle is the program handle to pass to syscalls as part of the setup.
        // argv is the argument string vector, and env is the environment variable string vector. Both are null-terminated (argc is technically redundant)
        // Here we will load all the dependencies of the object (per depends()) before calling its initializers (per dlinit()).
        init_fn* preinit = dlpreinit(phandle);
        if(!preinit) { exit(errno); __builtin_unreachable(); }
        if(*preinit) { for(init_fn f : std::vector<init_fn>(preinit, end_of<init_fn>(preinit))) f(argc, argv, env); }
        if(__load_deps(phandle)) exit(0);
        else exit(errno);
    }
    void dlend(void* phandle)
    {
        // The kernel will call this function to invoke the dynamic linker. The handle is the program handle to pass to syscalls as part of the cleanup.
        // Here we will call the destructors for the object (per dlfini()) before returning to the kernel.
        for(void* handle : __loaded_obj_handles) { if(fini_fn* fini = dlfini(handle)) { if(*fini) { for(fini_fn f : std::vector<fini_fn>(fini, end_of<fini_fn>(fini))) f(); } } else { exit(errno); __builtin_unreachable(); } }
        if(fini_fn* fini = dlfini(phandle)) { if(*fini) { for(fini_fn f : std::vector<fini_fn>(fini, end_of<fini_fn>(fini))) f(); } } 
        else { exit(errno); __builtin_unreachable(); }
        exit(0);
    }
    void* dlopen(char* name, int flags)
    {
        void* result = __so_open(name, flags);
        if(result && __load_deps(result)) return result;
        else return nullptr;
    }
}