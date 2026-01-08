#ifndef __RANGE_ACCESS
#define __RANGE_ACCESS
#include <bits/stl_iterator.hpp>
#include <initializer_list>
namespace std
{
	template<typename CT> constexpr inline auto begin(CT& __cont) -> decltype(__cont.begin()) { return __cont.begin(); }
	template<typename CT> constexpr inline auto begin(const CT& __cont) -> decltype(__cont.begin()) { return __cont.begin(); }
	template<typename CT> constexpr inline auto end(CT& __cont) -> decltype(__cont.end()) { return __cont.end(); }
	template<typename CT> constexpr inline auto end(const CT& __cont) -> decltype(__cont.end()) { return __cont.end(); }
	template<typename T, size_t N> constexpr inline T* begin(T (&__arr)[N]) noexcept { return __arr; }
	template<typename T, size_t N> constexpr inline T* end(T (&__arr)[N]) noexcept { return __arr + N; }
	template<typename T> class valarray;
	template<typename T> T* begin(valarray<T>&) noexcept;
	template<typename T> const T* begin(const valarray<T>&) noexcept;
	template<typename T> T* end(valarray<T>&) noexcept;
	template<typename T> const T* end(const valarray<T>&) noexcept;
	template<typename CT> constexpr inline auto cbegin(const CT& __cont) noexcept(noexcept(begin(__cont))) -> decltype(begin(__cont)) { return begin(__cont); }
	template<typename CT> constexpr inline auto cend(const CT& __cont) noexcept(noexcept(end(__cont))) -> decltype(end(__cont)) { return end(__cont); }
	template<typename CT> constexpr inline auto rbegin(CT& __cont) -> decltype(__cont.rbegin()) { return __cont.rbegin(); }
	template<typename CT> constexpr inline auto rbegin(const CT& __cont) -> decltype(__cont.rbegin()) { return __cont.rbegin(); }
	template<typename CT> constexpr inline auto rend(CT& __cont) -> decltype(__cont.rend()) { return __cont.rend(); }
	template<typename CT> constexpr inline auto rend(const CT& __cont) -> decltype(__cont.rend()) { return __cont.rend(); }
	template<typename T, size_t N> constexpr inline reverse_iterator<T*> rbegin(T (&__arr)[N]) noexcept { return reverse_iterator<T*>(__arr + N); }
	template<typename T, size_t N> constexpr inline reverse_iterator<T*> rend(T (&__arr)[N]) noexcept { return reverse_iterator<T*>(__arr); }
	template<typename T> constexpr inline reverse_iterator<const T*> rbegin(initializer_list<T> __il) noexcept { return reverse_iterator<const T*>(__il.end()); }
	template<typename T> constexpr inline reverse_iterator<const T*> rend(initializer_list<T> __il) noexcept { return reverse_iterator<const T*>(__il.begin()); }
	template<typename CT> constexpr inline auto crbegin(const CT& __cont) -> decltype(rbegin(__cont)) { return rbegin(__cont); }
	template<typename CT> constexpr inline auto crend(const CT& __cont) -> decltype(rend(__cont)) { return rend(__cont); }
	template<typename CT> constexpr auto size(const CT& __cont) noexcept(noexcept(__cont.size())) -> decltype(__cont.size()) { return __cont.size(); }
	template<typename T, size_t N> constexpr size_t size(const T (&)[N]) noexcept { return N; }
	template<typename CT> [[nodiscard]] constexpr auto empty(const CT& __cont) noexcept(noexcept(__cont.empty())) -> decltype(__cont.empty()) { return __cont.empty(); }
	template<typename T, size_t N> [[nodiscard]] constexpr bool empty(const T (&)[N]) noexcept { return false; }
	template<typename T> [[nodiscard]] constexpr bool empty(initializer_list<T> __il) noexcept { return __il.size() == 0;}
	template<typename CT> constexpr auto data(CT& __cont) noexcept(noexcept(__cont.data())) -> decltype(__cont.data()) { return __cont.data(); }
	template<typename CT> constexpr auto data(const CT& __cont) noexcept(noexcept(__cont.data())) -> decltype(__cont.data()) { return __cont.data(); }
	template<typename T, size_t N> constexpr T* data(T (&__array)[N]) noexcept { return __array; }
	template<typename T> constexpr const T* data(initializer_list<T> __il) noexcept { return __il.begin(); }
	template<typename CT> constexpr auto ssize(const CT& __cont)  noexcept(noexcept(__cont.size())) -> common_type_t<ptrdiff_t, make_signed_t<decltype(__cont.size())>> { using type = make_signed_t<decltype(__cont.size())>; return static_cast<common_type_t<ptrdiff_t, type>>(__cont.size()); }
	template<typename T, ptrdiff_t N> constexpr ptrdiff_t ssize(const T (&)[N]) noexcept { return N; }
	namespace ranges
	{
		template<typename> constexpr inline bool disable_sized_range	= false;
		template<typename> constexpr inline bool enable_borrowed_range	= false;
		namespace __detail { template<typename T> concept __maybe_borrowed_range	= ranges::enable_borrowed_range<remove_cvref_t<T>> || is_lvalue_reference_v<T>; }
		namespace __access
		{
			using std::__detail::__class_or_enum;
			using std::__detail::__range_iter_t;
			void begin()	= delete;
			void end()		= delete;
			void rbegin()	= delete;
			void rend()		= delete;
			void size()		= delete;
			template<typename T> concept __member_begin	= requires(T& __t) { { __decay_copy(__t.begin()) } -> input_or_output_iterator; };
			template<typename T> concept __adl_begin	= __class_or_enum<remove_reference_t<T>> && requires(T& __t) { { __decay_copy(begin(__t)) } -> input_or_output_iterator; };
			template<typename T> concept __member_end	= requires(T& __t) { { __decay_copy(__t.end()) } -> sentinel_for<__range_iter_t<T>>; };
			template<typename T> concept __adl_end		= __class_or_enum<remove_reference_t<T>> && requires(T& __t)  { { __decay_copy(end(__t)) } -> sentinel_for<__range_iter_t<T>>; };
			template<typename T> concept __valid_begin	= (__member_begin<T> || __adl_begin<T> || (is_array_v<remove_reference_t<T>> && is_lvalue_reference_v<T>)) && __detail::__maybe_borrowed_range<T>;
			template<typename T> concept __valid_end	= (__member_end<T> || __adl_end<T> || (is_bounded_array_v<remove_reference_t<T>> && is_lvalue_reference_v<T>)) && __detail::__maybe_borrowed_range<T>;	
			struct __begin
			{
				template<typename T>
				consteval static bool __noexcept() noexcept
				{
					if constexpr(is_array_v<remove_reference_t<T>>) return true;
					else if constexpr(__member_begin<T>) return noexcept(__decay_copy(declval<T&>().begin()));
					else return noexcept(__decay_copy(begin(declval<T&>())));
				}
			public:
				template<__valid_begin T>
				constexpr auto operator()(T&& t) const noexcept(__noexcept<T>())
				{
					if constexpr(is_array_v<remove_reference_t<T>>) return t + 0Z;
					else if constexpr(__member_begin<T>) return t.begin();
					else return begin(t);
				}
			};
			struct __end
			{
			private:
				template<typename T>
				consteval static bool __noexcept() noexcept
				{
					if constexpr(is_bounded_array_v<remove_reference_t<T>>) return true;
					else if constexpr(__member_end<T>) return noexcept(__decay_copy(declval<T&>().end()));
					else return noexcept(__decay_copy(end(declval<T&>())));
				}
			public:
				template<__valid_end T>
				constexpr auto operator()(T&& t) const noexcept(__noexcept<T>())
				{
					if constexpr(is_bounded_array_v<remove_reference_t<T>>)	return t + extent_v<remove_reference_t<T>>;
					else if constexpr(__member_end<T>) return t.end();
					else return end(t);
				}
			};
			template<typename T> concept __member_rbegin	= requires(T& __t) { { __decay_copy(__t.rbegin()) } -> input_or_output_iterator; };
			template<typename T> concept __adl_rbegin		= __class_or_enum<remove_reference_t<T>> && requires(T& __t) { { __decay_copy(rbegin(__t)) } -> input_or_output_iterator; };
			template<typename T> concept __member_rend		= requires(T& __t) { { __decay_copy(__t.rend()) } -> sentinel_for<__range_iter_t<T>>; };
			template<typename T> concept __adl_rend			= __class_or_enum<remove_reference_t<T>> && requires(T& __t)  { { __decay_copy(rend(__t)) } -> sentinel_for<__range_iter_t<T>>; };
			template<typename T> concept __reversable		= requires(T& t) { { __begin{}(t) } -> bidirectional_iterator; { __end{}(t) } -> same_as<decltype(__begin{}(t))>; };
			template<typename T> concept __valid_rbegin		= (__member_rbegin<T> || __adl_rbegin<T> || __reversable<T>) && __detail::__maybe_borrowed_range<T>;
			template<typename T> concept __valid_rend		= (__member_rend<T> || __adl_rend<T> || __reversable<T>) && __detail::__maybe_borrowed_range<T>;
			struct __rbegin
			{
				template<typename T>
				consteval static bool __noexcept() noexcept
				{
					if constexpr(__member_rbegin<T>) return noexcept(__decay_copy(declval<T&>().rbegin()));
					else if constexpr(__adl_rbegin<T>) return noexcept(__decay_copy(rbegin(declval<T&>())));
					else if constexpr(noexcept(__end{}(__decay_copy(declval<T&>())))) {
						typedef decltype(__end{}(__decay_copy(declval<T&>()))) __fwd;
						return noexcept(make_reverse_iterator(declval<__fwd>()));
					}
				}
			public:
				template<__valid_rbegin T>
				constexpr auto operator()(T&& t) const noexcept(__noexcept<T>())
				{
					if constexpr(__member_rbegin<T>) return t.rbegin();
					else if constexpr(__adl_rbegin<T>) return rbegin(t);
					else return make_reverse_iterator(__end{}(t));
				}
			};
			struct __rend
			{
			private:
				template<typename T>
				consteval static bool __noexcept() noexcept
				{
					if constexpr(__member_rend<T>) return noexcept(__decay_copy(declval<T&>().rend()));
					else if constexpr(__adl_rend<T>) return noexcept(__decay_copy(rend(declval<T&>())));
					else if constexpr(noexcept(__begin{}(__decay_copy(declval<T&>())))) {
						typedef decltype(__begin{}(__decay_copy(declval<T&>()))) __fwd;
						return noexcept(make_reverse_iterator(declval<__fwd>()));
					}
				}
			public:
				template<__valid_rend T>
				constexpr auto operator()(T&& t) const noexcept(__noexcept<T>())
				{
					if constexpr(__member_rend<T>) return t.rend();
					else if constexpr(__adl_rend<T>) return rend(t);
					else return make_reverse_iterator(__begin{}(t));
				}
			};
			template<typename T>
			concept __sentinel_size = requires(T& __t)
			{
				requires (!is_unbounded_array_v<remove_reference_t<T>>);
				{ __begin{}(__t) } -> forward_iterator;
				{ __end{}(__t) } -> sized_sentinel_for<decltype(__begin{}(__t))>;
				{ __end{}(__t) - __begin{}(__t) } -> integral;
			};
			template<typename T> concept __member_size = requires(T& __t) { { __decay_copy(__t.size()) } -> __detail::__is_integer_like; };
			template<typename T> concept __adl_size = requires(T& __t) { { __decay_copy(size(__t)) } -> __detail::__is_integer_like; };
			template<typename T> concept __valid_size = is_bounded_array_v<remove_reference_t<T>> || __member_size<T> || __adl_size<T> || __sentinel_size<T>;
			struct __size
			{
			private:
				template<typename T>
				consteval static bool __noexcept() noexcept
				{
					if constexpr(is_bounded_array_v<remove_reference_t<T>>) return true;
					else if constexpr(__member_size<T>) return noexcept(__decay_copy(declval<T&>().size()));
					else if constexpr(__adl_size<T>) return noexcept(__decay_copy(size(declval<T&>())));
					else return noexcept(__end{}(declval<T&>()) - __begin{}(declval<T&>()));
				}
			public:
				template<__valid_size T>
				constexpr auto operator()(T&& t) const noexcept(__noexcept<T>())
				{
					if constexpr(is_bounded_array_v<remove_reference_t<T>>) return extent_v<remove_reference_t<T>>;
					else if constexpr(__member_size<T>) return t.size();
					else if constexpr(__adl_size<T>) return size(t);
					else
					{
						typedef decltype(__end{}(t) - __begin{}(t)) diff;
						typedef conditional_t<is_integral_v<diff>, make_unsigned_t<diff>, diff> udiff;
						return static_cast<udiff>(__end{}(t) - __begin{}(t));
					}
				}
			};
			struct __ssize
			{
				template<__valid_size T> constexpr auto operator()(T&& t) const noexcept(noexcept(__size{}(declval<T&>())))
				{
					auto sz = __size{}(t);
					typedef decltype(sz) uresult;
					typedef conditional_t<is_integral_v<uresult>, make_signed_t<uresult>, uresult> sresult;
					if constexpr(is_integral_v<uresult> && sizeof(sresult) < sizeof(ptrdiff_t)) return static_cast<ptrdiff_t>(sz);
					else return static_cast<sresult>(sz);
				}
			};
			template<typename T> concept __member_empty = requires(T& __t) { bool(__t.empty()); };
			template<typename T> concept __size0_empty = requires(T& __t) { __size{}(__t) == 0; };
			template<typename T>
			concept __eq_iter_empty = requires(T& __t)
			{
				requires (!is_unbounded_array_v<remove_reference_t<T>>);
				{ __begin{}(__t) } -> forward_iterator;
				bool(__begin{}(__t) == __end{}(__t));
			};
			template<typename T> concept __valid_empty = __member_empty<T> || __size0_empty<T> || __eq_iter_empty<T>;
			struct __empty
			{
			private:
				template<typename T>
				consteval static bool __noexcept() noexcept
				{
					if constexpr(__member_empty<T>) return noexcept(bool(declval<T&>().empty()));
					else if constexpr(__size0_empty<T>) return noexcept(__size{}(declval<T&>()) == 0);
					else return noexcept(bool(__begin{}(declval<T&>()) == __end{}(declval<T&>())));
				}
			public:
				template<typename T>
				constexpr bool operator()(T&& t) const noexcept(__noexcept<T>())
				{
					if constexpr(__member_empty<T>) return bool(t.empty());
					else if constexpr(__size0_empty<T>) return __size{}(t) == 0;
					else return bool(__begin{}(t) == __end{}(t));
				}
			};
			template<typename T> concept __begin_data = contiguous_iterator<__range_iter_t<T>>;
			template<typename T> concept __pointer_to_object = is_pointer_v<T> && is_object_v<remove_pointer_t<T>>;
			template<typename T> concept __member_data = requires(T& __t) { { __decay_copy(__t.data()) } -> __pointer_to_object; };
			template<typename T> concept __valid_data = (__member_data<T> || __begin_data<T>) && __detail::__maybe_borrowed_range<T>;
			struct __data
			{
			private:
				template<typename T>
				consteval static bool __noexcept() noexcept {
					if constexpr(__member_data<T>) return noexcept(__decay_copy(declval<T&>().data()));
					else return noexcept(__begin{}(declval<T&>()));
				}
			public:
				template<__valid_data T> constexpr auto operator()(T&& t) const noexcept(__noexcept<T>()) {
					if constexpr(__member_data<T>) return t.data();
					else return to_address(__begin{}(t));
				}
			};
		}
		inline namespace __access_ftors
		{
			constexpr inline std::ranges::__access::__begin		begin{};
			constexpr inline std::ranges::__access::__end		end{};
			constexpr inline std::ranges::__access::__size		size{};
			constexpr inline std::ranges::__access::__ssize		ssize{};
			constexpr inline std::ranges::__access::__data		data{};
			constexpr inline std::ranges::__access::__rbegin	rbegin{};
			constexpr inline std::ranges::__access::__rend		rend{};
			constexpr inline std::ranges::__access::__empty		empty{};
		};
		template<typename T> concept range					= requires(T& t) { ranges::begin(t); ranges::end(t); };
		template<typename T> concept sized_range			= range<T> && requires(T& t) { ranges::size(t); };
		template<typename T> concept borrowed_range			= range<T> && __detail::__maybe_borrowed_range<T>;
		template<range RT> using iterator_t					= std::__detail::__range_iter_t<RT>;
		template<range RT> using sentinel_t					= decltype(ranges::end(declval<RT&>()));
		template<range RT> using range_difference_t			= iter_difference_t<iterator_t<RT>>;
		template<range RT> using range_value_t				= iter_value_t<iterator_t<RT>>;
		template<range RT> using range_reference_t			= iter_reference_t<iterator_t<RT>>;
		template<range RT> using range_rvalue_reference_t	= iter_rvalue_reference_t<iterator_t<RT>>;
		template<sized_range RT> using range_size_t			= decltype(ranges::size(declval<RT&>()));
		struct view_base {};
		namespace __detail { template<typename T> concept __valid_view_interface_arg	= is_class_v<T> && same_as<T, remove_cv_t<T>>; }
		template<__detail::__valid_view_interface_arg DT> class view_interface;
		namespace __detail
		{
			template<typename T, typename U> requires(!same_as<T, view_interface<U>>) void __is_derived_from_view_interface_fn(T const&, view_interface<U> const&); // not defined
			template<typename T> concept __derived_from_view_interface = requires(T t) { __is_derived_from_view_interface_fn(t, t); };
		}
		template<typename T> inline constexpr bool enable_view	= derived_from<T, view_base> || __detail::__derived_from_view_interface<T>;
		template<typename T> concept view						= range<T> && movable<T> && enable_view<T>;
		template<typename RT, typename T> concept output_range	= range<RT> && output_iterator<iterator_t<RT>, T>;
		template<typename T> concept input_range				= range<T> && input_iterator<iterator_t<T>>;
		template<typename T> concept forward_range				= input_range<T> && forward_iterator<iterator_t<T>>;
		template<typename T> concept bidirectional_range		= forward_range<T> && bidirectional_iterator<iterator_t<T>>;
		template<typename T> concept random_access_range		= bidirectional_range<T> && random_access_iterator<iterator_t<T>>;
		template<typename T> concept contiguous_range			= random_access_range<T> && contiguous_iterator<iterator_t<T>> && requires(T& t) { { ranges::data(t) } -> same_as<add_pointer_t<range_reference_t<T>>>; };
		template<typename T> concept common_range				= range<T> && same_as<iterator_t<T>, sentinel_t<T>>;
		template<typename T> concept constant_range				= input_range<T> && std::__detail::__constant_iterator<iterator_t<T>>;
		namespace __access
		{
			template<input_range RT>
			constexpr auto& __possibly_const_range(RT& r) noexcept
			{
				if constexpr(!input_range<RT const>) return r;
				return const_cast<RT const&>(r);
			}
			template<typename T, typename U> requires(is_same_v<T&, U&>)
			constexpr decltype(auto) __as_const(U& u)
			{
				if constexpr(is_lvalue_reference_v<T>)
					return const_cast<U const&>(u);
				else return static_cast<const U&&>(u);
			}
			template<typename T> concept __valid_cbegin = __detail::__maybe_borrowed_range<T> && requires(T&& t) { make_const_iterator(ranges::begin(__possibly_const_range(t))); };
			template<typename T> concept __valid_cend	= __detail::__maybe_borrowed_range<T> && requires(T&& t) { make_const_sentinel(ranges::end(__possibly_const_range(t))); };
			template<typename T> concept __valid_crbegin = __detail::__maybe_borrowed_range<T> && requires(T&& t) { make_const_iterator(ranges::rbegin(__possibly_const_range(t))); };
			template<typename T> concept __valid_crend	= __detail::__maybe_borrowed_range<T> && requires(T&& t) { make_const_sentinel(ranges::rend(__possibly_const_range(t))); };
			struct __cbegin
			{
				template<__valid_cbegin T>
				constexpr auto operator()(T&& t) const
				noexcept(noexcept(make_const_iterator(ranges::begin(__possibly_const_range(t))))) {
					auto& r = __possibly_const_range(t);
					return const_iterator<decltype(ranges::begin(r))>(ranges::begin(r));
				}
			};
			struct __cend
			{
				template<__valid_cend T>
				constexpr auto operator()(T&& t) const
				noexcept(noexcept(make_const_sentinel(ranges::end(__possibly_const_range(t))))) {
					auto& r = __possibly_const_range(t);
					return const_iterator<decltype(ranges::end(r))>(ranges::end(r));
				}
			};
			struct __crbegin
			{
				template<__valid_crbegin T>
				constexpr auto operator()(T&& t) const
				noexcept(noexcept(make_const_iterator(ranges::rbegin(__possibly_const_range(t))))) {
					auto& r = __possibly_const_range(t);
					return const_iterator<decltype(ranges::rbegin(r))>(ranges::rbegin(r));
				}
			};
			struct __crend
			{
				template<__valid_crend T>
				constexpr auto operator()(T&& t) const
				noexcept(noexcept(make_const_sentinel(ranges::rend(__possibly_const_range(t))))) {
					auto& r = __possibly_const_range(t);
					return const_iterator<decltype(ranges::rend(r))>(ranges::rend(r));
				}
			};
			template<typename T> concept __valid_cdata = __detail::__maybe_borrowed_range<T> && requires(T&& t) { ranges::data(__possibly_const_range(t)); };
			struct __cdata
			{
				template<__valid_cdata T>
				constexpr auto const* operator()(T&& t) const
				noexcept(noexcept(ranges::data(__possibly_const_range(t)))) { return ranges::data(__possibly_const_range(t)); }
			};
		}
		inline namespace __access_ftors
		{
			constexpr inline ranges::__access::__cbegin		cbegin{};
			constexpr inline ranges::__access::__cend		cend{};
			constexpr inline ranges::__access::__crbegin	crbegin{};
			constexpr inline ranges::__access::__crend		crend{};
		}
		namespace __detail
		{
			template<typename T> constexpr inline bool __is_initializer_list						= false;
			template<typename T> constexpr inline bool __is_initializer_list<initializer_list<T>>	= true;
			template<typename T> concept __simple_ctor_view	= view<remove_cvref_t<T>> && constructible_from<remove_cvref_t<T>, T>;
			template<typename T> concept __movable_non_view	= !view<remove_cvref_t<T>> && (is_lvalue_reference_v<T> || (movable<remove_reference_t<T>> && !__is_initializer_list<T>));
		}
		template<typename T> concept viewable_range	= range<T> && (__detail::__simple_ctor_view<T> || __detail::__movable_non_view<T>);
		struct __advance_fn final
		{
			template<input_or_output_iterator IT>
			constexpr void operator()(IT& i, iter_difference_t<IT> n) const
			{
				if constexpr(random_access_iterator<IT>) i += n;
				else if constexpr(bidirectional_iterator<IT>) {
					if(n > 0) do { ++i; } while(--n);
					else if(n < 0) do { --i; } while(++n);
				}
				else
				{
					__libk_assert(n >= 0);
					while(n-- > 0)
						++i;
				}
			}
			template<input_or_output_iterator IT, sentinel_for<IT> ST>
			constexpr void operator()(IT& i, ST bound) const
			{
				if constexpr(assignable_from<IT&, ST>)
					i	= move(bound);
				else if constexpr(sized_sentinel_for<ST, IT>)
					(*this)(i, bound - i);
				else while(i != bound)
					++i;
			}
			template<input_or_output_iterator IT, sentinel_for<IT> ST>
			constexpr iter_difference_t<IT> operator()(IT& i, iter_difference_t<IT> n, ST bound) const
			{
				if constexpr(sized_sentinel_for<ST, IT>)
				{
					iter_difference_t<IT> const diff = bound - i;
					if(diff == 0)
					{
						// inline any possible side effects of advance(it, bound)
						if constexpr(assignable_from<IT&, ST>)
							i = move(bound);
						else if constexpr(random_access_iterator<IT>)
							i += iter_difference_t<IT>(0);
						return n;
					}
					else if((diff > 0) ? (n > diff) : (n <= diff)) {
						(*this)(i, bound);
						return n - diff;
					}
					else if(n != 0) [[likely]]
					{
						__libk_assert(n < 0 == diff < 0);
						(*this)(i, n);
						return 0;
					}
					else
					{
						// inline any possible side effects of advance(it, n)
						if constexpr(random_access_iterator<IT>)
							i += iter_difference_t<IT>(0);
						return 0;
					}
				}
				else if(n == 0 || i == bound) return n;
				else if(n > 0)
				{
					iter_difference_t<IT> m = 0;
					do {
						++i;
						++m;
					} while(m != n && i != bound);
					return n - m;
				}
				else if constexpr(bidirectional_iterator<IT> && same_as<IT, ST>)
				{
					iter_difference_t<IT> m = 0;
					do {
						--i;
						--m;
					} while(m != n && i != bound);
					return n - m;
				}
				else { __libk_assert(n >= 0); return n; }
			}
			void operator&() = delete;
		};
		struct __distance_fn final
		{
			template<typename IT, sentinel_for<IT> ST>
			constexpr iter_difference_t<IT> operator()(IT&& first, ST last) const
			{
				if constexpr(sized_sentinel_for<ST, IT>) { return last - static_cast<decay_t<IT> const&>(first); }
				else
				{
					iter_difference_t<IT> result = 0;
					for(IT i = first; i != last; i++, result++);
					return result;
				}
			}
			template<range RT>
			constexpr range_difference_t<RT> operator()(RT&& r) const
			{
				if constexpr(sized_range<RT>)
					return static_cast<range_difference_t<RT>>(ranges::size(r));
				else return (*this)(ranges::begin(r), ranges::end(r));
			}
			void operator&() const = delete;
		};
		constexpr inline __advance_fn advance{};
		constexpr inline __distance_fn distance{};
		struct __next_fn final
		{
			template<input_or_output_iterator IT>
			constexpr IT operator()(IT i) const {
				++i;
				return i;
			}
			template<input_or_output_iterator IT>
			constexpr IT operator()(IT i, iter_difference_t<IT> n) const {
				ranges::advance(i, n);
				return i;
			}
			template<input_or_output_iterator IT, sentinel_for<IT> ST>
			constexpr IT operator()(IT i, ST bound) const {
				ranges::advance(i, bound);
				return i;
			}
			template<input_or_output_iterator IT, sentinel_for<IT> ST>
			constexpr IT operator()(IT i, iter_difference_t<IT> n, ST bound) const {
				ranges::advance(i, n, bound);
				return i;
			}
			void operator&() const = delete;
		};
		struct __prev_fn final
		{
			template<bidirectional_iterator IT>
			constexpr IT operator()(IT i) const {
				--i;
				return i;
			}
			template<bidirectional_iterator IT>
			constexpr IT operator()(IT i, iter_difference_t<IT> n) const {
				ranges::advance(i, -n);
				return i;
			}
			template<bidirectional_iterator IT>
			constexpr IT operator()(IT i, iter_difference_t<IT> n, IT bound) const {
				ranges::advance(i, -n, bound);
				return i;
			}
			void operator&() const = delete;
		};
		constexpr inline __next_fn next{};
		constexpr inline __prev_fn prev{};
		struct dangling {
			constexpr dangling() noexcept = default;
			template<typename ... Args> constexpr dangling(Args&& ...) noexcept {}
		};
		template<range RT> using borrowed_iterator_t = conditional_t<borrowed_range<RT>, iterator_t<RT>, dangling>;
		template<range RT, typename AT = std::allocator<uint8_t>>
		struct elements_of
		{
			[[no_unique_address]] RT range;
			[[no_unique_address]] AT allocator	= AT();
		};
		template<range RT, typename AT = std::allocator<uint8_t>> elements_of(RT&&, AT = AT()) -> elements_of<RT&&, AT>;
	}
}
#endif