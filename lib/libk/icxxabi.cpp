#include "bits/icxxabi.h"
#include "kernel/libk_decls.h"

extern "C" void panic(const char* str);
namespace __cxxabiv1 
{
    __extension__ typedef spinlock_t __guard;
    extern "C" int __cxa_guard_acquire (__guard *g)  { while(acquire(g)); return 0; }
    extern "C" void __cxa_guard_release (__guard *g) { release(g); }
    extern "C" void __cxa_guard_abort (__guard *){}
}
extern "C" 
{	
	atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
	atexit_func_entry_t __tmp_atexit_buff[ATEXIT_MAX_FUNCS];
	uarch_t __atexit_func_count = 0;
	extern "C" void *__dso_handle;
	extern "C" __cxxabiv1::__guard __atexit_guard;
	extern "C" void _fini();
	int __cxa_atexit(void (*f)(void *), void *objptr, void *dso)
	{
		__cxxabiv1::__cxa_guard_acquire(&__atexit_guard);
		if (__atexit_func_count >= ATEXIT_MAX_FUNCS) {return -1;};
		__atexit_funcs[__atexit_func_count].destructor_func = f;
		__atexit_funcs[__atexit_func_count].obj_ptr = objptr;
		__atexit_funcs[__atexit_func_count].dso_handle = dso;
		__atexit_func_count++;
		__cxxabiv1::__cxa_guard_release(&__atexit_guard);
		return 0;
	};
	void __cxa_finalize(void *f)
	{
		if(!f) for(size_t i = __atexit_func_count - 1; i >= 0; i--) { if(__atexit_funcs[i].destructor_func) __atexit_funcs[i].destructor_func(__atexit_funcs[i].obj_ptr); }
		else for(size_t i = 0; i < __atexit_func_count; i++) if(__atexit_funcs[i].destructor_func == f)
		{
			__atexit_funcs[i].destructor_func(__atexit_funcs[i].obj_ptr);
			__builtin_memcpy(__tmp_atexit_buff, &__atexit_funcs[i + 1], ((ATEXIT_MAX_FUNCS - i - 1) * sizeof(atexit_func_entry_t)));
			__builtin_memcpy(&__atexit_funcs[i], __tmp_atexit_buff, ((ATEXIT_MAX_FUNCS - i - 1) * sizeof(atexit_func_entry_t)));
			__builtin_memset(__tmp_atexit_buff, 0, ATEXIT_MAX_FUNCS * sizeof(atexit_func_entry_t));
			__atexit_func_count--;
		}
		if(!f)_fini();
	};
	void __cxa_pure_virtual()
	{
		panic("Call to pure virtual");
		__builtin_abort();
		__builtin_unreachable();
	}
	extern char* __assert_fail_text(const char* text, const char* fname, const char* filename, int line);
	void __on_fail_assert(const char* text, const char* fname, const char* filename, int line)
	{
		char* etxt = __assert_fail_text(text, fname, filename, line);
		panic(etxt);
		__builtin_abort();
		__builtin_unreachable();
	}
}