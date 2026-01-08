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
			friend struct views::__drop;
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
		template<size_t N, typename IT, typename ST, subrange_kind K>
		requires((N == 0 && copyable<IT>) || N == 1)
		constexpr auto get(subrange<IT, ST, K> const& r)
		{
			if constexpr(N == 0)
				return r.begin();
			else return r.end();
		}
		template<size_t N, typename IT, typename ST, subrange_kind K>
		requires(N < 2)
		constexpr auto get(subrange<IT, ST, K>&& r)
		{
			if constexpr(N == 0)
				return r.begin();
			else return r.end();
		}
		template<typename IT, typename ST, subrange_kind K> constexpr inline bool enable_borrowed_range<subrange<IT, ST, K>> = true;
		template<range RT> using borrowed_subrange_t = conditional_t<borrowed_range<RT>, subrange<iterator_t<RT>>, dangling>;
		namespace __detail
		{
			template<typename> constexpr inline bool __is_subrange = false;
			template<typename IT, typename ST, subrange_kind K> constexpr inline bool __is_subrange<subrange<IT, ST, K>> = true;
			template<typename T> using __memchr_searchable	= __and_<__or_<is_integral<T>, is_enum<T>>, bool_constant<sizeof(T) == sizeof(char)>>;
			template<input_iterator IT, sentinel_for<IT> ST> using __contiguous_sized = bool_constant<contiguous_iterator<IT> && sized_sentinel_for<ST, IT>>;
			template<input_iterator IT, sentinel_for<IT> ST, typename PT, typename VT>
			using __use_memchr_for_find	= __and_<is_same<PT, identity>, __memchr_searchable<VT>, __memchr_searchable<iter_value_t<IT>>, __contiguous_sized<IT, ST>>;
		}
		struct __find_fn
		{
			template<input_iterator IT, sentinel_for<IT> ST, typename PT = identity, typename VT = projected_value_t<IT, PT>> 
			requires indirect_binary_predicate<ranges::equal_to, projected<IT, PT>, VT const*>
			[[nodiscard]] constexpr IT operator()(IT start, ST finish, VT const& value, PT proj = {}) const
			{
				if constexpr(__detail::__use_memchr_for_find<IT, ST, PT, VT>::value)
				{
					if(!is_constant_evaluated())
					{
						auto n	= finish - start;
						if(static_cast<iter_value_t<IT>>(value) == value) [[likely]]
						{
							if(n > 0)
							{
								const void* ptr = to_address(start);
								void* result	= __builtin_memchr(ptr, static_cast<int>(value), static_cast<size_t>(n));
								if(result) n	= static_cast<char*>(result) - static_cast<const char*>(ptr);
							}
						}
						return start + n;
					}
				}
				while(start != finish && !(std::__invoke(proj, *start) == value)) ++start;
				return start;
			}
			template<input_range RT, typename PT = identity, typename VT = projected_value_t<iterator_t<RT>, PT>>
			requires indirect_binary_predicate<ranges::equal_to, projected<iterator_t<RT>, PT>, VT const*> [[nodiscard]]
			constexpr borrowed_iterator_t<RT> operator()(RT&& r, VT const& v, PT p = {}) const { return (*this)(ranges::begin(r), ranges::end(r), v, std::move(p)); }
		};
		template<bool QV>
		struct __find_if_pred_fn
		{
			template<input_iterator IT, sentinel_for<IT> ST, typename PT = identity, indirect_unary_predicate<projected<IT, PT>> QT>
			[[nodiscard]] constexpr IT operator()(IT start, ST finish, QT pred, PT proj = {}) const
			{
				while(start != finish && static_cast<bool>(std::__invoke(pred, std::__invoke(proj, *start))) != QV)
					++start;
				return start;
			}
			template<input_range RT, typename PT = identity, indirect_unary_predicate<projected<iterator_t<RT>, PT>> QT>
			[[nodiscard]] constexpr borrowed_iterator_t<RT> operator()(RT&& r, QT q, PT p = {}) const { return (*this)(ranges::begin(r), ranges::end(r), std::move(q), std::move(p)); }
		};
		template<typename I1, typename I2>
		struct in_in_result
		{
			[[no_unique_address]] I1 in1;
			[[no_unique_address]] I2 in2;
			template<convertible_from<I1 const&> J1, convertible_from<I2 const&> J2>
			constexpr operator in_in_result<J1, J2>() const& { return { in1, in2 }; }
			template<convertible_from<I1> J1, convertible_from<I2> J2>
			constexpr operator in_in_result<J1, J2>()&& { return { std::move(in1), std::move(in2) }; }
		};
		template<typename I1, typename I2> using mismatch_result = in_in_result<I1, I2>;
		struct __mismatch_fn
		{
			template<input_iterator I1, sentinel_for<I1> S1, input_iterator I2, sentinel_for<I2> S2, typename QT = ranges::equal_to, typename P1 = identity, typename P2 = identity>
			requires indirectly_comparable<I1, I2, QT, P1, P2>
			[[nodiscard]] constexpr mismatch_result<I1, I2> operator()(I1 i1, S1 s1, I2 i2, S2 s2, QT pred = {}, P1 p1 = {}, P2 p2 = {}) const
			{
				while(i1 != s1 && i2 != s2)
				{
					if(static_cast<bool>(std::__invoke(pred, std::__invoke(p1, *i1), std::__invoke(p2, *i2)))) break;
					++i1;
					++i2;
				}
				return { i1, i2 };
			}
			template<input_range R1, input_range R2, typename QT = ranges::equal_to, typename P1 = identity, typename P2 = identity>
			requires indirectly_comparable<iterator_t<R1>, iterator_t<R2>, QT, P1, P2>
			[[nodiscard]] constexpr mismatch_result<iterator_t<R1>, iterator_t<R2>> operator()(R1&& r1, R2&& r2, QT pred = {}, P1 p1 = {}, P2 p2 = {}) const
			{
				return 	(*this)(ranges::begin(r1), ranges::end(r1),
								ranges::begin(r2), ranges::end(r2),
								std::move(pred), std::move(p1), std::move(p2));
			}
		};
		struct __search_fn
		{
			template<input_iterator I1, sentinel_for<I1> S1, input_iterator I2, sentinel_for<I2> S2, typename QT = ranges::equal_to, typename P1 = identity, typename P2 = identity>
			requires indirectly_comparable<I1, I2, QT, P1, P2>
			[[nodiscard]] constexpr subrange<I1> operator()(I1 i1, S1 s1, I2 i2, S2 s2, QT pred = {}, P1 p1 = {}, P2 p2 = {}) const
			{
				for(I1 i = i1; !(i2 == s2); ++i)
				{
					I2 j;
					for(j = i2; !(i == s1) && !std::__invoke(pred, std::__invoke(p1, *i), std::__invoke(p2, *j)); ++i);
					if(i == s1)	return { i, i };
					++j;
					for(I1 curr	= i; std::__invoke(pred, std::__invoke(p1, *curr), std::__invoke(p2, *j)); ++j)
					{
						if(j == s2)
							return { i, ++curr };
						if(++curr == s1)
							return { curr, curr };
					}
				}
				return { i1, i1 };
			}
			template<forward_range R1, forward_range R2, typename QT = ranges::equal_to, typename P1 = identity, typename P2 = identity>
			requires indirectly_comparable<iterator_t<R1>, iterator_t<R2>, QT, P1, P2>
			[[nodiscard]] constexpr borrowed_subrange_t<R1> operator()(R1&& r1, R2&& r2, QT pred = {}, P1 p1 = {}, P2 p2 = {}) const
			{
				return	(*this)(ranges::begin(r1), ranges::end(r1),
								ranges::begin(r2), ranges::end(r2),
								std::move(pred), std::move(p1), std::move(p2));
			}
		};
		struct __min_fn
		{
			template<typename T, typename PT = identity, indirect_strict_weak_order<projected<T const*, PT>> CT = ranges::less>
			[[nodiscard]] constexpr T const& operator()(T const& a, T const& b, CT cmp = {}, PT proj = {}) const
			{
				if(std::__invoke(cmp, std::__invoke(proj, a), std::__invoke(proj, b)))
					return a;
				else return b;
			}
			template<input_range RT, typename PT = identity, indirect_strict_weak_order<projected<iterator_t<RT>, PT>> CT = ranges::less>
			[[nodiscard]] constexpr range_value_t<RT> operator()(RT&& r, CT cmp = {}, PT proj = {}) const
			{
				auto start	= ranges::begin(r);
				auto finish	= ranges::end(r);
				__libk_assert(start != finish);
				range_value_t<RT> result(*start);
				while(++start != finish)
				{
					auto&& tmp = *start;
					if(std::__invoke(cmp, std::__invoke(proj, tmp), std::__invoke(proj, result)))
						result = std::forward<decltype(tmp)>(tmp);
				}
				return result;
			}
			template<copyable T, typename PT = identity, indirect_strict_weak_order<projected<T const*, PT>> CT = ranges::less>
			[[nodiscard]] constexpr T operator()(initializer_list<T> ini, CT cmp = {}, PT proj = {}) const { return (*this)(ranges::subrange(ini), std::move(cmp), std::move(proj)); }
		};
		struct __adjacent_find_fn
		{
			template<input_iterator IT, sentinel_for<IT> ST, typename PT = identity, indirect_binary_predicate<projected<IT, PT>, projected<IT, PT>> QT = ranges::equal_to>
			[[nodiscard]] constexpr IT operator()(IT start, ST finish, QT pred = {}, PT proj = {}) const
			{
				if(start == finish) return start;
				IT next		= start;
				for(++next; next != finish; ++next)
				{
					if(std::__invoke(pred, std::__invoke(proj, *start), std::__invoke(proj, *next)))
						return start;
					start	= next;
				}
				return next;
			}
			template<forward_range RT, typename PT = identity, indirect_binary_predicate<projected<iterator_t<RT>, PT>, projected<iterator_t<RT>, PT>> QT = ranges::equal_to>
			[[nodiscard]] constexpr borrowed_iterator_t<RT> operator()(RT&& r, QT q = {}, PT p = {}) const { return (*this)(ranges::begin(r), ranges::end(r), std::move(q), std::move(p)); }
		};
		constexpr inline __find_fn find{};
		constexpr inline __find_if_pred_fn<true> find_if{};
		constexpr inline __find_if_pred_fn<false> find_if_not{};
		constexpr inline __mismatch_fn mismatch{};
		constexpr inline __search_fn search{};
		constexpr inline __min_fn min{};
		constexpr inline __adjacent_find_fn adjacent_find{};
		template<range RT> requires(is_object_v<RT>)
		class ref_view : public view_interface<ref_view<RT>>
		{
			RT* __range;
			static void __fun(RT&); // not defined
			static void __fun(RT&&) = delete;
		public:
			template<not_self<ref_view> T> requires(convertible_to<T, RT&> && requires { __fun(declval<T>()); })
			constexpr ref_view(T&& t)
			noexcept(noexcept(static_cast<RT&>(std::declval<T>()))) : __range(std::addressof(static_cast<RT&>(std::forward<T>(t)))) {}
			constexpr RT& base() const { return *__range; }
			constexpr iterator_t<RT> begin() const { return ranges::begin(*__range); }
			constexpr sentinel_t<RT> end() const { return ranges::end(*__range); }
			constexpr bool empty() const requires(requires{ ranges::empty(*__range); }) { return ranges::empty(*__range); }
			constexpr auto size() const requires(sized_range<RT>) { return ranges::size(*__range); }
			constexpr auto data() const requires(contiguous_range<RT>) { return ranges::data(*__range); }
		};
		template<typename RT> ref_view(RT&) -> ref_view<RT>;
		template<typename RT> constexpr inline bool enable_borrowed_range<ref_view<RT>> = true;
		template<range RT> requires(movable<RT> && !__detail::__is_initializer_list<remove_cv_t<RT>>)
		class owning_view : public view_interface<owning_view<RT>>
		{
			RT __range = RT();
		public:
			owning_view() requires(default_initializable<RT>) = default;
			constexpr owning_view(RT&& r) noexcept(is_nothrow_move_constructible_v<RT>) : __range(std::move(r)) {}
			owning_view(owning_view&&) = default;
			owning_view(owning_view const&) = default;
			constexpr RT& base()& noexcept { return __range; }
			constexpr RT const& base() const& noexcept { return __range; }
			constexpr RT&& base()&& noexcept { return move(__range); }
			constexpr const RT&& base() const&& noexcept { return move(__range); }
			constexpr iterator_t<RT> begin() { return ranges::begin(__range); }
			constexpr sentinel_t<RT> end() { return ranges::end(__range); }
			constexpr bool empty() requires(requires{ ranges::empty(__range); }) { return ranges::empty(__range); }
			constexpr auto size() requires(sized_range<RT>) { return ranges::size(__range); }
			constexpr auto data() requires(contiguous_range<RT>) { return ranges::data(__range); }
			constexpr auto begin() const requires(range<const RT>) { return ranges::begin(__range); }
			constexpr auto end() const requires(range<const RT>) { return ranges::end(__range); }
			constexpr auto size() const requires(sized_range<const RT>) { return ranges::size(__range); }
			constexpr auto data() const requires(contiguous_range<const RT>) { return ranges::data(__range); }
			constexpr bool empty() const requires(requires{ ranges::empty(__range); }) { return ranges::empty(__range); }
		};
		template<typename T> constexpr bool enable_borrowed_range<owning_view<T>> = enable_borrowed_range<T>;
	}
	using ranges::get;
	template<typename IT, typename ST, ranges::subrange_kind K>	struct tuple_size<ranges::subrange<IT, ST, K>> : integral_constant<size_t, 2UZ> {};
	template<typename IT, typename ST, ranges::subrange_kind K>	struct tuple_element<0, ranges::subrange<IT, ST, K>> { typedef IT type; };
	template<typename IT, typename ST, ranges::subrange_kind K>	struct tuple_element<1, ranges::subrange<IT, ST, K>> { typedef ST type; };
	template<typename IT, typename ST, ranges::subrange_kind K>	struct tuple_element<0, const ranges::subrange<IT, ST, K>> { typedef IT type; };
	template<typename IT, typename ST, ranges::subrange_kind K>	struct tuple_element<1, const ranges::subrange<IT, ST, K>> { typedef ST type; };
}
#endif