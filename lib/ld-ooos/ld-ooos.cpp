#include "ld-ooos.hpp"
const char path_var_str[] = "LD_LIBRARY_PATH=";
constexpr size_t name_str_size = sizeof(path_var_str);
extern "C"
{
    __local int argc;
    __local int errno;
    __local char** argv;
    __local char** env;
    __local dl_action last_error_action;
}
static uint64_t elf64_gnu_hash(const void* data, size_t n) noexcept { const char* cdata = static_cast<const char*>(data); uint32_t h = 5381; for(size_t i = 0; i < n; i++) h += static_cast<uint8_t>(cdata[i]) + (h << 5); return h; }
constexpr bool operator==(link_map const& a, link_map const& b) noexcept { return a.__so_handle == b.__so_handle; }
constexpr bool operator==(link_map const& a, void* b) noexcept { return a.__so_handle == b; }
constexpr bool operator==(void* a, link_map const& b) noexcept { return a == b.__so_handle; }
static uint64_t __hash(link_map const& l) noexcept { return elf64_gnu_hash(&l.__so_handle, sizeof(void*)); }
struct node : link_map { node* chain_next; };
struct res_pair { node* first; bool second; };
static struct
{
    using node_ptr = node*;
    using node_const_ptr = node const*;
    using buckets_ptr = node_ptr*;
    size_t bucket_count;
    size_t element_count;
    buckets_ptr buckets;
    node root;
    size_t after_root_idx;
    buckets_ptr allocate_buckets(size_t count) { return static_cast<buckets_ptr>(allocate(count * sizeof(node_ptr), alignof(node_ptr))); }
    node_ptr advance(node_ptr n) { return n ? n->chain_next : nullptr; }
    node_const_ptr advance(node_const_ptr n) { return n ? n->chain_next : nullptr; }
    size_t idx(node_const_ptr n) const { return bucket_count ? __hash(*n) % bucket_count : 0; }
    bool initialize(size_t nbkt) { buckets = allocate_buckets(nbkt); bucket_count = nbkt; return (buckets != nullptr); }
    void drop_node(node_ptr n) 
    {
        if(!n) return;
        if(n->l_next) n->l_next->l_prev = n->l_prev;
        if(n->l_prev) n->l_prev->l_next = n->l_next;
        deallocate(n, alignof(node));
    }
    node_ptr create_node(void* handle) 
    { 
        node_ptr result = static_cast<node_ptr>(allocate(sizeof(node), alignof(node))); 
        if(result) 
        { 
            result->__so_handle = handle; 
            result->l_prev = root.l_next;
            if(root.l_next) root.l_next->l_next = result;
            root.l_next = result;
        }
        return result; 
    }
    void insert_at(size_t index, node_ptr n) 
    { 
        if(buckets[index]) { n->chain_next = buckets[index]->chain_next; buckets[index]->chain_next = n; } 
        else
        {
            n->chain_next = root.chain_next;
            root.chain_next = n;
            if(n->chain_next) { buckets[after_root_idx] = n; }
            after_root_idx = index;
            buckets[index] = &root;
        }
    }
    void remove_first_at(size_t index, node_ptr n, size_t next)
    {
        if(!n) buckets[index] = nullptr;
        else if(index != next)
        {
            buckets[next] = buckets[index];
            buckets[index] = nullptr;
            if(index == after_root_idx) after_root_idx = next;
        }
    }
    node_ptr find_before(void* handle) 
    { 
        size_t index = bucket_count ? elf64_gnu_hash(&handle, sizeof(void*)) : 0; 
        node_ptr prev = buckets[index];
        for(node_ptr n = advance(prev); n && idx(n) == index; prev = n, n = advance(n)) { if(handle == *n) return prev; }
        return nullptr;
    }
    res_pair add(void* handle)
    {
        if(node_ptr p = find_before(handle)) { return { p, false }; }
        if(node_ptr result = create_node(handle)) { insert_at(idx(result), result); element_count++; return { result, true }; }
        return { nullptr, true };
    }
    node_ptr erase_node(size_t index, node_ptr prev, node_ptr n)
    {
        if(!n) return nullptr;
        if(prev == buckets[index]) { remove_first_at(index, advance(n), n->chain_next ? idx(n->chain_next) : 0); }
        else if(node_ptr subs = advance(n)) { size_t subs_idx = idx(subs); if(subs_idx != index) buckets[subs_idx] = prev; }
        node_ptr result = n->chain_next;
        prev->chain_next = result;
        drop_node(n);
        element_count--;
        return result;
    }
    bool erase(void* what)
    {
        node_ptr prev = find_before(what); 
        if(!prev) return false;
        node_ptr n = advance(prev);
        return n && erase_node(idx(n), prev, n);
    }
    node_ptr find(void* what) { return advance(find_before(what)); }
    node_ptr begin() { return root.chain_next; }
} rtld_map;
static void* __so_open(char* name, int flags)
{
    long result;
    asm volatile("syscall" : "=a"(result) : "0"(SCV_DLOPEN), "D"(name), "S"(flags) : "memory", "%r11", "%rcx");
    if(result < 0 && result > -4096) { errno = static_cast<int>(result * -1); last_error_action = DLA_OPEN; return nullptr; }
    else { return reinterpret_cast<void*>(result); }
}
static bool __finalize(void* handle)
{
    fini_fn* fini = dlfini(handle);
    if(fini) { if(*fini) { for(size_t i = 0; fini[i]; i++) fini[i](); } }
    else { last_error_action = DLA_FINI; return false; }
    return true;
}
static bool __so_close(void* handle)
{
    if(!__finalize(handle)) return false;
    int result;
    asm volatile("syscall" : "=a"(result) : "0"(SCV_DLCLOSE), "D"(handle) : "memory", "%r11", "%rcx");
    if(result < 0 && result > -4096) { errno = result * -1; last_error_action = DLA_CLOSE; return false; }
    rtld_map.erase(handle);
    return true;
}
static bool __load_deps(void* handle)
{
    char** deps = depends(handle);
    if(!deps) { last_error_action = DLA_GETDEP; return false; }
    if(*deps)
    {
        for(size_t i = 0; deps[i]; i++)
        {
            void* so = __so_open(deps[i], RTLD_LAZY | RTLD_PREINIT);
            if(!so) return false;
            auto result = rtld_map.add(so);
            if(!result.second) continue;
            if(dlmap(handle, result.first) < 0) { last_error_action = DLA_LMAP; }
            if(!__load_deps(handle)) return false;
        }
    }
    init_fn* ini = dlinit(handle);
    if(!ini) { last_error_action = DLA_INIT; return false; }
    if(*ini) { for(size_t i = 0; ini[i]; i++) ini[i](argc, argv, env); }
    return true;
}
extern "C"
{
    static int __strncmp(const char* lhs, const char* rhs, size_t n) { size_t i; for(i = 0; i < n && lhs[i] == rhs[i]; i++); return lhs[i] < rhs[i] ? -1 : lhs[i] > rhs[i] ? 1 : 0; }
    int dlbegin(void* phandle, char** __argv, char** __env)
    {
        for(argc = 0; __argv[argc]; argc++);
        argv = __argv;
        env = __env;
        for(char** str = env; *str; str++) { if(!__strncmp(*str, path_var_str, name_str_size)) { int result = dlpath(*str + name_str_size); if(result < 0) { last_error_action = DLA_SETPATH; return errno; } break; } }
        // The kernel will call this function to invoke the dynamic linker. The handle is the program handle to pass to syscalls as part of the setup.
        // argv is the argument string vector, and env is the environment variable string vector. Both are null-terminated (argc is technically redundant).
        // Here we will load all the dependencies of the object (per depends()) before calling its initializers (per dlinit()).
        init_fn* preinit = dlpreinit(phandle);
        if(!preinit) { last_error_action = DLA_PREINIT; return errno; }
        if(*preinit) { for(size_t i = 0; preinit[i]; i++) preinit[i](argc, argv, env); } 
        if(!rtld_map.initialize(256)) return -1;
        if(__load_deps(phandle)) return 0;
        else return errno;
    }
    int dlend(void* phandle)
    {
        // The kernel will call this function to invoke the dynamic linker. The handle is the program handle to pass to syscalls as part of the cleanup.
        // Here we will call the destructors for the object (per dlfini()) before returning to the kernel.
        for(node* l = rtld_map.begin(); l; l = l->chain_next) { __finalize(l->__so_handle); } 
        if(!__finalize(phandle)) { last_error_action = DLA_FINI; return errno; }
        return 0;
    }
    void* dlopen(char* name, int flags)
    {
        void* result = __so_open(name, flags);
        if(result && __load_deps(result)) return result;
        else return nullptr;
    }
    int dlclose(void* handle)
    {
        if(__so_close(handle)) return 0;
        if(last_error_action == DLA_FINI) return 1;
        return -1;
    }
}