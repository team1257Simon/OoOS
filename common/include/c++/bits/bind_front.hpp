#ifndef __STD_BIND_FRONT
#define __STD_BIND_FRONT
#include <bits/invoke.hpp>
#include <tuple>
namespace std
{
	template<typename FT, typename ... BArgs>
	class __bind_front_expr
	{
		using __bound_indices = std::index_sequence_for<BArgs...>;
		FT __my_functor;
		std::tuple<BArgs...> __my_bound_args;
		template<typename T, size_t ... Is, typename ... CArgs> constexpr static decltype(auto) __call(T&& g, index_sequence<Is...>, CArgs&&... args) { return std::__invoke(std::forward<T>(g).__my_functor, std::get<Is>(std::forward<T>(g).__my_bound_args)..., std::forward<CArgs>(args)...); }
	public:
		template<typename T, typename ... CArgs> constexpr static decltype(auto) __call(T&& g, CArgs&&... args) { return __call(std::forward<T>(g), __bound_indices(), std::forward<CArgs>(args)...); }
		template<typename GT, typename ... Args> requires(sizeof...(Args) == sizeof...(BArgs) && std::convertible_to<GT, FT>) constexpr explicit __bind_front_expr(int, GT&& gt, Args&&... args) noexcept(__and_v<is_nothrow_constructible<FT, GT>, is_nothrow_constructible<BArgs, Args>...>) : __my_functor(std::forward<GT>(gt)), __my_bound_args(std::forward<Args>(args)...) {}
		constexpr __bind_front_expr(__bind_front_expr const&) = default;
		constexpr __bind_front_expr(__bind_front_expr&&) = default;
		constexpr __bind_front_expr& operator=(__bind_front_expr const&) = default;
		constexpr __bind_front_expr& operator=(__bind_front_expr&&) = default;
		constexpr ~__bind_front_expr() = default;
		template<typename ... CArgs> constexpr std::invoke_result_t<FT&, BArgs&..., CArgs...> operator()(CArgs&&... args)& noexcept(std::is_nothrow_invocable_v<FT&, BArgs&..., CArgs...>) { return __call(*this, __bound_indices{}, std::forward<CArgs>(args)...); }
		template<typename ... CArgs> constexpr std::invoke_result_t<FT const&, BArgs const&..., CArgs...> operator()(CArgs&&... args) const& noexcept(std::is_nothrow_invocable_v<FT const&, BArgs const&..., CArgs...>) { return __call(*this, __bound_indices{}, std::forward<CArgs>(args)...); }
		template<typename ... CArgs> constexpr std::invoke_result_t<FT, BArgs..., CArgs...> operator()(CArgs&&... args)&& noexcept(std::is_nothrow_invocable_v<FT, BArgs..., CArgs...>) { return __call(std::move(*this), __bound_indices{}, std::forward<CArgs>(args)...); }
		template<typename ... CArgs> constexpr std::invoke_result_t<const FT, const BArgs..., CArgs...> operator()(CArgs&&... args) const&& noexcept(std::is_nothrow_invocable_v<const FT, const BArgs..., CArgs...>) { return __call(move(*this), __bound_indices{}, std::forward<CArgs>(args)...); }
	};
	template<typename FT, typename ... BArgs> constexpr __bind_front_expr<FT, BArgs...> bind_front(FT&& ft, BArgs&& ... args) { return __bind_front_expr<FT, BArgs...>(0, std::move(ft), std::forward<BArgs>(args)...); }
}
#endif