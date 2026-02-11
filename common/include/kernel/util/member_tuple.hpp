#ifndef __MEMBER_TUPLE
#define __MEMBER_TUPLE
#include <meta>
#ifdef __have_reflection
#include <ranges>
namespace ooos
{
	namespace __internal
	{
		template<typename T>
		class __member_tuple_helper
		{
			consteval static bool __is_pointable(std::meta::info i) { return !std::meta::is_special_member_function(i); }
			template<bool SB, typename M> using __maybe_member_t = std::conditional_t<SB, M*, M T::*>;
			consteval static size_t __count_members()
			{
				return (std::meta::members_of(^^T, std::meta::access_context::current())
						| std::views::filter(__is_pointable)
						| std::ranges::to<std::vector>())
						.size();
			}
			constexpr static size_t __count = __count_members();
			template<size_t I>
			struct __member_at_index
			{
				consteval static std::meta::info __get()
				{
					return (std::meta::members_of(^^T, std::meta::access_context::current())
						| std::views::filter(__is_pointable)
						| std::ranges::to<std::vector>())[I];
				}
				using __base_type = typename [: std::meta::type_of(__get()):];
				typedef __maybe_member_t<std::meta::is_static_member(__get()), __base_type> __type;
				constexpr static __type __value = std::meta::extract<__type>(__get());
			};
			template<size_t ... Is>
			consteval static auto __get(std::index_sequence<Is...>) { return std::make_tuple(__member_at_index<__count - Is - 1>::__value...); }
			typedef std::make_index_sequence<__count_members()> __indices;
		public:
			consteval static auto get() { return __get(__indices()); }
			typedef decltype(get()) __type;
		};
	}
	template<typename T> using member_tuple_t = typename  __internal::__member_tuple_helper<T>::__type;
	template<typename T> constexpr member_tuple_t<T> member_tuple() { return __internal::__member_tuple_helper<T>::get(); }
}
#endif
#endif