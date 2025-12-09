#include <bits/icxxabi.h>
#include <module.hpp>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
namespace __cxxabiv1
{
	extension typedef spinlock_t __guard;
	extern "C" int __cxa_guard_acquire(__guard* g)  { lock(g); return 0; }
	extern "C" void __cxa_guard_release(__guard* g) { release(g); }
	extern "C" void __cxa_guard_abort(__guard*) {}
}
extern "C"
{
	atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
	atexit_func_entry_t __tmp_atexit_buff[ATEXIT_MAX_FUNCS];
	uarch_t __atexit_func_count			= 0;
	static const char* msg_text			= nullptr;
	extern "C" void* __dso_handle;
	extern "C" __cxxabiv1::__guard __atexit_guard;
	extern "C" void _fini();
	[[noreturn]] void __on_fail_assert(const char* text, const char* fname, const char* filename, int line)
	{
		ooos::abstract_module_base* inst	= ooos::module_instance();
		if(msg_text) free(const_cast<char*>(msg_text));
		inst->asprintf(std::addressof(msg_text), "E: failed assertion in function %s (%s line %i): %s", fname, filename, line, text);
		inst->raise_error(msg_text);
		__builtin_unreachable();
	}
	int __cxa_atexit(void (*f)(void*), void* objptr, void* dso)
	{
		__cxxabiv1::__cxa_guard_acquire(std::addressof(__atexit_guard));
		if(__atexit_func_count >= ATEXIT_MAX_FUNCS) return -1;
		__atexit_funcs[__atexit_func_count].destructor_func = f;
		__atexit_funcs[__atexit_func_count].obj_ptr 		= objptr;
		__atexit_funcs[__atexit_func_count].dso_handle 		= dso;
		__atexit_func_count++;
		__cxxabiv1::__cxa_guard_release(std::addressof(__atexit_guard));
		return 0;
	};
	void __cxa_finalize(void* f)
	{
		if(!f) for(size_t i = __atexit_func_count - 1Z; i >= 0UZ; i--) { if(__atexit_funcs[i].destructor_func) __atexit_funcs[i].destructor_func(__atexit_funcs[i].obj_ptr); }
		else for(size_t i = 0UZ; i < __atexit_func_count; i++) if(__atexit_funcs[i].destructor_func == f)
		{
			__atexit_funcs[i].destructor_func(__atexit_funcs[i].obj_ptr);
			memcpy(__tmp_atexit_buff, std::addressof(__atexit_funcs[i + 1Z]), ((ATEXIT_MAX_FUNCS - i - 1Z) * sizeof(atexit_func_entry_t)));
			memcpy(std::addressof(__atexit_funcs[i]), __tmp_atexit_buff, ((ATEXIT_MAX_FUNCS - i - 1Z) * sizeof(atexit_func_entry_t)));
			memset(__tmp_atexit_buff, 0, ATEXIT_MAX_FUNCS * sizeof(atexit_func_entry_t));
			__atexit_func_count--;
		}
		if(!f) _fini();
	};

}