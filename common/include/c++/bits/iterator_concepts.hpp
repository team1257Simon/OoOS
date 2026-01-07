#ifndef __ITERATOR_CONCEPTS
#define __ITERATOR_CONCEPTS
#include <bits/ptr_traits.hpp>
#include <bits/ranges_cmp.hpp>
namespace std
{
	struct input_iterator_tag;
	struct output_iterator_tag;
	struct forward_iterator_tag;
	struct bidirectional_iterator_tag;
	struct random_access_iterator_tag;
	struct contiguous_iterator_tag;
	template<typename IT> struct iterator_traits;
	template<typename T> requires is_object_v<T> struct iterator_traits<T*>;
	template<typename IT, typename> struct __iterator_traits;
	namespace __detail
	{
		template<typename T> using __with_ref = T&;
		template<typename T> concept __can_reference = requires { typename __with_ref<T>; };
		template<typename T> concept __dereferenceable = requires(T& t) { { *t } -> __can_reference; };
		template<typename IT, typename T2> concept __points_to = convertible_to<decltype(*declval<IT>()), T2>;
	}
	template<__detail::__dereferenceable T> using iter_reference_t = decltype(*declval<T&>());
	namespace ranges
	{
		namespace __cust_imove
		{
			void iter_move();
			template<typename T> concept __adl_imove = (std::__detail::__class_or_enum<remove_reference_t<T>>) && requires(T&& t) { iter_move(static_cast<T&&>(t)); };
			struct __imove
			{
			private:
				template<typename T> struct __result { using type = iter_reference_t<T>; };
				template<typename T> requires(__adl_imove<T>) struct __result<T> { using type = decltype(iter_move(declval<T>())); };
				template<typename T> requires(!__adl_imove<T> && is_lvalue_reference_v<iter_reference_t<T>>) struct __result<T> { using type = remove_reference_t<iter_reference_t<T>>&&; };
				template<typename T> static constexpr bool _is_noexcept() { if constexpr (__adl_imove<T>) return noexcept(iter_move(declval<T>())); else return noexcept(*declval<T>()); }
			public:
				template<std::__detail::__dereferenceable T> using __type = typename __result<T>::type;
				template<std::__detail::__dereferenceable T> constexpr __type<T> operator()(T&& __e) const noexcept(_is_noexcept<T>()) { if constexpr (__adl_imove<T>) return iter_move(static_cast<T&&>(__e)); else if constexpr (is_lvalue_reference_v<iter_reference_t<T>>) return static_cast<__type<T>>(*__e); else return *__e; }
			};
		}
		inline namespace __cust { inline constexpr __cust_imove::__imove iter_move{}; }
	}
	template<__detail::__dereferenceable T> requires(__detail::__can_reference<ranges::__cust_imove::__imove::__type<T&>>) using iter_rvalue_reference_t = ranges::__cust_imove::__imove::__type<T&>;
	template<typename> struct incrementable_traits {};
	template<typename T> requires(is_object_v<T>) struct incrementable_traits<T*> { using difference_type = ptrdiff_t; };
	template<typename IT> struct incrementable_traits<const IT> : incrementable_traits<IT> {};
	template<typename T> requires requires { typename T::difference_type; } struct incrementable_traits<T> { using difference_type = typename T::difference_type; };
	template<typename T> requires(!requires { typename T::difference_type; } && requires(const T& __a, const T& __b) { { __a - __b } -> integral; }) struct incrementable_traits<T> { using difference_type = typename make_signed<decltype(declval<T>() - declval<T>())>::type; };
	#if defined __STRICT_ANSI__ && defined __SIZEOF_INT128__
	template<> struct incrementable_traits<__int128> { using difference_type = __int128; };
	template<> struct incrementable_traits<unsigned __int128> { using difference_type = __int128; };
	#endif
	namespace __detail
	{
		template<typename IT> concept __primary_traits_iter = __is_base_of(__iterator_traits<IT, void>, iterator_traits<IT>);
		template<typename IT, typename T> struct __iter_traits_impl { using type = iterator_traits<IT>; };
		template<typename IT, typename T> requires(__primary_traits_iter<IT>) struct __iter_traits_impl<IT, T> { using type = T; };
		template<typename IT, typename T = IT> using __iter_traits = typename __iter_traits_impl<IT, T>::type;
		template<typename T> using __iter_diff_t = typename __iter_traits<T, incrementable_traits<T>>::difference_type;
	}
	template<typename T> using iter_difference_t = __detail::__iter_diff_t<remove_cvref_t<T>>;
	namespace __detail
	{
		template<typename> struct __cond_value_type {};
		template<typename T> requires(is_object_v<T>) struct __cond_value_type<T> { using value_type = remove_cv_t<T>; };
		template<typename T> concept __has_member_value_type = requires { typename T::value_type; };
		template<typename T> concept __has_member_element_type = requires { typename T::element_type; };
	}
	template<typename> struct indirectly_readable_traits {};
	template<typename T> struct indirectly_readable_traits<T*> : __detail::__cond_value_type<T> {};
	template<typename IT> requires(is_array_v<IT>) struct indirectly_readable_traits<IT> { using value_type = remove_cv_t<remove_extent_t<IT>>; };
	template<typename IT> struct indirectly_readable_traits<const IT> : indirectly_readable_traits<IT> {};
	template<__detail::__has_member_value_type T> struct indirectly_readable_traits<T> : __detail::__cond_value_type<typename T::value_type> {};
	template<__detail::__has_member_element_type T> struct indirectly_readable_traits<T> : __detail::__cond_value_type<typename T::element_type> {};
	template<__detail::__has_member_value_type T> requires(__detail::__has_member_element_type<T> && same_as<remove_cv_t<typename T::element_type>, remove_cv_t<typename T::value_type>>) struct indirectly_readable_traits<T> : __detail::__cond_value_type<typename T::value_type> {};
	template<__detail::__has_member_value_type T> requires(__detail::__has_member_element_type<T>) struct indirectly_readable_traits<T> {};
	namespace __detail
	{
		template<typename T> using __iter_value_t = typename __iter_traits<T, indirectly_readable_traits<T>>::value_type;
		template<typename IT> concept __cpp17_iterator = requires(IT i) { { *i } -> __can_reference; { ++i } -> same_as<IT&>; { *i++ } -> __can_reference; } && copyable<IT>;
		template<typename IT> concept __cpp17_input_iterator = __cpp17_iterator<IT> && equality_comparable<IT> && requires(IT i)
		{
			typename incrementable_traits<IT>::difference_type;
			typename indirectly_readable_traits<IT>::value_type;
			typename common_reference_t<iter_reference_t<IT>&&, typename indirectly_readable_traits<IT>::value_type&>;
			typename common_reference_t<decltype(*i++)&&, typename indirectly_readable_traits<IT>::value_type&>;
			requires(signed_integral<typename incrementable_traits<IT>::difference_type>);
		};
		template<typename IT> concept __cpp17_fwd_iterator = __cpp17_input_iterator<IT> && constructible_from<IT> && is_lvalue_reference_v<iter_reference_t<IT>> && same_as<remove_cvref_t<iter_reference_t<IT>>, typename indirectly_readable_traits<IT>::value_type> && requires(IT i) { {  i++ } -> convertible_to<const IT&>; { *i++ } -> same_as<iter_reference_t<IT>>; };
		template<typename IT> concept __cpp17_bidi_iterator = __cpp17_fwd_iterator<IT> && requires(IT i) { {  --i } -> same_as<IT&>; {  i-- } -> convertible_to<const IT&>;{ *i-- } -> same_as<iter_reference_t<IT>>; };
		template<typename IT> concept __cpp17_randacc_iterator = __cpp17_bidi_iterator<IT> && totally_ordered<IT> && requires(IT i, typename incrementable_traits<IT>::difference_type n)
		{
			{ i += n } -> same_as<IT&>;
			{ i -= n } -> same_as<IT&>;
			{ i +  n } -> same_as<IT>;
			{ n +  i } -> same_as<IT>;
			{ i -  n } -> same_as<IT>;
			{ i -  i } -> same_as<decltype(n)>;
			{ i[n] } -> convertible_to<iter_reference_t<IT>>;
		};
		template<typename IT> concept __iter_with_nested_types = requires { typename IT::iterator_category; typename IT::value_type; typename IT::difference_type; typename IT::reference; };
		template<typename IT> concept __iter_without_nested_types = !__iter_with_nested_types<IT>;
		template<typename IT> concept __iter_without_category = !requires { typename IT::iterator_category; };
	}
	template<typename T> using iter_value_t = __detail::__iter_value_t<remove_cvref_t<T>>;
	template<typename IT> requires(__detail::__iter_with_nested_types<IT>) struct __iterator_traits<IT, void> {
	private:
		template<typename JT> struct __ptr { using type = void; };
		template<typename JT> requires requires { typename JT::pointer; } struct __ptr<JT> { using type = typename JT::pointer; };
	public:
		using iterator_category	= typename IT::iterator_category;
		using value_type		= typename IT::value_type;
		using difference_type	= typename IT::difference_type;
		using pointer			= typename __ptr<IT>::type;
		using reference			= typename IT::reference;
	};
	template<typename IT>
	requires(__detail::__iter_without_nested_types<IT> && __detail::__cpp17_input_iterator<IT>)
	struct __iterator_traits<IT, void>
	{
	private:
		template<typename JT> struct __cat { using type = input_iterator_tag; };
		template<typename JT> requires requires { typename JT::iterator_category; } struct __cat<JT> { using type = typename JT::iterator_category; };
		template<typename JT> requires(__detail::__iter_without_category<JT> && __detail::__cpp17_randacc_iterator<JT>) struct __cat<JT> { using type = random_access_iterator_tag; };
		template<typename JT> requires(__detail::__iter_without_category<JT> && __detail::__cpp17_bidi_iterator<JT>) struct __cat<JT> { using type = bidirectional_iterator_tag; };
		template<typename JT> requires(__detail::__iter_without_category<JT> && __detail::__cpp17_fwd_iterator<JT>) struct __cat<JT> { using type = forward_iterator_tag; };
		template<typename JT> struct __ptr { using type = void; };
		template<typename JT> requires requires { typename JT::pointer; } struct __ptr<JT> { using type = typename JT::pointer; };
		template<typename JT> requires(!requires { typename JT::pointer; } && requires(JT& i) { i.operator->(); }) struct __ptr<JT> { using type = decltype(declval<JT&>().operator->()); };
		template<typename JT> struct __ref { using type = iter_reference_t<JT>; };
		template<typename JT> requires requires { typename JT::reference; } struct __ref<JT> { using type = typename JT::reference; };
	public:
		using iterator_category	= typename __cat<IT>::type;
		using value_type		= typename indirectly_readable_traits<IT>::value_type;
		using difference_type	= typename incrementable_traits<IT>::difference_type;
		using pointer			= typename __ptr<IT>::type;
		using reference			= typename __ref<IT>::type;
	};
	template<typename IT> requires(__detail::__iter_without_nested_types<IT> && __detail::__cpp17_iterator<IT>)
	struct __iterator_traits<IT, void>
	{
	private:
		template<typename JT> struct __diff { using type = void; };
		template<typename JT> requires requires { typename incrementable_traits<JT>::difference_type; } struct __diff<JT> { using type = typename incrementable_traits<JT>::difference_type; };
	public:
		using iterator_category = output_iterator_tag;
		using value_type	    = void;
		using difference_type   = typename __diff<IT>::type;
		using pointer	        = void;
		using reference	        = void;
	};
	namespace __detail
	{
		template<typename IT> struct __iter_concept_impl;
		template<typename IT> requires requires { typename __iter_traits<IT>::iterator_concept; } struct __iter_concept_impl<IT> { using type = typename __iter_traits<IT>::iterator_concept; };
		template<typename IT> requires(!requires { typename __iter_traits<IT>::iterator_concept; } && requires { typename __iter_traits<IT>::iterator_category; }) struct __iter_concept_impl<IT> { using type = typename __iter_traits<IT>::iterator_category; };
		template<typename IT> requires(!requires { typename __iter_traits<IT>::iterator_concept; } && !requires { typename __iter_traits<IT>::iterator_category; } && __primary_traits_iter<IT>) struct __iter_concept_impl<IT> { using type = random_access_iterator_tag; };
		template<typename IT> struct __iter_concept_impl {};
		template<typename IT> using __iter_concept				= typename __iter_concept_impl<IT>::type;
		template<typename I> concept __indirectly_readable_impl	= requires
		{
			typename iter_value_t<I>;
			typename iter_reference_t<I>;
			typename iter_rvalue_reference_t<I>;
			requires(same_as<iter_reference_t<const I>, iter_reference_t<I>>);
			requires(same_as<iter_rvalue_reference_t<const I>, iter_rvalue_reference_t<I>>);
		} && common_reference_with<iter_reference_t<I>&&, iter_value_t<I>&> && common_reference_with<iter_reference_t<I>&&, iter_rvalue_reference_t<I>&&> && common_reference_with<iter_rvalue_reference_t<I>&&, const iter_value_t<I>&>;
	}
	template<typename I> concept indirectly_readable				= __detail::__indirectly_readable_impl<remove_cvref_t<I>>;
	template<indirectly_readable T> using iter_common_reference_t	= common_reference_t<iter_reference_t<T>, iter_value_t<T>&>;
	template<typename O, typename T> concept indirectly_writable	= requires(O&& o, T&& t)
	{
		*o														= forward<T>(t);
		*forward<O>(o)											= forward<T>(t);
		const_cast<const iter_reference_t<O>&&>(*o)				= forward<T>(t);
		const_cast<const iter_reference_t<O>&&>(*forward<O>(o))	= forward<T>(t);
	};
	namespace ranges::__detail
	{
		class __max_diff_type;
		class __max_size_type;
		template<typename T> concept __is_signed_int128			= same_as<T, __int128>;
		template<typename T> concept __is_unsigned_int128		= same_as<T, unsigned __int128>;
		template<typename T> concept __cv_bool					= same_as<const volatile T, const volatile bool>;
		template<typename T> concept __integral_nonbool			= integral<T> && !__cv_bool<T>;
		template<typename T> concept __is_int128				= __is_signed_int128<T> || __is_unsigned_int128<T>;
		template<typename T> concept __is_integer_like			= __integral_nonbool<T> || __is_int128<T> || same_as<T, __max_diff_type> || same_as<T, __max_size_type>;
		template<typename T> concept __is_signed_integer_like	= signed_integral<T> || __is_signed_int128<T> || same_as<T, __max_diff_type>;
	}
	namespace __detail { using ranges::__detail::__is_signed_integer_like; }
	template<typename IT> concept weakly_incrementable = movable<IT> && requires(IT i) { typename iter_difference_t<IT>; requires __detail::__is_signed_integer_like<iter_difference_t<IT>>; { ++i } -> same_as<IT&>; i++; };
	template<typename IT> concept incrementable = regular<IT> && weakly_incrementable<IT> && requires(IT i) { { i++ } -> same_as<IT>; };
	template<typename IT> concept input_or_output_iterator = requires(IT i) { { *i } -> __detail::__can_reference; } && weakly_incrementable<IT>;
	template<typename ST, typename IT> concept sentinel_for = semiregular<ST> && input_or_output_iterator<IT> && __detail::__weakly_eq_cmp_with<ST, IT>;
	template<typename ST, typename IT> inline constexpr bool disable_sized_sentinel_for	= false;
	template<typename ST, typename IT> concept sized_sentinel_for = sentinel_for<ST, IT> && !disable_sized_sentinel_for<remove_cv_t<ST>, remove_cv_t<IT>> && requires(const IT& i, const ST& __s) { { __s - i } -> same_as<iter_difference_t<IT>>; { i - __s } -> same_as<iter_difference_t<IT>>; };
	template<typename IT> concept input_iterator = input_or_output_iterator<IT> && indirectly_readable<IT> && requires { typename __detail::__iter_concept<IT>; } && derived_from<__detail::__iter_concept<IT>, input_iterator_tag>;
	template<typename IT, typename T> concept output_iterator = input_or_output_iterator<IT> && indirectly_writable<IT, T> && requires(IT i, T&& t) { *i++ = forward<T>(t); };
	template<typename IT> concept forward_iterator = input_iterator<IT> && derived_from<__detail::__iter_concept<IT>, forward_iterator_tag> && incrementable<IT> && sentinel_for<IT, IT>;
	template<typename IT> concept bidirectional_iterator = forward_iterator<IT> && derived_from<__detail::__iter_concept<IT>, bidirectional_iterator_tag> && requires(IT i) { { --i } -> same_as<IT&>; { i-- } -> same_as<IT>; };
	template<typename IT> concept random_access_iterator = bidirectional_iterator<IT> && derived_from<__detail::__iter_concept<IT>, random_access_iterator_tag> && totally_ordered<IT> && sized_sentinel_for<IT, IT> && requires(IT i, const IT j, const iter_difference_t<IT> n)
	{
		{ i += n } -> same_as<IT&>;
		{ j +  n } -> same_as<IT>;
		{ n +  j } -> same_as<IT>;
		{ i -= n } -> same_as<IT&>;
		{ j -  n } -> same_as<IT>;
		{  j[n]  } -> same_as<iter_reference_t<IT>>;
	};
	template<typename IT> concept contiguous_iterator = random_access_iterator<IT> && derived_from<__detail::__iter_concept<IT>, contiguous_iterator_tag> && is_lvalue_reference_v<iter_reference_t<IT>> && same_as<iter_value_t<IT>, remove_cvref_t<iter_reference_t<IT>>> && requires(const IT& i) { { to_address(i) } -> same_as<add_pointer_t<iter_reference_t<IT>>>; };
	template<typename FT, typename IT> concept indirectly_unary_invocable = indirectly_readable<IT> && copy_constructible<FT> && invocable<FT&, iter_value_t<IT>&> && invocable<FT&, iter_reference_t<IT>> && invocable<FT&, iter_common_reference_t<IT>> && common_reference_with<invoke_result_t<FT&, iter_value_t<IT>&>, invoke_result_t<FT&, iter_reference_t<IT>>>;
	template<typename FT, typename IT> concept indirectly_regular_unary_invocable = indirectly_readable<IT> && copy_constructible<FT> && regular_invocable<FT&, iter_value_t<IT>&> && regular_invocable<FT&, iter_reference_t<IT>> && regular_invocable<FT&, iter_common_reference_t<IT>> && common_reference_with<invoke_result_t<FT&, iter_value_t<IT>&>, invoke_result_t<FT&, iter_reference_t<IT>>>;
	template<typename FT, typename IT> concept indirect_unary_predicate = indirectly_readable<IT> && copy_constructible<FT> && predicate<FT&, iter_value_t<IT>&> && predicate<FT&, iter_reference_t<IT>> && predicate<FT&, iter_common_reference_t<IT>>;
	template<typename FT, typename IT, typename JT> concept indirect_binary_predicate = indirectly_readable<IT> && indirectly_readable<JT> && copy_constructible<FT> && predicate<FT&, iter_value_t<IT>&, iter_value_t<JT>&> && predicate<FT&, iter_value_t<IT>&, iter_reference_t<JT>> && predicate<FT&, iter_reference_t<IT>, iter_value_t<JT>&> && predicate<FT&, iter_reference_t<IT>, iter_reference_t<JT>> && predicate<FT&, iter_common_reference_t<IT>, iter_common_reference_t<JT>>;
	template<typename FT, typename IT, typename JT = IT> concept indirect_equivalence_relation = indirectly_readable<IT> && indirectly_readable<JT> && copy_constructible<FT> && equivalence_relation<FT&, iter_value_t<IT>&, iter_value_t<JT>&> && equivalence_relation<FT&, iter_value_t<IT>&, iter_reference_t<JT>> && equivalence_relation<FT&, iter_reference_t<IT>, iter_value_t<JT>&> && equivalence_relation<FT&, iter_reference_t<IT>, iter_reference_t<JT>> && equivalence_relation<FT&, iter_common_reference_t<IT>, iter_common_reference_t<JT>>;
	template<typename FT, typename IT, typename JT = IT> concept indirect_strict_weak_order = indirectly_readable<IT> && indirectly_readable<JT> && copy_constructible<FT> && strict_weak_order<FT&, iter_value_t<IT>&, iter_value_t<JT>&> && strict_weak_order<FT&, iter_value_t<IT>&, iter_reference_t<JT>> && strict_weak_order<FT&, iter_reference_t<IT>, iter_value_t<JT>&> && strict_weak_order<FT&, iter_reference_t<IT>, iter_reference_t<JT>> && strict_weak_order<FT&, iter_common_reference_t<IT>, iter_common_reference_t<JT>>;
	template<typename FT, typename... ITs> requires(indirectly_readable<ITs> && ...) && invocable<FT, iter_reference_t<ITs>...> using indirect_result_t = invoke_result_t<FT, iter_reference_t<ITs>...>;
	template<indirectly_readable IT, indirectly_regular_unary_invocable<IT> PT> struct projected { using value_type = remove_cvref_t<indirect_result_t<PT&, IT>>; indirect_result_t<PT&, IT> operator*() const; };
	template<weakly_incrementable IT, typename PT> struct incrementable_traits<projected<IT, PT>> { using difference_type = iter_difference_t<IT>; };
	template<indirectly_readable IT, indirectly_regular_unary_invocable<IT> PT> using projected_value_t = remove_cvref_t<invoke_result_t<PT&, iter_value_t<IT>&>>;
	template<typename I, typename O> concept indirectly_movable = indirectly_readable<I> && indirectly_writable<O, iter_rvalue_reference_t<I>>;
	template<typename I, typename O> concept indirectly_movable_storable = indirectly_movable<I, O> && indirectly_writable<O, iter_value_t<I>> && movable<iter_value_t<I>> && constructible_from<iter_value_t<I>, iter_rvalue_reference_t<I>> && assignable_from<iter_value_t<I>&, iter_rvalue_reference_t<I>>;
	template<typename I, typename O> concept indirectly_copyable = indirectly_readable<I> && indirectly_writable<O, iter_reference_t<I>>;
	template<typename I, typename O> concept indirectly_copyable_storable = indirectly_copyable<I, O> && indirectly_writable<O, iter_value_t<I>&> && indirectly_writable<O, const iter_value_t<I>&> && indirectly_writable<O, iter_value_t<I>&&> && indirectly_writable<O, const iter_value_t<I>&&> && copyable<iter_value_t<I>> && constructible_from<iter_value_t<I>, iter_reference_t<I>> && assignable_from<iter_value_t<I>&, iter_reference_t<I>>;
	namespace ranges
	{
		namespace __cust_iswap
		{
			template<typename IT, typename JT> void iter_swap(IT, JT) = delete;
			template<typename T, typename U> concept __adl_iswap = (std::__detail::__class_or_enum<remove_reference_t<T>> || std::__detail::__class_or_enum<remove_reference_t<U>>) && requires(T&& t, U&& __u) { iter_swap(static_cast<T&&>(t), static_cast<U&&>(__u)); };
			template<typename X, typename Y> constexpr iter_value_t<X> __iter_exchange_move(X&& __x, Y&& __y) noexcept(noexcept(iter_value_t<X>(iter_move(__x))) && noexcept(*__x = iter_move(__y))) { iter_value_t<X> __old_value(iter_move(__x)); *__x = iter_move(__y); return __old_value; }
			struct __iter_swap
			{
			private:
				template<typename T, typename U> static constexpr bool _is_noexcept() { if constexpr (__adl_iswap<T, U>) return noexcept(iter_swap(declval<T>(), declval<U>())); else if constexpr(indirectly_readable<T> && indirectly_readable<U> && swappable_with<iter_reference_t<T>, iter_reference_t<U>>) return noexcept(ranges::swap(*declval<T>(), *declval<U>())); else return noexcept(*declval<T>() = __iter_exchange_move(declval<U>(), declval<T>())); }
			public:
				template<typename T, typename U> requires __adl_iswap<T, U> || (indirectly_readable<remove_reference_t<T>> && indirectly_readable<remove_reference_t<U>> && swappable_with<iter_reference_t<T>, iter_reference_t<U>>) || (indirectly_movable_storable<T, U> && indirectly_movable_storable<U, T>)
				constexpr void operator()(T&& t, U&& u) const noexcept(_is_noexcept<T, U>()) { if constexpr (__adl_iswap<T, U>) iter_swap(static_cast<T&&>(t), static_cast<U&&>(u)); else if constexpr(indirectly_readable<T> && indirectly_readable<U> && swappable_with<iter_reference_t<T>, iter_reference_t<U>>) ranges::swap(*t, *u); else *t = __iter_exchange_move(u, t); }
			};
		}
		inline namespace __cust { inline constexpr __cust_iswap::__iter_swap iter_swap{}; }
	}
	template<typename IT, typename JT = IT> concept indirectly_swappable = indirectly_readable<IT> && indirectly_readable<JT> && requires(const IT i, const JT j)
	{
		ranges::iter_swap(i, i);
		ranges::iter_swap(j, j);
		ranges::iter_swap(i, j);
		ranges::iter_swap(j, i);
	};
	template<typename IT, typename JT, typename RT, typename PT = identity, typename QT = identity> concept indirectly_comparable = indirect_binary_predicate<RT, projected<IT, PT>, projected<JT, QT>>;
	template<typename IT> concept permutable = forward_iterator<IT> && indirectly_movable_storable<IT, IT> && indirectly_swappable<IT, IT>;
	template<typename IT, typename JT, typename O, typename RT = ranges::less, typename PT = identity, typename QT = identity> concept mergeable = input_iterator<IT> && input_iterator<JT> && weakly_incrementable<O> && indirectly_copyable<IT, O> && indirectly_copyable<JT, O> && indirect_strict_weak_order<RT, projected<IT, PT>, projected<JT, QT>>;
	template<typename IT, typename RT = ranges::less, typename PT = identity> concept sortable = permutable<IT> && indirect_strict_weak_order<RT, projected<IT, PT>>;
	struct unreachable_sentinel_t { template<weakly_incrementable IT> friend constexpr bool operator==(unreachable_sentinel_t, const IT&) noexcept { return false; } };
	inline constexpr unreachable_sentinel_t unreachable_sentinel{};
	struct default_sentinel_t {};
	inline constexpr default_sentinel_t default_sentinel{};
	namespace ranges::__cust_access
	{
		using std::__detail::__class_or_enum;
		void begin() = delete;
		template<typename T> concept __member_begin		= requires(T& t) { { __decay_copy(t.begin()) } -> input_or_output_iterator; };
		template<typename T> concept __adl_begin		= __class_or_enum<remove_reference_t<T>> && requires(T& t) { { __decay_copy(begin(t)) } -> input_or_output_iterator; };
		template<typename T> concept __cust_beginnable	= (is_array_v<T> || __member_begin<T&> || __adl_begin<T&>);
		template<__cust_beginnable T> constexpr auto __begin(T& t) { if constexpr(is_array_v<T>) return t + 0; else if constexpr(__member_begin<T&>) return t.begin(); else return begin(t); }
	}
	namespace __detail { template<typename T> using __range_iter_t = decltype(ranges::__cust_access::__begin(declval<T&>())); }
	
#pragma region nonstandard useful concepts and typedefs
	extension template<typename IT, typename T2> concept matching_input_iterator = input_iterator<IT> && __detail::__points_to<IT, T2>;
	extension template<typename IT, typename T2> concept matching_forward_iterator = forward_iterator<IT> && __detail::__points_to<IT, T2>;
	template<typename C, typename T> concept iterable = output_iterator<decltype(declval<C>().begin()), T> && output_iterator<decltype(declval<C>().end()), T>;
	template<typename C, typename T> concept const_iterable = matching_input_iterator<decltype(declval<C const&>().begin()), T> && matching_input_iterator<decltype(declval<C const&>().end()), T>;
#pragma endregion
}
#endif