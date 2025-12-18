#ifndef __STD_CONSTRUCT
#define __STD_CONSTRUCT
#include <bits/stl_allocator.h>
#include <tuple>
namespace std
{
	template<typename T, typename ... Args> requires(__detail::__dynamic_constructible<T, Args...>) constexpr T* construct_at(T* ptr, Args ... args) noexcept(is_nothrow_constructible_v<T, Args...>) { return ::new(static_cast<void*>(ptr)) T(std::forward<Args>(args)...); }
	extension namespace ext
	{
		template<typename T, typename ... Args, size_t ... Is> constexpr T* __tuple_construct(T* ptr, tuple<Args...>&& args, index_sequence<Is...>) { return construct_at(ptr, get<Is>(forward<tuple<Args...>>(args))...);  }
		template<typename T, typename ... Args> requires constructible_from<T, Args...> constexpr T* tuple_construct(T* ptr, tuple<Args...>&& args) { return __tuple_construct(ptr, forward<tuple<Args...>>(args), make_index_sequence<sizeof...(Args)>{}); }
	}
}
#endif