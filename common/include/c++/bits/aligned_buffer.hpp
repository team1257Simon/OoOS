#ifndef __ALIGNED_BUFFER
#define __ALIGNED_BUFFER
#include <bits/move.h>
namespace __impl
{
	template<typename T>
	struct __aligned_buffer : std::aligned_storage<sizeof(T), alignof(T)>
	{
		typename std::aligned_storage<sizeof(T), alignof(T)>::type __my_storage;
		__aligned_buffer() = default;
		__aligned_buffer(std::nullptr_t) {}
		constexpr void* __get_addr() noexcept { return static_cast<void*>(std::addressof(__my_storage)); }
		constexpr const void* __get_addr() const noexcept { return static_cast<const void*>(std::addressof(__my_storage)); }
		constexpr T* __get_ptr() noexcept { return static_cast<T*>(__get_addr()); }
		constexpr T const* __get_ptr() const noexcept { return static_cast<T const*>(__get_addr()); }
	};
}
#endif