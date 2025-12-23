#ifndef __RANGE_UTIL
#define __RANGE_UTIL
#include <bits/range_access.hpp>
#include <bits/invoke.hpp>
#include <bits/in_place_t.hpp>
#include <tuple>
namespace std
{
	namespace ranges
	{
		namespace __detail
		{
			template<typename RT> concept __simple_view = view<RT> && same_as<iterator_t<RT>, iterator_t<RT const>> && same_as<sentinel_t<RT>, sentinel_t<RT const>>;
			template<typename IT> concept __has_arrow	= input_iterator<IT> && (is_pointer_v<IT> || requires(IT const i) { i.operator->(); });
		}
		template<__detail::__valid_view_interface_arg DT>
		class view_interface
		{
			constexpr DT& __downcast() noexcept
			{
				static_assert(derived_from<DT, view_interface<DT>);
				static_assert(view<DT>);
				return static_cast<DT&>(*this);
			}
			constexpr DT const& __downcast() const noexcept
			{
				static_assert(derived_from<DT, view_interface<DT>);
				static_assert(view<DT>);
				return static_cast<DT const&>(*this);
			}
			constexpr static bool __bool(bool) noexcept;	// not defined; noexcept specifier wrapper
			template<typename T> constexpr static bool __empty(T& t) noexcept(noexcept(__bool(ranges::begin(t) == ranges::end(t)))) { return ranges::begin(t) == ranges::end(t); }
			template<typename T> constexpr static auto __size(T& t) noexcept(noexcept(ranges::end(t) - ranges::begin(t))) { return ranges::end(t) - ranges::begin(t); }
		public:
			constexpr bool empty() noexcept(noexcept(__empty(__downcast()))) requires(forward_range<DT> && (!sized_range<DT>)) { return __empty(__downcast()); }
			constexpr bool empty() noexcept(noexcept(ranges::size(__downcast()) == 0UZ)) requires(sized_range<DT>) { return ranges::size(__downcast()) == 0UZ; }
			constexpr bool empty() const noexcept(noexcept(__empty(__downcast()))) requires(forward_range<DT const> && (!sized_range<DT const>)) { return __empty(__downcast()); }
			constexpr bool empty() const noexcept(noexcept(ranges::size(__downcast()) == 0UZ)) requires(sized_range<DT const>) { return ranges::size(__downcast()) == 0UZ; }
			constexpr explicit operator bool() noexcept(noexcept(ranges::empty(__downcast()))) requires(requires { ranges::empty(__downcast()); }) { return !ranges::empty(__downcast()); }
			constexpr explicit operator bool() const noexcept(noexcept(ranges::empty(__downcast()))) requires(requires { ranges::empty(__downcast()); }) { return !ranges::empty(__downcast()); }
			constexpr auto data() noexcept(noexcept(ranges::begin(__downcast()))) requires(contiguous_iterator<iterator_t<DT>>) { return std::to_address(ranges::begin(__downcast())); }
			constexpr auto data() const noexcept(noexcept(ranges::begin(__downcast()))) requires(range<DT const> && contiguous_iterator<iterator_t<DT const>>) { return std::to_address(ranges::begin(__downcast())); }
			constexpr auto size() noexcept(noexcept(__size(__downcast()))) requires(forward_range<DT> && sized_sentinel_for<sentinel_t<DT>, iterator_t<DT>>) { return __size(__downcast()); }
			constexpr auto size() const noexcept(noexcept(__size(__downcast()))) requires(forward_range<DT const> && sized_sentinel_for<sentinel_t<DT const>, iterator_t<DT const>>) { return __size(__downcast()); }
			constexpr decltype(auto) front() requires(forward_range<DT>) { __libk_assert(!empty()); return *ranges::begin(__downcast()); }
			constexpr decltype(auto) front() const requires(forward_range<DT const>) { __libk_assert(!empty()); return *ranges::begin(__downcast()); }
			constexpr decltype(auto) back() requires(bidirectional_range<DT> && common_range<DT>) { __libk_assert(!empty()); return *ranges::prev(ranges::end(__downcast())); }
			constexpr decltype(auto) back() const requires(bidirectional_range<DT const> && common_range<DT const>) { __libk_assert(!empty()); return *ranges::prev(ranges::end(__downcast())); }
			template<random_access_range RT = DT> constexpr decltype(auto) operator[](range_difference_t<RT> n) { return ranges::begin(__downcast())[n]; }
			template<random_access_range RT = const DT> constexpr decltype(auto) operator[](range_difference_t<RT> n) const { return ranges::begin(__downcast())[n]; }
			constexpr auto cbegin() requires(input_range<DT>) { return ranges::cbegin(__downcast()); }
			constexpr auto cbegin() const requires(input_range<DT const>) { return ranges::cbegin(__downcast()); }
			constexpr auto cend() requires(input_range<DT>) { return ranges::cend(__downcast()); }
			constexpr auto cend() const requires(input_range<DT const>) { return ranges::cend(__downcast()); }
		};
	}
	namespace __detail
	{
		template<typename RT, typename T> concept __container_compatible_range	= ranges::input_range<RT> && convertible_to<ranges::range_reference_t<RT>, T>;
		template<ranges::input_range RT> using __range_key_type					= remove_cvref_t<tuple_element_t<0, ranges::range_value_t<RT>>>;
		template<ranges::input_range RT> using __range_mapped_type				= remove_cvref_t<tuple_element_t<1, ranges::range_value_t<RT>>>;
		template<ranges::input_range RT> using __range_to_alloc_type			= pair<const __range_key_type<RT>, __range_mapped_type<RT>>;
	}
}
#endif