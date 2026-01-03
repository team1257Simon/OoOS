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
				static_assert(derived_from<DT, view_interface<DT>>);
				static_assert(view<DT>);
				return static_cast<DT&>(*this);
			}
			constexpr DT const& __downcast() const noexcept
			{
				static_assert(derived_from<DT, view_interface<DT>>);
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
		namespace __detail
		{
			template<typename RT, typename T> concept __container_compatible_range	= ranges::input_range<RT> && convertible_to<ranges::range_reference_t<RT>, T>;
			template<ranges::input_range RT> using __range_key_type					= remove_cvref_t<tuple_element_t<0, ranges::range_value_t<RT>>>;
			template<ranges::input_range RT> using __range_mapped_type				= remove_cvref_t<tuple_element_t<1, ranges::range_value_t<RT>>>;
			template<ranges::input_range RT> using __range_to_alloc_type			= pair<const __range_key_type<RT>, __range_mapped_type<RT>>;
			template<typename FT, typename TT> 
			concept __uses_nonqualification_pointer_conversion	=	is_pointer_v<FT>
																	&& is_pointer_v<TT>
																	&& !convertible_to<remove_pointer_t<FT>(*)[], remove_pointer_t<TT>(*)[]>;
			template<typename FT, typename TT>
			concept __convertible_to_non_slicing				=	convertible_to<FT, TT>
																	&& !__uses_nonqualification_pointer_conversion<decay_t<FT>, decay_t<TT>>;
			template<typename T>
			concept __pair_like									=	!is_reference_v<T>
																	&& requires(T __t)
																	{
																		typename tuple_size<T>::type;
																		requires(derived_from<tuple_size<T>, integral_constant<size_t, 2>>);
																		typename tuple_element_t<0, remove_const_t<T>>;
																		typename tuple_element_t<1, remove_const_t<T>>;
																		{ get<0>(__t) } -> convertible_to<const tuple_element_t<0, T>&>;
																		{ get<1>(__t) } -> convertible_to<const tuple_element_t<1, T>&>;
																	};
			template<typename T, typename U, typename V>
			concept __pair_like_convertible_from				=	!range<T> && !is_reference_v<T> && __pair_like<T>
																	&& constructible_from<T, U, V>
																	&& __convertible_to_non_slicing<U, tuple_element_t<0, T>>
																	&& convertible_to<V, tuple_element_t<1, T>>;
		}
		namespace views { struct __drop; }
		enum class subrange_kind : bool { unsized, sized };
		template<input_or_output_iterator IT, sentinel_for<IT> ST = IT, subrange_kind K = sized_sentinel_for<ST, IT> ? subrange_kind::sized : subrange_kind::unsized>
		requires(K == subrange_kind::sized || !sized_sentinel_for<ST, IT>)
		class subrange : public view_interface<subrange<IT, ST, K>>
		{
			constexpr static bool __store_size	= (K == subrange_kind::sized && !sized_sentinel_for<ST, IT>);
			IT __beg							= IT();
			[[no_unique_address]] ST __end		= ST();
			using __size_type					= typename std::make_unsigned<iter_difference_t<IT>>::type;
			template<typename T, bool = __store_size>
			struct __maybe_size {
				[[gnu::always_inline]]
				constexpr __maybe_size(T = T()) noexcept {}
			};
			template<typename T>
			struct __maybe_size<T, true> {
				T __size;
				[[gnu::always_inline]] constexpr __maybe_size(T t = T()) noexcept : __size(t) {}
			};
			[[no_unique_address]] __maybe_size<__size_type> __size{};
			template<typename RT> struct __input_r			: bool_constant<not_self<RT, subrange> && borrowed_range<RT>> {};
			template<typename RT> struct __input_i			: bool_constant<__detail::__convertible_to_non_slicing<iterator_t<RT>, IT>> {};
			template<typename RT> struct __input_s			: bool_constant<convertible_to<sentinel_t<RT>, ST>> {};
			template<typename RT> struct __input			: __and_<__input_r<RT>, __input_i<RT>, __input_s<RT>> {};
			template<typename PT> struct __pair_like_cvt	: bool_constant<not_self<subrange, PT> && __detail::__pair_like_convertible_from<PT, IT const&, ST const&>> {};
		public:
			subrange() requires(default_initializable<IT>)	= default;
			constexpr subrange(__detail::__convertible_to_non_slicing<IT> auto i, ST s)
			noexcept(is_nothrow_constructible_v<IT, decltype(i)> && is_nothrow_constructible_v<ST, ST&>)
			requires(!__store_size) :
				__beg(i),
				__end(s)
			{}
			constexpr subrange(__detail::__convertible_to_non_slicing<IT> auto i, ST s, __size_type n)
			noexcept(is_nothrow_constructible_v<IT, decltype(i)> && is_nothrow_constructible_v<ST, ST&>)
			requires(K == subrange_kind::sized) :
				__beg(i),
				__end(s),
				__size(n)
			{}
			template<satisfies<__input> RT>
			constexpr subrange(RT&& r)
			noexcept(noexcept(subrange(ranges::begin(r), ranges::end(r))))
			requires(!__store_size) : subrange(ranges::begin(r), ranges::end(r)) {}
			template<satisfies<__input> RT>
			constexpr subrange(RT&& r, __size_type n)
			noexcept(noexcept(subrange(ranges::begin(r), ranges::end(r), n)))
			requires(K == subrange_kind::sized) : subrange(ranges::begin(r), ranges::end(r), n) {}
			template<satisfies<__pair_like_cvt> PT> constexpr operator PT() const { return PT(__beg, __end); }
			constexpr IT begin() const requires(copyable<IT>) { return __beg; }
			[[nodiscard]] constexpr IT begin() const requires(!copyable<IT>) { return std::move(__beg); }
			constexpr ST end() const { return __end; }
			constexpr bool empty() const { return __beg == __end; }
			constexpr __size_type size() const requires(K == subrange_kind::sized)
			{
				if constexpr(__store_size)
					return __size.__size;
				else return static_cast<__size_type>(__end - __beg);
			}
			constexpr subrange advance(iter_difference_t<IT> n)
			{
				if constexpr(bidirectional_iterator<IT>)
				{
					if(n < 0)
					{
						ranges::advance(__beg, n);
						if constexpr(__store_size)
							__size.__size	+= static_cast<__size_type>(-n);
						return *this;
					}
				}
				auto diff			= n - ranges::advance(__beg, n, __end);
				if constexpr(__store_size)
					__size.__size	-= static_cast<__size_type>(diff);
				return *this;
			}
			[[nodiscard]] constexpr subrange next(iter_difference_t<IT> n = 1) const& requires(forward_iterator<IT>)
			{
				subrange that	= *this;
				that.advance(n);
				return that;
			}
			[[nodiscard]] constexpr subrange prev(iter_difference_t<IT> n = 1) const requires(bidirectional_iterator<IT>)
			{
				subrange that	= *this;
				that.advance(-n);
				return that;
			}
			[[nodiscard]] constexpr subrange next(iter_difference_t<IT> n = 1)&& {
				advance(n);
				return std::move(*this);
			}
		};
		template<input_or_output_iterator IT, sentinel_for<IT> ST> subrange(IT, ST) -> subrange<IT, ST>;
		template<input_or_output_iterator IT, sentinel_for<IT> ST> subrange(IT, ST, make_unsigned_t<iter_difference_t<IT>>) -> subrange<IT, ST, subrange_kind::sized>;
		template<borrowed_range RT> subrange(RT&&) -> subrange<iterator_t<RT>, sentinel_t<RT>, (sized_range<RT> || sized_sentinel_for<sentinel_t<RT>, iterator_t<RT>>) ? subrange_kind::sized : subrange_kind::unsized>;
		template<borrowed_range RT> subrange(RT&&, make_unsigned_t<range_difference_t<RT>>) -> subrange<iterator_t<RT>, sentinel_t<RT>, subrange_kind::sized>;
		template<size_t N, class IT, class ST, subrange_kind K>
		requires((N == 0 && copyable<IT>) || N == 1)
		constexpr auto get(subrange<IT, ST, K> const& r)
		{
			if constexpr(N == 0)
				return r.begin();
			else return r.end();
		}

		template<size_t N, class IT, class ST, subrange_kind K>
		requires(N < 2)
		constexpr auto get(subrange<IT, ST, K>&& r)
		{
			if constexpr(N == 0)
				return r.begin();
			else return r.end();
		}
	}
}
#endif