#ifndef __STL_ITERATOR
#define __STL_ITERATOR
#include <bits/iterator_concepts.hpp>
#include <compare>
#include <libk_decls.h>
namespace std
{
	struct input_iterator_tag {};
	struct output_iterator_tag {};
	struct forward_iterator_tag : public input_iterator_tag {};
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};
	struct contiguous_iterator_tag : public random_access_iterator_tag {};
	namespace __detail
	{
		typedef tuple<input_iterator_tag, forward_iterator_tag, bidirectional_iterator_tag, random_access_iterator_tag, contiguous_iterator_tag> __iter_concept_tag_selector;
		template<input_iterator IT> constexpr auto __iter_concept_helper(IT) -> decltype(std::get<first_false_in<forward_iterator<IT>, bidirectional_iterator<IT>, random_access_iterator<IT>, contiguous_iterator<IT>>()>(std::declval<__iter_concept_tag_selector>()));
		template<input_iterator IT> using __iterator_concept	= decltype(__iter_concept_helper(declval<IT>()));
	}
	template<typename CT, typename T, typename DT = ptrdiff_t, typename PT = T*, typename RT = T&>
	struct iterator
	{
		typedef CT	iterator_category;
		typedef T	value_type;
		typedef DT	difference_type;
		typedef PT	pointer;
		typedef RT	reference;
	};
	template<typename IT> struct iterator_traits;
	template<typename IT> struct iterator_traits : public __iterator_traits<IT, __void_t<IT>> {};
	template<typename T>
	requires(is_object_v<T>)
	struct iterator_traits<T*>
	{
		using iterator_concept	= contiguous_iterator_tag;
		using iterator_category	= random_access_iterator_tag;
		using value_type		= remove_cv_t<T>;
		using difference_type	= ptrdiff_t;
		using pointer			= T*;
		using reference			= T&;
	};
	template<typename IT> constexpr inline typename iterator_traits<IT>::iterator_category __iterator_category(const IT&) { return typename iterator_traits<IT>::iterator_category(); }
	template<typename IT> using __iterator_category_t = typename iterator_traits<IT>::iterator_category;
	template<typename IT, typename CT = __iterator_category_t<IT>>
	struct __is_random_access_iter : is_base_of<random_access_iterator_tag, CT> { typedef is_base_of<random_access_iterator_tag, CT> __base; enum { __value = __base::value }; };
	template<std::input_iterator IIT> constexpr inline typename iterator_traits<IIT>::difference_type __distance(IIT __first, IIT __last, input_iterator_tag) { typename iterator_traits<IIT>::difference_type __n = 0; while (__first != __last) { ++__first; ++__n; } return __n; }
	template<std::random_access_iterator RIT> constexpr inline typename iterator_traits<RIT>::difference_type __distance(RIT __first, RIT __last, random_access_iterator_tag) { return __last - __first; }
	template<std::input_iterator IIT> constexpr inline typename iterator_traits<IIT>::difference_type distance(IIT __first, IIT __last) { return std::__distance(__first, __last, std::__iterator_category(__first));}
	template<std::input_iterator IIT, typename DT> constexpr inline void __advance(IIT& __i, DT __n, input_iterator_tag) { while (__n--) ++__i; }
	template<std::bidirectional_iterator BIT, typename DT> constexpr inline void __advance(BIT& __i, DT __n, bidirectional_iterator_tag) { if (__n > 0) while (__n--) ++__i; else while (__n++) --__i; }
	template<std::random_access_iterator RIT, typename DT> constexpr inline void __advance(RIT& __i, DT __n, random_access_iterator_tag) { if (__builtin_constant_p(__n) && __n == 1) ++__i; else if (__builtin_constant_p(__n) && __n == -1) --__i; else __i += __n; }
	template<std::input_iterator IIT, typename DT> constexpr inline void advance(IIT& __i, DT __n) { typename iterator_traits<IIT>::difference_type __d = __n; std::__advance(__i, __d, std::__iterator_category(__i)); }
	template<std::input_iterator IIT> constexpr inline IIT next(IIT __x, typename iterator_traits<IIT>::difference_type __n = 1) { std::advance(__x, __n); return __x; }
	template<std::bidirectional_iterator BIT> constexpr inline BIT prev(BIT __x, typename iterator_traits<BIT>::difference_type __n = 1)  { std::advance(__x, -__n); return __x; }
	template<typename CT, typename LT, typename OT = CT> using __clamp_iter_cat = conditional_t<derived_from<CT, LT>, LT, OT>;
	template<typename BIT>
	class reverse_iterator
	{
	protected:
		BIT current{};
		typedef std::iterator_traits<BIT> __traits;
	public:
		typedef BIT iterator_type;
		typedef __traits::iterator_category		iterator_category;
		typedef __traits::value_type			value_type;
		typedef __traits::difference_type		difference_type;
		typedef __traits::pointer				pointer;
		typedef __traits::reference				reference;
		reverse_iterator() = default;
		constexpr explicit reverse_iterator(BIT __it) : current(__it) {}
		template<typename BJT> requires (!same_as<BIT, BJT> && std::convertible_to<BJT const&, BIT>)
		constexpr reverse_iterator(BJT const& __it) : current(__it) {}
		constexpr reference operator*() const { return *std::prev(current); }
		constexpr pointer operator->() const requires (std::is_pointer_v<BIT> || requires(const BIT i) { i.operator->(); }) { if constexpr(std::is_pointer_v<BIT>) return current - 1; else return std::prev(current).operator->(); }
		constexpr iterator_type base() const { return current; }
		constexpr reference operator[](difference_type __n) { return current[ -__n - 1]; }
		constexpr reverse_iterator& operator++() { --current; return *this; }
		constexpr reverse_iterator& operator--() { ++current; return *this; }
		constexpr reverse_iterator operator++(int) { return reverse_iterator(current--); }
		constexpr reverse_iterator operator--(int) { return reverse_iterator(current++); }
		constexpr reverse_iterator operator+(difference_type __n) const { return reverse_iterator(current - __n); }
		constexpr reverse_iterator operator-(difference_type __n) const { return reverse_iterator(current + __n); }
		constexpr reverse_iterator& operator+=(difference_type __n) { current -= __n; return *this; }
		constexpr reverse_iterator& operator-=(difference_type __n) { current += __n; return *this; }
	};
	template<typename IT, typename JT> constexpr bool operator==(reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) requires requires {  {__this.base() == __that.base()} -> __detail::__boolean_testable;  } { return __this.base() == __that.base(); }
	template<typename IT, typename JT> constexpr bool operator!=(reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) requires requires {  {__this.base() != __that.base()} -> __detail::__boolean_testable;  } { return __this.base() != __that.base(); }
	template<typename IT, typename JT> constexpr bool operator <(reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) requires requires {  {__this.base() > __that.base()} -> __detail::__boolean_testable;  } { return __this.base()  > __that.base(); }
	template<typename IT, typename JT> constexpr bool operator >(reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) requires requires {  {__this.base() < __that.base()} -> __detail::__boolean_testable;  } { return __this.base()  < __that.base(); }
	template<typename IT, typename JT> constexpr bool operator<=(reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) requires requires {  {__this.base() >= __that.base()} -> __detail::__boolean_testable;  } { return __this.base() >= __that.base(); }
	template<typename IT, typename JT> constexpr bool operator>=(reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) requires requires {  {__this.base() <= __that.base()} -> __detail::__boolean_testable;  } { return __this.base() <= __that.base(); }
	template<typename IT, typename JT> requires(three_way_comparable_with<JT, IT>) constexpr compare_three_way_result_t<IT, JT> operator<=>(reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) { return __that.base() <=> __this.base(); }
	template<typename IT, typename JT> constexpr typename iterator_traits<IT>::difference_type operator-(reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) { return __that.base() - __this.base(); }
	template<__detail::__dereferenceable IT> struct deref { typedef decltype(*declval<IT>()) type; };
	template<__detail::__dereferenceable IT> using deref_t			= typename deref<IT>::type;
	template<indirectly_readable IT> using iter_const_reference_t	= common_reference_t<const iter_value_t<IT>&&, iter_reference_t<IT>>;
	template<input_iterator IT> class basic_const_iterator;
	namespace __detail
	{
		template<typename IT> concept __constant_iterator						= input_iterator<IT> && same_as<iter_const_reference_t<IT>, iter_reference_t<IT>>;
		template<typename IT> constexpr inline bool __is_const_iterator			= __constant_iterator<IT>;
		template<typename IT> concept __not_const_iterator						= !__is_const_iterator<IT>;
		template<indirectly_readable IT> using __iter_const_rvalue_reference_t	= common_reference_t<const iter_value_t<IT>&&, iter_rvalue_reference_t<IT>>;
		template<typename> struct __basic_const_iterator_iter_cat{};
		template<forward_iterator IT> struct __basic_const_iterator_iter_cat<IT> { typedef typename iterator_traits<IT>::iterator_category iterator_category; };
	}
	template<input_iterator IT> using const_iterator = conditional_t<__detail::__constant_iterator<IT>, IT, basic_const_iterator<IT>>;
	namespace __detail
	{
		template<typename ST> struct __const_sentinel { typedef ST type; };
		template<input_iterator IT> struct __const_sentinel<IT> { typedef const_iterator<IT> type; };
	}
	template<semiregular ST> using const_sentinel	= typename __detail::__const_sentinel<ST>::type;
	template<input_iterator IT>
	class basic_const_iterator : public __detail::__basic_const_iterator_iter_cat<IT>
	{
		IT __current{};
		typedef iter_const_reference_t<IT> __reference;
		typedef __detail::__iter_const_rvalue_reference_t<IT> __rvalue_reference;
		template<input_iterator JT> friend class basic_const_iterator;
	public:
		typedef __detail::__iter_concept<IT> iterator_concept;
		typedef iter_value_t<IT> value_type;
		typedef iter_difference_t<IT> difference_type;
		constexpr basic_const_iterator() requires(default_initializable<IT>) = default;
		constexpr basic_const_iterator(IT i) noexcept(is_nothrow_move_constructible_v<IT>) : __current(move(i)) {}
		template<convertible_to<IT> JT> constexpr basic_const_iterator(basic_const_iterator<JT> that) noexcept(is_nothrow_move_constructible_v<IT>) : __current(move(that.__current)) {}
		template<not_self<IT> JT> requires(convertible_to<JT, IT>) constexpr basic_const_iterator(JT&& j) noexcept(is_nothrow_constructible_v<IT, JT>) : __current(forward<JT>(j)) {}
		constexpr IT const& base() const & noexcept { return __current; }
		constexpr IT base() && noexcept(is_nothrow_move_constructible_v<IT>) { return move(__current); }
		constexpr __reference operator*() const noexcept(noexcept(static_cast<__reference>(*__current))) { return static_cast<__reference>(*__current); }
		constexpr const auto* operator->() const noexcept(contiguous_iterator<IT> || noexcept(*__current)) requires is_lvalue_reference_v<iter_reference_t<IT>> && same_as<remove_cvref_t<iter_reference_t<IT>>, value_type>
		{
			if constexpr(contiguous_iterator<IT>)
			return to_address(__current);
			else return addressof(*__current);
		}
		constexpr basic_const_iterator& operator++() noexcept(noexcept(++__current)) {
			++__current;
			return *this;
		}
		constexpr void operator++(int) noexcept(noexcept(++__current)) { ++__current; }
		constexpr basic_const_iterator operator++(int)
		noexcept(noexcept(++*this) && is_nothrow_copy_constructible_v<basic_const_iterator>)
		requires(forward_iterator<IT>)
		{
			auto __tmp = *this;
			++*this;
			return __tmp;
		}
		constexpr basic_const_iterator& operator--()
		noexcept(noexcept(--__current))
		requires(bidirectional_iterator<IT>) {
			--__current;
			return *this;
		}
		constexpr basic_const_iterator operator--(int)
		noexcept(noexcept(--*this) && is_nothrow_copy_constructible_v<basic_const_iterator>)
		requires(bidirectional_iterator<IT>)
		{
			auto __tmp = *this;
			--*this;
			return __tmp;
		}
		constexpr basic_const_iterator& operator+=(difference_type n)
		noexcept(noexcept(__current += n))
		requires(random_access_iterator<IT>) {
			__current += n;
			return *this;
		}
		constexpr basic_const_iterator& operator-=(difference_type n)
		noexcept(noexcept(__current -= n))
		requires(random_access_iterator<IT>) {
			__current -= n;
			return *this;
		}
		constexpr __reference operator[](difference_type n) const
		noexcept(noexcept(static_cast<__reference>(__current[n]))) 
		requires(random_access_iterator<IT>) { return static_cast<__reference>(__current[n]); }
		template<sentinel_for<IT> ST>
		constexpr bool operator==(ST const& __s) const
		noexcept(noexcept(__current == __s)) { return __current == __s; }
		template<__detail::__not_const_iterator CIT> requires(__detail::__constant_iterator<CIT> && convertible_to<IT, CIT>)
		constexpr operator CIT() const& { return __current; }
		template<__detail::__not_const_iterator CIT> requires(__detail::__constant_iterator<CIT> && convertible_to<IT, CIT>)
		constexpr operator CIT() && { return move(__current); }
		constexpr bool operator<(basic_const_iterator const& that) const
		noexcept(noexcept(__current < that.__current))
		requires(random_access_iterator<IT>) { return this->__current < that.__current; }
		constexpr bool operator>(basic_const_iterator const& that) const
		noexcept(noexcept(__current > that.__current))
		requires(random_access_iterator<IT>) { return this->__current > that.__current; }
		constexpr bool operator<=(basic_const_iterator const& that) const
		noexcept(noexcept(__current <= that.__current))
		requires(random_access_iterator<IT>) { return this->__current <= that.__current; }
		constexpr bool operator>=(basic_const_iterator const& that) const
		noexcept(noexcept(__current >= that.__current))
		requires(random_access_iterator<IT>) { return this->__current >= that.__current; }
		constexpr auto operator<=>(basic_const_iterator const& that) const
		noexcept(noexcept(__current <=> that.__current))
		requires(random_access_iterator<IT> && three_way_comparable<IT>) { return this->__current <=> that.__current; }
		template<not_self<basic_const_iterator> IT2>
		constexpr bool operator<(IT2 const& that) const
		noexcept(noexcept(__current < that))
		requires(random_access_iterator<IT> && totally_ordered_with<IT, IT2>) { return this->__current < that; }
		template<not_self<basic_const_iterator> IT2>
		constexpr bool operator>(IT2 const& that) const
		noexcept(noexcept(__current > that))
		requires(random_access_iterator<IT> && totally_ordered_with<IT, IT2>) { return this->__current > that; }
		template<not_self<basic_const_iterator> IT2>
		constexpr bool operator<=(IT2 const& that) const
		noexcept(noexcept(__current <= that))
		requires(random_access_iterator<IT> && totally_ordered_with<IT, IT2>) { return this->__current <= that; }
		template<not_self<basic_const_iterator> IT2>
		constexpr bool operator>=(IT2 const& that) const
		noexcept(noexcept(__current >= that))
		requires(random_access_iterator<IT> && totally_ordered_with<IT, IT2>) { return this->__current >= that; }
		template<not_self<basic_const_iterator> IT2>
		constexpr auto operator<=>(IT2 const& that) const
		noexcept(noexcept(__current <=> that))
		requires(random_access_iterator<IT> && totally_ordered_with<IT, IT2> && three_way_comparable_with<IT, IT2>) { return this->__current <=> that; }
		template<__detail::__not_const_iterator IT2, same_as<IT> IT3>
		friend constexpr bool operator<(IT2 const& __this, basic_const_iterator<IT3> const& __that)
		noexcept(noexcept(__this < __that.__current))
		requires(random_access_iterator<IT> && totally_ordered_with<IT, IT2>) { return __this < __that.__current; }
		template<__detail::__not_const_iterator IT2, same_as<IT> IT3>
		friend constexpr bool operator>(IT2 const& __this, basic_const_iterator<IT3> const& __that)
		noexcept(noexcept(__this > __that.__current))
		requires(random_access_iterator<IT> && totally_ordered_with<IT, IT2>) { return __this > __that.__current; }
		template<__detail::__not_const_iterator IT2, same_as<IT> IT3>
		friend constexpr bool operator<=(IT2 const& __this, basic_const_iterator<IT3> const& __that)
		noexcept(noexcept(__this <= __that.__current))
		requires(random_access_iterator<IT> && totally_ordered_with<IT, IT2>) { return __this <= __that.__current; }
		template<__detail::__not_const_iterator IT2, same_as<IT> IT3>
		friend constexpr bool operator>=(IT2 const& __this, basic_const_iterator<IT3> const& __that)
		noexcept(noexcept(__this >= __that.__current))
		requires(random_access_iterator<IT> && totally_ordered_with<IT, IT2>) { return __this >= __that.__current; }
		friend constexpr basic_const_iterator operator+(basic_const_iterator const& that, difference_type n)
		noexcept(noexcept(basic_const_iterator(that.__current + n)))
		requires(random_access_iterator<IT>) { return basic_const_iterator(that.__current + n); }
		friend constexpr basic_const_iterator operator+(difference_type n, basic_const_iterator const& that)
		noexcept(noexcept(basic_const_iterator(that.__current + n)))
		requires(random_access_iterator<IT>) { return basic_const_iterator(that.__current + n); }
		friend constexpr basic_const_iterator operator-(basic_const_iterator const& that, difference_type n)
		noexcept(noexcept(basic_const_iterator(that.__current - n)))
		requires(random_access_iterator<IT>) { return basic_const_iterator(that.__current - n); }
		template<sized_sentinel_for<IT> ST>
		constexpr difference_type operator-(ST const& __that) const
		noexcept(noexcept(__current - __that)) { return this->__current - __that; }
		template<__detail::__not_const_iterator ST, same_as<IT> IT2> requires(sized_sentinel_for<ST, IT>)
		friend constexpr difference_type operator-(ST const& __this, const basic_const_iterator<IT2>& __that)
		noexcept(noexcept(__this - __that.__current)) { return __this - __that.__current; }
		friend constexpr __rvalue_reference iter_move(basic_const_iterator const& that)
		noexcept(noexcept(static_cast<__rvalue_reference>(ranges::iter_move(that.__current)))) { return static_cast<__rvalue_reference>(ranges::iter_move(that.__current)); }
	};
	template<typename T, common_with<T> U> requires(input_iterator<common_type_t<T, U>>) struct common_type<basic_const_iterator<T>, U> { typedef basic_const_iterator<common_type_t<T, U>> type; };
	template<typename T, common_with<T> U> requires(input_iterator<common_type_t<T, U>>) struct common_type<U, basic_const_iterator<T>> { typedef basic_const_iterator<common_type_t<T, U>> type; };
	template<typename T, common_with<T> U> requires(input_iterator<common_type_t<T, U>>) struct common_type<basic_const_iterator<T>, basic_const_iterator<U>> { typedef basic_const_iterator<common_type_t<T, U>> type; };
	template<input_iterator IT> constexpr const_iterator<IT> make_const_iterator(IT i) noexcept(is_nothrow_convertible_v<IT, const_iterator<IT>>) { return i; }
	template<semiregular ST> constexpr const_sentinel<ST> make_const_sentinel(ST s) noexcept(is_nothrow_convertible_v<ST, const_sentinel<ST>>) { return s; }
}
namespace __impl
{
	template<typename IT, typename CT>
	struct __iterator 
	{
	protected:
		typedef std::iterator_traits<IT> __traits;
		IT current;
	public:
		typedef IT								iterator_type;
		typedef __traits::iterator_category		iterator_category;
		typedef __traits::value_type			value_type;
		typedef __traits::difference_type		difference_type;
		typedef __traits::pointer				pointer;
		typedef __traits::reference				reference;
		using iterator_concept	= std::__detail::__iter_concept<IT>;
		constexpr IT const& base() const noexcept { return current; }
		constexpr __iterator() noexcept : current(IT()) {}
		constexpr explicit __iterator(IT const& __i) noexcept : current(__i) {}
		template<typename JT> requires(std::is_convertible_v<JT, IT>) constexpr __iterator(__iterator<JT, CT> const& that) noexcept : current(that.base()) {}
		constexpr reference operator*() const noexcept { return *current; }
		constexpr pointer operator->() const noexcept { return current; }
		constexpr reference operator[](difference_type __n) const noexcept { return current[__n]; }
		constexpr __iterator& operator++() noexcept { ++current; return *this; }
		constexpr __iterator operator++(int) noexcept { return __iterator(current++); }
		constexpr __iterator& operator--() noexcept { --current; return *this; }
		constexpr __iterator operator--(int) noexcept { return __iterator(current--); }
		constexpr __iterator& operator+=(difference_type __n) noexcept { current += __n; return *this; }
		constexpr __iterator& operator-=(difference_type __n) noexcept { current -= __n; return *this; }
		constexpr __iterator operator+(difference_type __n) const noexcept { return __iterator(current + __n); }
		constexpr __iterator operator-(difference_type __n) const noexcept { return __iterator(current - __n); }
	};
	template<typename IT, typename JT, typename CT> constexpr bool operator==(__iterator<IT, CT> const& __this, __iterator<JT, CT> const& __that) noexcept { return __this.base() == __that.base(); }
	template<typename IT, typename JT, typename CT> constexpr std::__detail::__synth3way_t<IT, JT> operator<=>(__iterator<IT, CT> const& __this, __iterator<JT, CT> const& __that) noexcept(noexcept(std::__detail::__synth3way(__this.base(), __that.base()))) { return std::__detail::__synth3way(__this.base(), __that.base()); }
	template<typename IT, typename JT, typename CT> constexpr typename __iterator<IT, CT>::difference_type operator-(__iterator<IT, CT> const& __this, __iterator<JT, CT> const& __that) noexcept { return __this.base() - __that.base(); }
	template<typename IT> using __deref_type = decltype(*std::declval<IT>());
	template<typename IT> concept __dereference_to_advance_capable = requires { { std::declval<IT>() + std::declval<__deref_type<IT>>() } -> std::convertible_to<IT>; };
	template<typename FT, typename IT> concept __deref_advance_transfer = std::is_default_constructible_v<FT> && requires(FT ft) { { ft(std::declval<IT>(), *std::declval<IT>(), std::declval<IT>()) } -> std::convertible_to<IT>; };
	template<__dereference_to_advance_capable IT> struct identity_deref { typedef decltype(*std::declval<IT>()) deref_type; constexpr IT operator()(IT start, deref_type what, IT end) noexcept { return(start + what < end) ? start + what : end; } };
	// Special iterator for containers where the value stored at an iterator points, either directly or via some modification, to location of the next element in some meaningful sequence (e.g. the FAT on an FATxx file system)
	extension template<__dereference_to_advance_capable IT, typename CT, __deref_advance_transfer<IT> AT = identity_deref<IT>>
	struct __dereference_to_advance_iterator
	{
	protected:
		typedef std::iterator_traits<IT> __traits;
		IT begin;
		IT current;
		IT end;
		AT __adv;
	public:
		typedef IT															iterator_type;
		typedef typename std::remove_reference<decltype(*current)>::type	deref_type;
		typedef __traits::iterator_category									iterator_category;
		typedef __traits::value_type										value_type;
		typedef decltype(std::declval<IT>() - std::declval<IT>())			difference_type;
		typedef __traits::pointer											pointer;
		typedef __traits::reference											reference;
		using iterator_concept = std::forward_iterator_tag;
		constexpr IT const& base() const noexcept { return current; }
		constexpr __dereference_to_advance_iterator() noexcept(noexcept(AT())) : begin(), current(), end(), __adv{} {}
		constexpr explicit __dereference_to_advance_iterator(IT b, IT c, IT e, AT a = AT()) noexcept(noexcept(AT())) : begin(b), current(c), end(e), __adv(a) {}
		template<std::same_as<typename CT::pointer> JT> constexpr explicit __dereference_to_advance_iterator(__dereference_to_advance_iterator<JT, CT, AT> const& that) noexcept : begin(that.begin), current(that.current), end(that.end), __adv(that.__adv) {}
		constexpr reference operator*() const noexcept { return *current; }
		constexpr pointer operator->() const noexcept { return current; }
		constexpr __dereference_to_advance_iterator& operator++() noexcept { if(current != end) { current = __adv(begin, *current, end); } return *this; }
		constexpr __dereference_to_advance_iterator operator++(int) noexcept { pointer old = current; if(current != end) { current = __adv(begin, *current, end); } return __dereference_to_advance_iterator(begin, old, end); }
		constexpr __dereference_to_advance_iterator next() const noexcept { return __dereference_to_advance_iterator{ begin, __adv(begin, *current, end), end }; }
		constexpr bool __is_equal(__dereference_to_advance_iterator const& that) const noexcept { return this->begin == that.begin && this->current == that.current && this->end == that.end; }
		constexpr deref_type offs() const noexcept requires requires{ static_cast<deref_type>(std::declval<difference_type>()); } { return static_cast<deref_type>(current - begin); }
	};
	extension template<__dereference_to_advance_capable IT, __dereference_to_advance_capable JT, typename CT, __deref_advance_transfer<IT> AT, __deref_advance_transfer<JT> BT>
	constexpr std::__detail::__synth3way_t<IT, JT> operator<=>(__dereference_to_advance_iterator<IT, CT, AT> const& __this, __dereference_to_advance_iterator<JT, CT, BT> const& __that) noexcept(noexcept(std::__detail::__synth3way(__this.base(), __that.base()))) { return std::__detail::__synth3way(__this.base(), __that.base()); }
	extension template<__dereference_to_advance_capable IT, __dereference_to_advance_capable JT, typename CT, __deref_advance_transfer<IT> AT, __deref_advance_transfer<JT> BT>
	constexpr bool operator==(__dereference_to_advance_iterator<IT, CT, AT> const& __this, __dereference_to_advance_iterator<JT, CT, BT> const& __that) noexcept { return __this.__is_equal(__that); }
}
#endif
