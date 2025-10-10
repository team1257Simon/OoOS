
#include "kernel/kernel_mm.hpp"
#include "new"
#include "stdexcept"
#include "atomic"
extern "C"
{
    attribute(section(".data.modframe")) kframe_tag* __module_frame_tag{};
    attribute(section(".data.modframe_hooks")) kframe_exports __frame_functions{};
    void* malloc(size_t count) { return ((__module_frame_tag)->*(__frame_functions.allocate))(count, 0UL); }
    void* memalign(size_t align, size_t count) { return ((__module_frame_tag)->*(__frame_functions.allocate))(count, align); }
    void free(void* ptr) { ((__module_frame_tag)->*(__frame_functions.deallocate))(ptr, 0UL); }
    void* calloc(size_t count, size_t s) { return ((__module_frame_tag)->*(__frame_functions.array_allocate))(count, s); }
    void* realloc(void* ptr, size_t count) { return ((__module_frame_tag)->*(__frame_functions.reallocate))(ptr, count, 0UL); }
}
#pragma GCC visibility push(hidden)
namespace std
{
    attribute(weak) exception::exception() noexcept {}
    attribute(weak) exception::~exception() noexcept {}
    attribute(weak) exception::exception(exception const&) noexcept {}
    attribute(weak) exception& exception::operator=(exception const&) noexcept { return *this; }
    attribute(weak) const char*           exception::what() const noexcept { return "std::exception"; }
    attribute(weak) bad_alloc::bad_alloc() noexcept {}
    attribute(weak) bad_alloc::~bad_alloc() noexcept {}
    attribute(weak) bad_alloc::bad_alloc(bad_alloc const&) noexcept {}
    attribute(weak) bad_alloc& bad_alloc::operator=(bad_alloc const&) noexcept { return *this; }
    attribute(weak) const char*           bad_alloc::what() const noexcept { return "std::bad_alloc"; }
    void* __detail::__aligned_reallocate(void* ptr, size_t count, size_t align) { if(!count) return nullptr; if(void* result = ((__module_frame_tag)->*(__frame_functions.reallocate))(ptr, count, align)) return result; else if(std::new_handler h = std::get_new_handler()) h(); else throw std::bad_alloc{}; return ((__module_frame_tag)->*(__frame_functions.reallocate))(ptr, count, align); }
}
#pragma GCC visibility pop