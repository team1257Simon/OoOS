
#include "kernel/kernel_mm.hpp"
#include "new"
#include "stdexcept"
#include "atomic"
extern "C"
{
	extern kframe_tag* __module_frame_tag;
	extern kframe_exports __frame_functions;
	void* malloc(size_t count) { return ((__module_frame_tag)->*(__frame_functions.allocate))(count, 0UL); }
	void* memalign(size_t align, size_t count) { return ((__module_frame_tag)->*(__frame_functions.allocate))(count, align); }
	void free(void* ptr) { ((__module_frame_tag)->*(__frame_functions.deallocate))(ptr, 0UL); }
	void* calloc(size_t count, size_t s) { return ((__module_frame_tag)->*(__frame_functions.array_allocate))(count, s); }
	void* realloc(void* ptr, size_t count) { return ((__module_frame_tag)->*(__frame_functions.reallocate))(ptr, count, 0UL); }
}
[[nodiscard]] void* operator new(std::size_t count) { if(void* ptr = ((__module_frame_tag)->*(__frame_functions.allocate))(count, 0UL)) return ptr; else if(std::new_handler h = std::get_new_handler()) h(); else throw std::bad_alloc{}; return ((__module_frame_tag)->*(__frame_functions.allocate))(count, 0UL); }
[[nodiscard]] void* operator new[](std::size_t count) { if(void* ptr = ((__module_frame_tag)->*(__frame_functions.allocate))(count, 0UL)) return ptr; else if(std::new_handler h = std::get_new_handler()) h(); else throw std::bad_alloc{}; return ((__module_frame_tag)->*(__frame_functions.allocate))(count, 0UL); }
[[nodiscard]] void* operator new(std::size_t count, std::align_val_t al) { if(void* ptr = ((__module_frame_tag)->*(__frame_functions.allocate))(count, static_cast<size_t>(al))) return ptr; else if(std::new_handler h = std::get_new_handler()) h(); else throw std::bad_alloc{}; return ((__module_frame_tag)->*(__frame_functions.allocate))(count, static_cast<size_t>(al)); }
[[nodiscard]] void* operator new[](std::size_t count, std::align_val_t al) { if(void* ptr = ((__module_frame_tag)->*(__frame_functions.allocate))(count, static_cast<size_t>(al))) return ptr; else if(std::new_handler h = std::get_new_handler()) h(); else throw std::bad_alloc{}; return ((__module_frame_tag)->*(__frame_functions.allocate))(count, static_cast<size_t>(al)); }
[[nodiscard]] void* operator new(std::size_t count, std::nothrow_t const&) noexcept { try { return operator new(count); } catch(std::bad_alloc& e) { return nullptr; } }
[[nodiscard]] void* operator new[](std::size_t count, std::nothrow_t const&) noexcept { try { return operator new[](count); } catch(std::bad_alloc& e) { return nullptr; } }
[[nodiscard]] void* operator new(std::size_t count, std::align_val_t al, std::nothrow_t const&) noexcept { try { return operator new(count); } catch(std::bad_alloc& e) { return nullptr; } }
[[nodiscard]] void* operator new[](std::size_t count, std::align_val_t al, std::nothrow_t const&) noexcept { try { return operator new[](count); } catch(std::bad_alloc& e) { return nullptr; } }
void operator delete(void* ptr) noexcept { ((__module_frame_tag)->*(__frame_functions.deallocate))(ptr, 0UL); }
void operator delete[](void* ptr) noexcept { ((__module_frame_tag)->*(__frame_functions.deallocate))(ptr, 0UL); }
void operator delete(void* ptr, std::size_t) noexcept { ((__module_frame_tag)->*(__frame_functions.deallocate))(ptr, 0UL); }
void operator delete[](void* ptr, std::size_t) noexcept { ((__module_frame_tag)->*(__frame_functions.deallocate))(ptr, 0UL); }
void operator delete(void* ptr, std::align_val_t al) noexcept { ((__module_frame_tag)->*(__frame_functions.deallocate))(ptr, static_cast<size_t>(al)); }
void operator delete[](void* ptr, std::align_val_t al) noexcept { ((__module_frame_tag)->*(__frame_functions.deallocate))(ptr, static_cast<size_t>(al)); }
void operator delete(void* ptr, std::size_t, std::align_val_t al) noexcept { ((__module_frame_tag)->*(__frame_functions.deallocate))(ptr, static_cast<size_t>(al)); }
void operator delete[](void* ptr, std::size_t, std::align_val_t al) noexcept { ((__module_frame_tag)->*(__frame_functions.deallocate))(ptr, static_cast<size_t>(al)); }
namespace std
{
	exception::exception() noexcept {}
	exception::~exception() noexcept {}
	exception::exception(exception const&) noexcept {}
	exception& exception::operator=(exception const&) noexcept { return *this; }
	const char* exception::what() const noexcept { return "std::exception"; }
	bad_alloc::bad_alloc() noexcept {}
	bad_alloc::~bad_alloc() noexcept {}
	bad_alloc::bad_alloc(bad_alloc const&) noexcept {}
	bad_alloc& bad_alloc::operator=(bad_alloc const&) noexcept { return *this; }
	const char* bad_alloc::what() const noexcept { return "std::bad_alloc"; }
	atomic<new_handler> __l_handler;
	new_handler set_new_handler(new_handler handler) { return __l_handler.exchange(handler); }
	new_handler get_new_handler() { return __l_handler.load(); }
	void* __detail::__aligned_reallocate(void* ptr, size_t count, size_t align)
	{ 
		if(!count) return nullptr; 
		if(void* result = ((__module_frame_tag)->*(__frame_functions.reallocate))(ptr, count, align)) return result;
		else if(std::new_handler h = std::get_new_handler()) h();
		else throw std::bad_alloc{};
		return ((__module_frame_tag)->*(__frame_functions.reallocate))(ptr, count, align);
	}
}