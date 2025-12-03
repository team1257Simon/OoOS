#ifndef __PTR_GUARD
#define __PTR_GUARD
#include <memory>
namespace std
{
	namespace ext
	{
		namespace __impl
		{
			template<typename T, std::allocator_object<T> A>
			struct __ptr_guard
			{
				T* ptr;
				size_t size;
				A alloc;
				constexpr __ptr_guard(T* p, size_t n, A const& a = A()) noexcept(std::is_nothrow_copy_constructible_v<A>) : ptr(p), size(n), alloc(std::move(a)) {}
				constexpr __ptr_guard(T* p, A const& a = A()) noexcept(std::is_nothrow_copy_constructible_v<A>) : ptr(p), size(1UZ), alloc(std::move(a)) {}
				constexpr void release() noexcept { ptr = nullptr; }
				constexpr ~__ptr_guard() { if(ptr) alloc.deallocate(ptr, size); }
			};
		}
		template<typename T, std::allocator_object<T> A = std::allocator<T>> using ptr_guard = __impl::__ptr_guard<T, A>;
	}
}
#endif