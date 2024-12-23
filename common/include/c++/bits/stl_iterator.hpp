#ifndef __STL_ITERATOR
#define __STL_ITERATOR
#include "bits/iterator_concepts.hpp"
namespace std
{
	struct input_iterator_tag {};
	struct output_iterator_tag {};
	struct forward_iterator_tag : public input_iterator_tag {};
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};
	struct contiguous_iterator_tag : public random_access_iterator_tag {};
	template<typename CT, typename T, typename DT = ptrdiff_t, typename PT = T*, typename RT = T&>
	struct iterator
	{
		typedef CT  iterator_category;
		typedef T   value_type;
		typedef DT  difference_type;
		typedef PT  pointer;
		typedef RT  reference;
	};
	template<typename IT> struct iterator_traits;
	template<typename IT, typename = __void_t<>> struct __iterator_traits {};
	template<typename IT> struct iterator_traits : public __iterator_traits<IT> {};
	template<typename T>
	requires is_object_v<T>
	struct iterator_traits<T*>
	{
		using iterator_concept  = contiguous_iterator_tag;
		using iterator_category = random_access_iterator_tag;
		using value_type	    = remove_cv_t<T>;
		using difference_type   = ptrdiff_t;
		using pointer	        = T*;
		using reference	        = T&;
	};
	template<typename IT> constexpr inline typename iterator_traits<IT>::iterator_category __iterator_category(const IT&) { return typename iterator_traits<IT>::iterator_category(); }
	template<typename IT> using __iterator_category_t = typename iterator_traits<IT>::iterator_category;
	template<typename IT, typename CT = __iterator_category_t<IT>>
	struct __is_random_access_iter : is_base_of<random_access_iterator_tag, CT> { typedef is_base_of<random_access_iterator_tag, CT> _Base; enum { __value = _Base::value }; };
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
    template<bidirectional_iterator BIT> class reverse_iterator
    {
    protected:
        BIT current{};
        typedef std::iterator_traits<BIT> __traits;
    public:
        typedef BIT iterator_type;
        typedef __traits::iterator_category     iterator_category;
		typedef __traits::value_type            value_type;
		typedef __traits::difference_type       difference_type;
		typedef __traits::pointer               pointer;
		typedef __traits::reference             reference;
        reverse_iterator() = default;
        constexpr explicit reverse_iterator(BIT __it) : current{__it} {}
        template<typename BJT> requires (!same_as<BIT, BJT> && std::convertible_to<BJT const&, BIT>)
        constexpr reverse_iterator(BJT const& __it) : current{ __it } {}
        constexpr reference operator*() const { return *std::prev(current); }
        constexpr pointer operator->() const requires (std::is_pointer_v<BIT> || requires(const BIT i) { i.operator->(); }) 
        {
            if constexpr(std::is_pointer_v<BIT>) return current - 1;
            else return std::prev(current).operator->(); 
        }
        constexpr iterator_type base() const { return current; }
        constexpr reference operator[](difference_type __n) { return current[ -__n - 1]; }
        constexpr reverse_iterator& operator++() { --current; return *this; }
        constexpr reverse_iterator& operator--() { ++current; return *this; }
        constexpr reverse_iterator operator++(int) { return reverse_iterator { current-- }; }
        constexpr reverse_iterator operator--(int) { return reverse_iterator { current++ }; }
        constexpr reverse_iterator operator+(difference_type __n) const { return reverse_iterator { current - __n }; }
        constexpr reverse_iterator operator-(difference_type __n) const { return reverse_iterator { current + __n }; }
        constexpr reverse_iterator& operator+=(difference_type __n) { current -= __n; return *this; }
        constexpr reverse_iterator& operator-=(difference_type __n) { current += __n; return *this; }
    };
    template<bidirectional_iterator IT, bidirectional_iterator JT> 
    requires totally_ordered_with<IT, JT>
    constexpr bool operator == (reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) { return __this.base() == __that.base(); }
    template<bidirectional_iterator IT, bidirectional_iterator JT> 
    requires totally_ordered_with<IT, JT>
    constexpr bool operator != (reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) { return __this.base() != __that.base(); }
    template<bidirectional_iterator IT, bidirectional_iterator JT> 
    requires totally_ordered_with<IT, JT>
    constexpr bool operator  < (reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) { return __this.base()  > __that.base(); }
    template<bidirectional_iterator IT, bidirectional_iterator JT> 
    requires totally_ordered_with<IT, JT>
    constexpr bool operator  > (reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) { return __this.base()  < __that.base(); }
    template<bidirectional_iterator IT, bidirectional_iterator JT> 
    requires totally_ordered_with<IT, JT>
    constexpr bool operator <= (reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) { return __this.base() >= __that.base(); }
    template<bidirectional_iterator IT, bidirectional_iterator JT> 
    requires totally_ordered_with<IT, JT>
    constexpr bool operator >= (reverse_iterator<IT> const& __this, reverse_iterator<JT> const& __that) { return __this.base() <= __that.base(); }
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
        typedef IT                              iterator_type;
        typedef __traits::iterator_category     iterator_category;
		typedef __traits::value_type            value_type;
		typedef __traits::difference_type       difference_type;
		typedef __traits::pointer               pointer;
		typedef __traits::reference             reference;
        using iterator_concept = std::__detail::__iter_concept<IT>;
        constexpr IT const& base() const noexcept { return current; }
        constexpr __iterator() noexcept : current{IT{}} {}
        constexpr explicit __iterator(IT const& __i) noexcept : current{__i} {}
        template<std::same_as<typename CT::pointer> JT> constexpr __iterator(__iterator<JT, CT> const& that) : current{ that.base() } {}
        constexpr reference operator*() const noexcept { return *current; }
        constexpr pointer operator->() const noexcept { return current; }
        constexpr reference operator[](difference_type __n) const noexcept { return current[__n]; }
        constexpr __iterator& operator++() noexcept { ++current; return *this; }
        constexpr __iterator operator++(int) noexcept { return __iterator{ current++ }; }
        constexpr __iterator& operator--() noexcept { --current; return *this; }
        constexpr __iterator operator--(int) noexcept { return __iterator { current-- }; }
        constexpr __iterator& operator+=(difference_type __n) noexcept { current += __n; return *this; }
        constexpr __iterator& operator-=(difference_type __n) noexcept { current -= __n; return *this; }
        constexpr __iterator operator+(difference_type __n) const noexcept { return __iterator{ current + __n }; }
        constexpr __iterator operator-(difference_type __n) const noexcept { return __iterator{ current - __n }; }
    };
    template<typename IT, std::totally_ordered_with<IT> JT, typename CT>
    constexpr bool operator == (__iterator<IT, CT> const& __this, __iterator<JT, CT> const& __that) noexcept { return __this.base() == __that.base(); }
    template<typename IT, std::totally_ordered_with<IT> JT, typename CT>
    constexpr bool operator != (__iterator<IT, CT> const& __this, __iterator<JT, CT> const& __that) noexcept { return __this.base() != __that.base(); }
    template<typename IT, std::totally_ordered_with<IT> JT, typename CT>
    constexpr bool operator <= (__iterator<IT, CT> const& __this, __iterator<JT, CT> const& __that) noexcept { return __this.base() <= __that.base(); }
    template<typename IT, std::totally_ordered_with<IT> JT, typename CT>
    constexpr bool operator >= (__iterator<IT, CT> const& __this, __iterator<JT, CT> const& __that) noexcept { return __this.base() >= __that.base(); }
    template<typename IT, std::totally_ordered_with<IT> JT, typename CT>
    constexpr bool operator  < (__iterator<IT, CT> const& __this, __iterator<JT, CT> const& __that) noexcept { return __this.base()  < __that.base(); }
    template<typename IT, std::totally_ordered_with<IT> JT, typename CT>
    constexpr bool operator  > (__iterator<IT, CT> const& __this, __iterator<JT, CT> const& __that) noexcept { return __this.base()  > __that.base(); }
}
#endif
