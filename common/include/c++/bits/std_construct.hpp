#ifndef __STD_CONSTRUCT
#define __STD_CONSTRUCT
#include <bits/stl_allocator.h>
#include <tuple>
namespace std
{
	template<typename T, typename ... Args> requires(__detail::__dynamic_constructible<T, Args&&...>)
	constexpr T* construct_at(T* ptr, Args&& ... args)
	noexcept(is_nothrow_constructible_v<T, Args&&...>) { return ::new(static_cast<void*>(ptr)) T(std::forward<Args>(args)...); }
	namespace __detail
	{
		template<typename T>
		struct __nothrow_destruct : is_nothrow_destructible<T> {};
		template<typename T> requires(is_array_v<T>)
		struct __nothrow_destruct<T> : __nothrow_destruct<remove_all_extents_t<T>> {};
	}
	template<typename T>
	constexpr void destroy_at(T* p) noexcept(__detail::__nothrow_destruct<T>::value)
	{
		if constexpr(std::is_array_v<T>)
			for(auto& e : *p)
				(destroy_at)(std::addressof(e));
		else p->~T();
	}
	extension namespace ext
	{
		template<typename T, typename ... Args, size_t ... Is>
		constexpr T* __tuple_construct(T* ptr, tuple<Args...>&& args, index_sequence<Is...>) { return construct_at(ptr, get<Is>(forward<tuple<Args...>>(args))...);  }
		template<typename T, typename ... Args> requires(constructible_from<T, Args...>)
		constexpr T* tuple_construct(T* ptr, tuple<Args...>&& args) { return __tuple_construct(ptr, forward<tuple<Args...>>(args), make_index_sequence<sizeof...(Args)>{}); }
	}
}
#endif