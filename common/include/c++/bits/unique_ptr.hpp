#ifndef __STL_UNIQUE_PTR
#define __STL_UNIQUE_PTR
#include "bits/move.h"
#include "bits/stl_allocator.h"
#include "bits/in_place_t.hpp"
#include "tuple"
namespace std
{
	template<typename T> struct default_delete { constexpr void operator()(T* ptr) const { ::operator delete(ptr, static_cast<std::align_val_t>(alignof(T))); } };
	namespace __detail
	{
		template<typename FT, typename T> concept __ptr_accept_ftor = requires(FT ft) { ft(declval<T*>()); };
		template<typename T> concept __unique_ptrable = __or_<is_unbounded_array<T>, __not_<is_array<T>>>::value;
		template<typename DT> concept __has_typedef_pointer = requires { typename remove_reference_t<DT>::pointer; };
		template<typename DT, typename T> concept __typedef_ptr_deleter = __ptr_accept_ftor<DT, T> && __has_typedef_pointer<DT>;
		template<__unique_ptrable T, __ptr_accept_ftor<T> DT>
		struct __unique_ptr_traits
		{
			typedef T element_type;
			typedef DT deleter_type;
			typedef T* pointer;
			typedef typename conditional<is_reference_v<DT>, DT, DT const&>::type copy_deleter_type;
		};
		template<__unique_ptrable T, __typedef_ptr_deleter<T> DT>
		struct __unique_ptr_traits<T, DT>
		{
			typedef T element_type;
			typedef DT deleter_type;
			typedef typename remove_reference_t<DT>::pointer pointer;
			typedef typename conditional<is_reference_v<DT>, DT, DT const&>::type copy_deleter_type;
		};
	}
	template<typename T, __detail::__ptr_accept_ftor<T> DT = default_delete<T>>
	class unique_ptr
	{
		using __traits = __detail::__unique_ptr_traits<T, DT>;
		typedef typename __traits::copy_deleter_type __del_t;
		tuple<T*, DT> __ptr_impl;
	public:
		typedef typename __traits::pointer pointer;
		typedef typename __traits::element_type element_type;
		typedef typename __traits::deleter_type deleter_type;
		constexpr unique_ptr() = default;
		constexpr unique_ptr(nullptr_t) : unique_ptr() {}
		constexpr explicit unique_ptr(pointer p) : __ptr_impl(p, deleter_type()) {}
		constexpr unique_ptr(pointer p, __del_t d) : __ptr_impl(p, d) {}
		constexpr unique_ptr(pointer p, remove_reference_t<DT>&& d) : __ptr_impl(p, forward<DT>(d)) {}
	private:
		constexpr T*& __ptr() noexcept { return get<0>(__ptr_impl); }
		constexpr T* __ptr() const noexcept { return get<0>(__ptr_impl); }
		constexpr DT& __deleter() noexcept { return get<1>(__ptr_impl); }
		constexpr DT const& __deleter() const noexcept { return get<1>(__ptr_impl); }
		constexpr void __delete_if_present() noexcept { if(__ptr()) __deleter()(__ptr()); }
	public:
		constexpr pointer get() const noexcept { return __ptr(); }
		constexpr deleter_type& get_deleter() noexcept { return __deleter(); }
		constexpr deleter_type const& get_deleter() const noexcept { return __deleter(); }
		constexpr operator bool() const noexcept { return __ptr() != nullptr; }
		constexpr void reset(pointer p = pointer()) noexcept { __delete_if_present(); __ptr() = p; }
		constexpr pointer release() noexcept { pointer result = __ptr(); reset(); return result; }
		unique_ptr(unique_ptr const&) = delete;
		constexpr unique_ptr(unique_ptr&& that) noexcept : __ptr_impl(that.release(), std::move(that.__deleter())) {}
		unique_ptr& operator=(unique_ptr const&) = delete;
		constexpr unique_ptr& operator=(unique_ptr&& that) noexcept { reset(that.release()); __deleter() = std::move(that.__deleter()); return *this; }
		constexpr ~unique_ptr() noexcept { __delete_if_present(); }
		constexpr void swap(unique_ptr& that) noexcept { __ptr_impl.swap(that.__ptr_impl); }
		constexpr typename add_lvalue_reference<element_type>::type operator*() const { return *__ptr(); }
		constexpr pointer operator->() const noexcept { return __ptr(); }
	};
	template<typename T, __detail::__ptr_accept_ftor<T> DT>
	class unique_ptr<T[], DT>
	{
		using __traits = __detail::__unique_ptr_traits<T[], DT>;
		typedef typename __traits::copy_deleter_type __del_t;
		tuple<T*, DT> __ptr_impl;
	public:
		typedef typename __traits::pointer pointer;
		typedef typename __traits::element_type element_type;
		typedef typename __traits::deleter_type deleter_type;
		constexpr unique_ptr() = default;
		constexpr unique_ptr(nullptr_t) : unique_ptr() {}
		constexpr explicit unique_ptr(pointer p) : __ptr_impl(p, deleter_type()) {}
		constexpr unique_ptr(pointer p, __del_t d) : __ptr_impl(p, d) {}
		constexpr unique_ptr(pointer p, remove_reference_t<DT>&& d) : __ptr_impl(p, forward<DT>(d)) {}
	private:
		constexpr T*& __ptr() noexcept { return get<0>(__ptr_impl); }
		constexpr T* __ptr() const noexcept { return get<0>(__ptr_impl); }
		constexpr DT& __deleter() noexcept { return get<1>(__ptr_impl); }
		constexpr DT const& __deleter() const noexcept { return get<1>(__ptr_impl); }
		constexpr void __delete_if_present() noexcept { if(__ptr()) __deleter()(__ptr()); }
	public:
		constexpr pointer get() const noexcept { return __ptr(); }
		constexpr deleter_type& get_deleter() noexcept { return __deleter(); }
		constexpr deleter_type const& get_deleter() const noexcept { return __deleter(); }
		constexpr operator bool() const noexcept { return __ptr() != nullptr; }
		constexpr void reset(pointer p = pointer()) noexcept { __delete_if_present(); __ptr() = p; }
		constexpr pointer release() noexcept { pointer result = __ptr(); reset(); return result; }
		unique_ptr(unique_ptr const&) = delete;
		constexpr unique_ptr(unique_ptr&& that) noexcept : __ptr_impl(that.release(), std::move(that.__deleter())) {}
		unique_ptr& operator=(unique_ptr const&) = delete;
		constexpr unique_ptr& operator=(unique_ptr&& that) noexcept { reset(that.release()); __deleter() = std::move(that.__deleter()); return *this; }
		constexpr ~unique_ptr() noexcept { __delete_if_present(); }
		constexpr void swap(unique_ptr& that) noexcept { __ptr_impl.swap(that.__ptr_impl); }
		constexpr element_type& operator[](size_t i) const { return __ptr()[i]; }
	};
}
#endif