#ifndef __EXPECTED_BASE
#define __EXPECTED_BASE
#include <bits/in_place_t.hpp>
#include <bits/std_construct.hpp>
#include <bits/exception.h>
namespace std
{
	template<typename VT, typename ET> class expected;
	template<typename ET> class unexpected;
	template<typename ET> class bad_expected_access;
	template<>
	class bad_expected_access<void> : public std::exception
	{
	protected:
		constexpr bad_expected_access() noexcept {}
		constexpr bad_expected_access(bad_expected_access const&) noexcept				= default;
		constexpr bad_expected_access(bad_expected_access&&) noexcept					= default;
		constexpr bad_expected_access& operator=(bad_expected_access const&) noexcept	= default;
		constexpr bad_expected_access& operator=(bad_expected_access&&) noexcept		= default;
		constexpr virtual ~bad_expected_access()										= default;
	public:
		constexpr virtual const char* what() const noexcept override { return "bad access to std::expected without expected value"; }
	};
	template<typename ET>
	class bad_expected_access : public bad_expected_access<void>
	{
		ET __err;
	public:
		constexpr explicit bad_expected_access(ET e) : __err(std::move(e)) {}
		template<typename ST> constexpr std::__like_t<ST, ET> error(this ST&& self) noexcept { return std::forward_like<ST>(self.__err); }
	};
	constexpr inline struct unexpect_t { constexpr explicit unexpect_t() noexcept = default; } unexpect{};
	namespace __detail
	{
		template<typename> struct __is_expected_type : std::false_type {};
		template<typename VT, typename ET> struct __is_expected_type<expected<VT, ET>> : std::true_type {};
		template<typename> struct __is_unexpected_type : std::false_type {};
		template<typename ET> struct __is_unexpected_type<unexpected<ET>> : std::true_type {};
		template<typename T> concept __expected_type	= __is_expected_type<T>::value;
		template<typename T> concept __unexpected_type	= __is_unexpected_type<T>::value;
		template<typename FT, typename T> struct __result : std::remove_cvref<std::invoke_result_t<FT&&, T&&>> {};
		template<typename FT, typename T> struct __transform_result : std::remove_cv<std::invoke_result_t<FT&&, T&&>> {};
		template<typename FT> struct __result_noarg : remove_cvref<std::invoke_result_t<FT&&>> {};
		template<typename FT> struct __transform_result_noarg : std::remove_cv<std::invoke_result_t<FT&&>> {};
		template<typename T> concept __is_tag			= same_as<std::remove_cv_t<T>, std::in_place_t>;
		template<typename T> concept __can_expect		= !is_reference_v<T> && !is_function_v<T> && !__is_tag<T> && !std::same_as<std::remove_cv_t<T>, unexpect_t>;
		template<typename T> concept __can_unexpect		= std::is_object_v<T> && !std::is_array_v<T> && !__unexpected_type<T> && !std::is_const_v<T> && !std::is_volatile_v<T>;
		struct __in_place_inv{};
		struct __unexpect_inv{};
		template<typename T> requires(is_nothrow_move_constructible_v<T>)
		class __reverse_guard
		{
			T* __guarded;
			T __tmp;
		public:
			constexpr explicit __reverse_guard(T& t) : __guarded(std::addressof(t)), __tmp(std::move(t)) { std::destroy_at(__guarded); }
			constexpr ~__reverse_guard() { if(__guarded) [[unlikely]] { std::construct_at(__guarded, std::move(__tmp)); } }
			constexpr T&& release() noexcept { __guarded = nullptr; return std::move(__tmp); }
		};
		template<typename T, typename U, typename V>
		constexpr void __reinit(T* __new, U* old, V&& arg) noexcept(is_nothrow_constructible_v<T, V>)
		{
			if constexpr(is_nothrow_constructible_v<T, V>) {
				std::destroy_at(old);
				std::construct_at(__new, std::forward<V>(arg));
			}
			else if constexpr(is_nothrow_move_constructible_v<T>)
			{
				T tmp(std::forward<V>(arg));
				std::destroy_at(old);
				std::construct_at(__new, std::move(tmp));
			}
			else
			{
				__reverse_guard<U> guard(*old);
				std::construct_at(__new, std::forward<V>(arg));
				guard.release();
			}
		}
		template<typename T, typename U> concept __not_constructing_bool_from_expected	= !std::is_same_v<remove_cv_t<T>, bool> || !__expected_type<std::remove_cvref_t<U>>;
		template<typename T, typename U> concept __cross_comparable						= requires(T const& t, U const& u) { { t == u } -> std::convertible_to<bool>; };
		template<typename T, typename U> concept __nt_cross_comparable					= noexcept(std::declval<T>() == std::declval<U>());
	}
}
#endif