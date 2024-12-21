#include "kernel/heap_allocator.hpp"
#include "new"
extern "C"
{
    extern frame_tag* __kernel_frame_tag;
    void* malloc(size_t n) { return __kernel_frame_tag->allocate(n); }
    void free(void* ptr) { __kernel_frame_tag->deallocate(ptr); }
    void* calloc(size_t n, size_t s) { return __kernel_frame_tag->array_allocate(n, s); }
    void* realloc(void* ptr, size_t n) { return __kernel_frame_tag->reallocate(ptr, n); }
}
[[nodiscard]] void* operator new(std::size_t count) { return __kernel_frame_tag->allocate(count); }
[[nodiscard]] void* operator new[](std::size_t count) { return __kernel_frame_tag->allocate(count); }
[[nodiscard]] void* operator new(std::size_t count, std::align_val_t al) { return __kernel_frame_tag->allocate(count, static_cast<size_t>(al)); }
[[nodiscard]] void* operator new[](std::size_t count, std::align_val_t al) { return __kernel_frame_tag->allocate(count, static_cast<size_t>(al)); }
void operator delete(void* ptr) noexcept { __kernel_frame_tag->deallocate(ptr); }
void operator delete[](void* ptr) noexcept { __kernel_frame_tag->deallocate(ptr); }
void operator delete(void* ptr, size_t) noexcept { __kernel_frame_tag->deallocate(ptr); }
void operator delete[](void* ptr, size_t) noexcept { __kernel_frame_tag->deallocate(ptr); }
void operator delete(void* ptr, std::align_val_t al) noexcept { __kernel_frame_tag->deallocate(ptr, static_cast<size_t>(al)); }
void operator delete[](void* ptr, std::align_val_t al) noexcept { __kernel_frame_tag->deallocate(ptr, static_cast<size_t>(al)); }
void operator delete(void* ptr, size_t, std::align_val_t al) noexcept { __kernel_frame_tag->deallocate(ptr, static_cast<size_t>(al)); }
void operator delete[](void* ptr, size_t, std::align_val_t al) noexcept { __kernel_frame_tag->deallocate(ptr, static_cast<size_t>(al)); }