#ifndef __STD_BIND_FRONT
#define __STD_BIND_FRONT
#include <bits/invoke.hpp>
#include <tuple>
namespace std
{
	template<std::move_constructible FT, std::move_constructible ... BArgs>
	class __bind_front_expr
	{
		using __bound_indices = std::index_sequence_for<BArgs...>;
		FT __my_functor;
		std::tuple<BArgs...> __my_bound_args;
		template<typename T, size_t ... Is, typename ... CArgs> constexpr static decltype(auto) __call(T&& __g, index_sequence<Is...>, CArgs&&... __args) { return std::__invoke(std::forward<T>(__g).__my_functor, std::get<Is>(std::forward<T>(__g).__my_bound_args)..., std::forward<CArgs>(__args)...); }
	public:
		template<std::move_constructible GT, std::move_constructible ... Args> requires(sizeof...(Args) == sizeof...(BArgs) && std::convertible_to<GT, FT>) constexpr explicit __bind_front_expr(int, GT&& __fn, Args&&... __args) noexcept(__and_v<is_nothrow_constructible<FT, GT>, is_nothrow_constructible<BArgs, Args>...>) : __my_functor(std::forward<GT>(__fn)), __my_bound_args(std::forward<Args>(__args)...) {}
		constexpr __bind_front_expr(__bind_front_expr const&) = default;
		constexpr __bind_front_expr(__bind_front_expr&&) = default;
		constexpr __bind_front_expr& operator=(__bind_front_expr const&) = default;
		constexpr __bind_front_expr& operator=(__bind_front_expr&&) = default;
		constexpr ~__bind_front_expr() = default;
		template<typename ... CArgs> constexpr std::invoke_result_t<FT&, BArgs&..., CArgs...> operator()(CArgs&&... __args)& noexcept(std::is_nothrow_invocable_v<FT&, BArgs&..., CArgs...>) { return __call(*this, __bound_indices{}, std::forward<CArgs>(__args)...); }
		template<typename ... CArgs> constexpr std::invoke_result_t<FT const&, BArgs const&..., CArgs...> operator()(CArgs&&... __args) const& noexcept(std::is_nothrow_invocable_v<FT const&, BArgs const&..., CArgs...>) { return __call(*this, __bound_indices{}, std::forward<CArgs>(__args)...); }
		template<typename ... CArgs> constexpr std::invoke_result_t<FT, BArgs..., CArgs...> operator()(CArgs&&... __args)&& noexcept(std::is_nothrow_invocable_v<FT, BArgs..., CArgs...>) { return __call(std::move(*this), __bound_indices{}, std::forward<CArgs>(__args)...); }
		template<typename ... CArgs> constexpr std::invoke_result_t<const FT, const BArgs..., CArgs...> operator()(CArgs&&... __args) const&& noexcept(std::is_nothrow_invocable_v<const FT, const BArgs..., CArgs...>) { return __call(move(*this), __bound_indices{}, std::forward<CArgs>(__args)...); }
	};
	template<std::move_constructible FT, std::move_constructible ... BArgs> constexpr __bind_front_expr<FT, BArgs...> bind_front(FT&& ft, BArgs&& ... args) { return __bind_front_expr<FT, BArgs...>(0, std::move(ft), std::forward<BArgs>(args)...); }
}
#endif