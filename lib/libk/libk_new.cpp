#include "kernel/kernel_mm.hpp"
#include "new"
#include "stdexcept"
#include "atomic"
extern "C"
{
	kframe_tag* __kernel_frame_tag{};
	void* malloc(size_t n) { return __kernel_frame_tag->allocate(n); }
	void free(void* ptr) { __kernel_frame_tag->deallocate(ptr); }
	void* calloc(size_t n, size_t s) { return __kernel_frame_tag->array_allocate(n, s); }
	void* realloc(void* ptr, size_t n) { return __kernel_frame_tag->reallocate(ptr, n); }
}
[[nodiscard]] void* operator new(std::size_t count) { if(void* ptr = __kernel_frame_tag->allocate(count)) return ptr; else if(std::new_handler h = std::get_new_handler()) h(); else throw std::bad_alloc{}; return __kernel_frame_tag->allocate(count); }
[[nodiscard]] void* operator new[](std::size_t count) { if(void* ptr = __kernel_frame_tag->allocate(count)) return ptr; else if(std::new_handler h = std::get_new_handler()) h(); else throw std::bad_alloc{}; return __kernel_frame_tag->allocate(count); }
[[nodiscard]] void* operator new(std::size_t count, std::align_val_t al) { if(void* ptr = __kernel_frame_tag->allocate(count, static_cast<size_t>(al))) return ptr; else if(std::new_handler h = std::get_new_handler()) h(); else throw std::bad_alloc{}; return __kernel_frame_tag->allocate(count, static_cast<size_t>(al)); }
[[nodiscard]] void* operator new[](std::size_t count, std::align_val_t al) { if(void* ptr = __kernel_frame_tag->allocate(count, static_cast<size_t>(al))) return ptr; else if(std::new_handler h = std::get_new_handler()) h(); else throw std::bad_alloc{}; return __kernel_frame_tag->allocate(count, static_cast<size_t>(al)); }
[[nodiscard]] void* operator new(std::size_t count, std::nothrow_t const&) noexcept { try { return operator new(count); } catch(std::bad_alloc& e) { return nullptr; } }
[[nodiscard]] void* operator new[](std::size_t count, std::nothrow_t const&) noexcept { try { return operator new[](count); } catch(std::bad_alloc& e) { return nullptr; } }
[[nodiscard]] void* operator new(std::size_t count, std::align_val_t al, std::nothrow_t const&) noexcept { try { return operator new(count); } catch(std::bad_alloc& e) { return nullptr; } }
[[nodiscard]] void* operator new[](std::size_t count, std::align_val_t al, std::nothrow_t const&) noexcept { try { return operator new[](count); } catch(std::bad_alloc& e) { return nullptr; } }
void operator delete(void* ptr) noexcept { __kernel_frame_tag->deallocate(ptr); }
void operator delete[](void* ptr) noexcept { __kernel_frame_tag->deallocate(ptr); }
void operator delete(void* ptr, std::size_t) noexcept { __kernel_frame_tag->deallocate(ptr); }
void operator delete[](void* ptr, std::size_t) noexcept { __kernel_frame_tag->deallocate(ptr); }
void operator delete(void* ptr, std::align_val_t al) noexcept { __kernel_frame_tag->deallocate(ptr, static_cast<size_t>(al)); }
void operator delete[](void* ptr, std::align_val_t al) noexcept { __kernel_frame_tag->deallocate(ptr, static_cast<size_t>(al)); }
void operator delete(void* ptr, std::size_t, std::align_val_t al) noexcept { __kernel_frame_tag->deallocate(ptr, static_cast<size_t>(al)); }
void operator delete[](void* ptr, std::size_t, std::align_val_t al) noexcept { __kernel_frame_tag->deallocate(ptr, static_cast<size_t>(al)); }
namespace std
{
	[[gnu::externally_visible]] void* __detail::__aligned_reallocate(void* ptr, size_t n, size_t align) { if(!n) return nullptr; if(void* result = __kernel_frame_tag->reallocate(ptr, n, align)) return result; else if(std::new_handler h = std::get_new_handler()) h(); else throw std::bad_alloc{}; return __kernel_frame_tag->reallocate(ptr, n, align); }
	atomic<new_handler> __l_handler;
	new_handler set_new_handler(new_handler handler) { return __l_handler.exchange(handler); }
	new_handler get_new_handler() { return __l_handler.load(); }
}