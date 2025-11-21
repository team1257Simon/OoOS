#include "ld-ooos.hpp"
static const char path_var_str[]    { "LD_LIBRARY_PATH=" };
constexpr size_t  name_str_size     { sizeof(path_var_str) };
static char       dl_error_str[54]  {};
extern "C"
{
	__hidden int       argc;
	__hidden int       errno;
	__hidden char**    argv;
	__hidden char**    env;
	__hidden dl_action last_error_action;
}
static size_t __strlen(const char* str) { return static_cast<size_t>(__strterm(str) - str); } // behaves like strnlen(str, 256) in this implementation
static int __strncmp(const char* lhs, const char* rhs, size_t n)
{
	size_t i;
	for(i = 0; i < n && lhs[i] == rhs[i]; i++);
	return lhs[i] < rhs[i] ? -1 : lhs[i] > rhs[i] ? 1 : 0;
}
static uint64_t elf64_gnu_hash(const void* data, size_t n) noexcept
{
	const char* cdata = static_cast<const char*>(data);
	uint32_t    h     = 5381;
	for(size_t i = 0; i < n && cdata[i]; i++) h += static_cast<uint8_t>(cdata[i]) + (h << 5);
	return h;
}
constexpr bool  operator==(link_map const& a, link_map const& b) noexcept { return a.__so_handle == b.__so_handle; }
constexpr bool  operator==(link_map const& a, void* b) noexcept { return a.__so_handle == b; }
constexpr bool  operator==(void* a, link_map const& b) noexcept { return a == b.__so_handle; }
struct node : link_map { node* chain_next; };
struct res_pair { node* first; bool  second; };
static struct
{
	using node_ptr       = node*;
	using node_const_ptr = node const*;
	using buckets_ptr    = node_ptr*;
	size_t         bucket_count;
	size_t         element_count;
	buckets_ptr    buckets;
	node           root;
	size_t         after_root_idx;
	node_ptr       advance(node_ptr n) { return n ? n->chain_next : nullptr; }
	size_t         idx(node_const_ptr n) const { return bucket_count ? elf64_gnu_hash(&n->__so_handle, sizeof(void*)) % bucket_count : 0; }
	buckets_ptr    allocate_buckets(size_t count)
	{
		buckets_ptr result = static_cast<buckets_ptr>(allocate(count * sizeof(node_ptr), alignof(node_ptr)));
		__zero(result, count * sizeof(node_ptr));
		return result;
	}
	bool initialize(size_t nbkt)
	{
		buckets      = allocate_buckets(nbkt);
		bucket_count = nbkt;
		return (buckets != nullptr);
	}
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
			result->l_prev      = root.l_next;
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
			n->chain_next   = root.chain_next;
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
			buckets[next]  = buckets[index];
			buckets[index] = nullptr;
			if(index == after_root_idx) after_root_idx = next;
		}
	}
	node_ptr find_before(void* handle)
	{
		size_t   index = bucket_count ? elf64_gnu_hash(&handle, sizeof(void*)) % bucket_count : 0;
		node_ptr prev  = buckets[index];
		for(node_ptr n = advance(prev); n && idx(n) == index; prev = n, n = advance(n)) { if(handle == *n) return prev; }
		return nullptr;
	}
	res_pair add(void* handle)
	{
		if(node_ptr p = find_before(handle)) { return { p, false }; }
		if(node_ptr result = create_node(handle))
		{
			insert_at(idx(result), result);
			element_count++;
			return { result, true };
		}
		return { nullptr, true };
	}
	node_ptr erase_node(size_t index, node_ptr prev, node_ptr n)
	{
		if(!n) return nullptr;
		if(prev == buckets[index]) { remove_first_at(index, advance(n), n->chain_next ? idx(n->chain_next) : 0); }
		else if(node_ptr subs = advance(n)) { size_t subs_idx = idx(subs); if(subs_idx != index) buckets[subs_idx] = prev; }
		node_ptr result  = n->chain_next;
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
	if(result < 0 && result > -4096)
	{
		errno             = static_cast<int>(result * -1);
		last_error_action = DLA_OPEN;
		return nullptr;
	}
	else { return reinterpret_cast<void*>(result); }
}
static bool __finalize(void* handle)
{
	fini_fn* fini = dlfini(handle);
	if(__builtin_expect(!fini, false)) { last_error_action = DLA_FINI; return false; }
	for(size_t i = 0; fini[i]; i++) fini[i]();
	return true;
}
static bool __so_close(void* handle)
{
	if(!__finalize(handle)) return false;
	int result;
	asm volatile("syscall" : "=a"(result) : "0"(SCV_DLCLOSE), "D"(handle) : "memory", "%r11", "%rcx");
	if(result < 0 && result > -4096)
	{
		errno             = result * -1;
		last_error_action = DLA_CLOSE;
		return false;
	}
	rtld_map.erase(handle);
	return true;
}
static bool __load_deps(void* handle)
{
	char** deps = depends(handle);
	if(!deps) { last_error_action = DLA_GETDEP; return false; }
	for(size_t i = 0; deps[i]; i++)
	{
		void* so = __so_open(deps[i], RTLD_LAZY | RTLD_PREINIT);
		if(!so) return false;
		res_pair result = rtld_map.add(so);
		if(!result.second) continue;
		if(__builtin_expect(dlmap(handle, result.first) < 0, false)) { last_error_action = DLA_LMAP; }
		const char* name_str = result.first->l_name;
		if(name_str)
		{
			size_t len = __strlen(name_str);
			result.first->l_name = static_cast<char*>(allocate(len + 1, 8));
			if(!result.first->l_name) { errno = ENOMEM; return false; }
			__copy(result.first->l_name, name_str, len);
			result.first->l_name[len] = 0;
		}
		if(__builtin_expect(!__load_deps(so), false)) return false;
	}
	init_fn* ini = dlinit(handle);
	if(__builtin_expect(!ini, false)) { last_error_action = DLA_INIT; return false; }
	for(size_t i = 0; ini[i]; i++) { ini[i](argc, argv, env); }
	return true;
}
static const char* __get_dl_error() 
{
	if(errno == ENOMEM) return "not enough memory";
	if(errno == EINVAL) return "a pointer argument was invalid";
	if(errno == EBADF) return "a handle argument was invalid";
	switch(last_error_action)
	{
		case DLA_INIT:
			switch(errno)
			{
				case ENOENT:
					return "the system cound not resolve a required object symbol";
				case ELIBBAD:
					return "a required library is corrupted";
				default:
					break;
			}
			break;
		case DLA_OPEN:
			if(errno == ENOENT) { return "shared object could not be found"; }
			break;
		case DLA_RESOLVE:
			switch(errno)
			{
				case ENOEXEC:
					return "the current process image is corrupted";
				case ELIBSCN:
					return "a required library is corrupted";
				case ELIBACC:
					return "the system could not resolve a required function symbol";
				case ENOENT:
					return "the system could not find the requested symbol";
				default:
					break;
			}
			break;
		default:
			if(errno == ENOENT) { return "the symbol requested does not exist"; }
			break;
	}
	if(errno == ENOSYS) return "libdl is not supported with static executables";
	return "unknown or invalid error code";
}
extern "C"
{
	
	__hidden int dlbegin(void* phandle, char** __argv, char** __env)
	{
		// The kernel will call this function to invoke the dynamic linker.
		// The handle is the program handle to pass to syscalls as part of the setup.
		// Parameter argv is the argument string vector, and env is the environment variable string vector.
		// Both are null-terminated (argc is technically redundant).
		// Here we will check the environment variables for additional path entries.
		// Then, we load all the dependencies of the object (per depends()) before calling its initializers (per dlinit()).
		for(argc = 0; __argv[argc]; argc++);
		argv = __argv;
		env  = __env;
		for(char** str = env; *str; str++)
		{
			if(!__strncmp(*str, path_var_str, name_str_size))
			{
				int result = dlpath(*str + name_str_size);
				if(__builtin_expect(result < 0, false)) { last_error_action = DLA_SETPATH; return errno; }
				break;
			}
		}
		alloc_init();
		init_fn* preinit = dlpreinit(phandle);
		if(__builtin_expect(!preinit, false)) { last_error_action = DLA_PREINIT; return errno; }
		for(size_t i = 0; preinit[i]; i++) preinit[i](argc, argv, env);
		if(__builtin_expect(!rtld_map.initialize(256), false)) return -1;
		if(__builtin_expect(__load_deps(phandle) && (tlinit() == 0), true)) return 0;
		else return errno;
	}
	__hidden [[noreturn]] void dlend(void* phandle)
	{
		// The kernel will call this function to invoke the dynamic linker.
		// The handle is the program handle to pass to syscalls as part of the cleanup.
		// Here we will call the destructors for the object (per dlfini()) before returning to the kernel.
		for(node* l = rtld_map.begin(); l; l = l->chain_next) { __finalize(l->__so_handle); }
		if(__builtin_expect(!__finalize(phandle), false)) { last_error_action = DLA_FINI; exit(errno); }
		exit(0);
		__builtin_unreachable();
	}
	void* dlopen(char* name, int flags)
	{
		void* result = __so_open(name, flags & ~(RTLD_PREINIT));
		if(__builtin_expect(result && __load_deps(result), true)) return result;
		else return nullptr;
	}
	void* dlsym(void* handle, const char* symbol)
	{
		long result;
		asm volatile("syscall" : "=a"(result) : "0"(SCV_DLSYM), "D"(handle), "S"(symbol) : "memory", "%r11", "%rcx");
		if(__builtin_expect(result < 0 && result > -4096, false)) { errno = static_cast<int>(result * -1); last_error_action = DLA_RESOLVE; }
		else return reinterpret_cast<void*>(result);
		return nullptr;
	}
	int dlclose(void* handle)
	{
		if(__builtin_expect(__so_close(handle), true)) return 0;
		if(last_error_action == DLA_FINI) return 1;
		return -1;
	}
	int dlinfo(void* handle, int request, void* info)
	{
		link_map* lm;
		switch(request)
		{
		case RTLD_DI_LMID:
			// namespaces are NYI so just return the default
			*static_cast<long*>(info) = 0L;
			break;
		case RTLD_DI_LINKMAP:
			if(__builtin_expect(!(lm = rtld_map.find(handle)), false))
			{
				errno				= EINVAL;
				last_error_action	= DLA_GETINFO;
				return -1;
			}
			*static_cast<link_map**>(info) = lm;
			break;
		case RTLD_DI_ORIGIN:
			if(__builtin_expect(!(lm = rtld_map.find(handle)), false))
			{
				errno				= EINVAL;
				last_error_action	= DLA_GETINFO;
				return -1;
			}
			__copy(info, lm->l_name, __strlen(lm->l_name));
			break;
		default:
			errno = ENOSYS; // NYI or unsupported
			return -1;
		}
		return 0;
	}
	int dladdr(const void* addr, dl_info* info)
	{
		int result;
		asm volatile("syscall" : "=a"(result) : "0"(SCV_DLADDR), "D"(addr), "S"(info) : "memory", "%r11", "%rcx");
		if(__builtin_expect(result < 0 && result > -4096, false)) { errno = static_cast<int>(result * -1); }
		return result;
	}
	char* dlerror()
	{
		__zero(dl_error_str, sizeof(dl_error_str));
		if(__builtin_expect(!errno || !last_error_action, false)) return nullptr;
		const char* err_str	= __get_dl_error();
		last_error_action	= DLA_NONE;
		errno				= 0;
		__copy(dl_error_str, err_str, __strlen(err_str));
		return dl_error_str;
	}
	void* __tls_get_addr(tls_index* ti)
	{
		long result;
		asm volatile("syscall" : "=a"(result) : "0"(SCV_TLGET), "D"(ti) : "memory", "%r11", "%rcx");
		if(__builtin_expect(result < 0 && result > -4096, false)) { errno = static_cast<int>(result * -1); return nullptr; }
		return reinterpret_cast<void*>(result);
	}
}