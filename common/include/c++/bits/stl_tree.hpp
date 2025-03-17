#ifndef __STL_TREE
#define __STL_TREE
/*
 * The most important thing eco-activists and programmers have in common is that we both love trees.
 * — Someone, somewhere, at some time, probably 
 */
#include "bits/move.h"
#include "memory"
#include "bits/iterator_concepts.hpp"
#include "bits/stl_pair.hpp"
#include "bits/stl_algobase.hpp"
#include "bits/aligned_buffer.hpp"
namespace std
{
    template<typename CT, typename T, typename U = T> concept __valid_comparator = is_default_constructible_v<CT> && requires(CT c, T t, U u) { { c(t, u) } -> __detail::__boolean_testable; { c(u, t) } -> __detail::__boolean_testable; };
    // Couldn't not make the Les Mis reference
    enum node_color
    {
        RED, // the blood of angry men; the world about to dawn; I feel my heart on fire; the color of desire
        BLACK // the dark of ages past; the night that ends at last; my world when she's not there; the color of despair
    };
    enum node_direction
    {
        // sliiiide to the...
        LEFT, 
        // sliiiide to the...
        RIGHT
        // two hops this time
        // cha-cha now y'all
    };
    struct __node_base
    {
        typedef __node_base* __ptr;
        typedef __node_base const* __const_ptr;
        node_color __my_color;
        __ptr __my_parent;
        __ptr __my_left;
        __ptr __my_right;
        static __ptr __min(__ptr x);
        static __const_ptr __min(__const_ptr x);
        static __ptr __max(__ptr x);
        static __const_ptr __max(__const_ptr x);
    };
    constexpr __node_base* __increment_node(__node_base* x)
    {
        if(x->__my_right) { x = x->__my_right; while(x->__my_left) x = x->__my_left; }
        else
        {
            __node_base* y = x->__my_parent;
            while(x == y->__my_right) { x = y; y = y->__my_parent; }
            if(x->__my_right != y) x = y;
        }
        return x;
    }
    constexpr __node_base const* __increment_node(__node_base const* x)
    {
        if(x->__my_right) { x = x->__my_right; while(x->__my_left) x = x->__my_left; }
        else
        {
            __node_base* y = x->__my_parent;
            while(x == y->__my_right) { x = y; y = y->__my_parent; }
            if(x->__my_right != y) x = y;
        }
        return x;
    }
    constexpr __node_base* __decrement_node(__node_base* x)
    {
        if(x->__my_color == RED && x->__my_parent->__my_parent == x) x = x->__my_right;
        else if(x->__my_left) { x = x->__my_left; while(x->__my_right) x = x->__my_right; }
        else
        {
            __node_base* y = x->__my_parent;
            while(x == y->__my_left) { x = y; y = y->__my_parent; }
            x = y;
        }
        return x;
    }
    constexpr __node_base const* __decrement_node(__node_base const* x)
    {
        if(x->__my_color == RED && x->__my_parent->__my_parent == x) x = x->__my_right;
        else if(x->__my_left) { x = x->__my_left; while(x->__my_right) x = x->__my_right; }
        else
        {
            __node_base* y = x->__my_parent;
            while(x == y->__my_left) { x = y; y = y->__my_parent; }
            x = y;
        }
        return x;
    }
    struct __tree_trunk
    {
        // This node's parent is the root node; its left is the min node; its right is the max node
        __node_base __trunk;
        size_t __count;
        constexpr void __reset() noexcept
        {
            __trunk.__my_color = RED;
            __trunk.__my_parent = nullptr;
            __trunk.__my_left = &__trunk;
            __trunk.__my_right = &__trunk;
            __count = 0;
        }
        constexpr __tree_trunk() noexcept :  __trunk{ RED, nullptr, &__trunk, &__trunk }, __count{ 0 } {}
        constexpr ~__tree_trunk() { if(__trunk.__my_parent) __trunk.__my_parent->__my_parent = nullptr; }
        constexpr void __copy(__tree_trunk const& that) noexcept
        {
            __trunk.__my_color = that.__trunk.__my_color;
            __trunk.__my_parent = that.__trunk.__my_parent;
            __trunk.__my_left = that.__trunk.__my_left;
            __trunk.__my_right = that.__trunk.__my_right;
            if(__trunk.__my_parent) __trunk.__my_parent->__my_parent = &__trunk;
            __count = that.__count;
        }
        constexpr void __move(__tree_trunk&& that) noexcept
        {
            __trunk.__my_color = that.__trunk.__my_color;
            __trunk.__my_parent = that.__trunk.__my_parent;
            __trunk.__my_left = that.__trunk.__my_left;
            __trunk.__my_right = that.__trunk.__my_right;
            if(__trunk.__my_parent) __trunk.__my_parent->__my_parent = &__trunk;
            __count = that.__count;
            that.__reset();
        }
        constexpr void __swap(__tree_trunk& that) noexcept
        {
            __tree_trunk tmp{ *this };
            this->__copy(that);
            that.__copy(tmp);
            tmp.__reset();
        }
        constexpr __tree_trunk(__tree_trunk&& that) noexcept : __tree_trunk{} { if(that.__trunk.__my_parent) __move(forward<__tree_trunk>(that)); }
        constexpr __tree_trunk(__tree_trunk const& that) noexcept : __tree_trunk{} { if(that.__trunk.__my_parent) __copy(that); }
        constexpr __tree_trunk& operator=(__tree_trunk const& that) noexcept { if(that.__trunk.__my_parent) __copy(that); return *this; }
        constexpr __tree_trunk& operator=(__tree_trunk&& that) noexcept { if(that.__trunk.__my_parent) __move(forward<__tree_trunk>(that)); return *this; }
        constexpr bool __have_left() const noexcept { return (&__trunk != __trunk.__my_left)&& __trunk.__my_left; }
        constexpr bool __have_right() const noexcept { return (&__trunk != __trunk.__my_right) && __trunk.__my_right; }
    };
    template<typename T>
    struct __node : public __node_base
    {
        typedef __node<T>* __link_ptr;
    private:
        ::__impl::__aligned_buffer<T> __my_data;
    public:
        constexpr __node() : __my_data{} {}
        constexpr T* __get_ptr() { return __my_data.__get_ptr(); }
        constexpr T const* __get_ptr() const { return __my_data.__get_ptr(); }
        constexpr T& __get_ref() { return *__get_ptr(); }
        constexpr T const& __get_ref() const { return *__get_ptr(); }
    };
    template<typename T>
    struct __tree_iterator
    {
        typedef T value_type;
        typedef add_lvalue_reference_t<T> reference;
        typedef add_pointer_t<T> pointer;
        typedef ptrdiff_t difference_type;
    private:
        typedef __tree_iterator<T> __iterator_type;
        typedef __node_base::__ptr __base_ptr;
        typedef __node<T>* __node_ptr;
        __base_ptr __my_node;
    public:
        constexpr __tree_iterator() noexcept : __my_node{} {}
        constexpr explicit __tree_iterator(__base_ptr x) noexcept : __my_node{ x } {}
        extension constexpr __node_ptr get_node() const noexcept { return static_cast<__node_ptr>(__my_node); }
        constexpr pointer base() const noexcept { return get_node()->__get_ptr(); }
        constexpr pointer operator->() const noexcept { return base(); }
        constexpr reference operator*() const noexcept { return *base(); }
        constexpr __iterator_type& operator++() noexcept { __my_node = __increment_node(__my_node); return *this; }
        constexpr __iterator_type operator++(int) noexcept { __iterator_type tmp = *this; __my_node = __increment_node(__my_node); return tmp; }
        constexpr __iterator_type& operator--() noexcept { __my_node = __decrement_node(__my_node); return *this; }
        constexpr __iterator_type operator--(int) noexcept { __iterator_type tmp = *this; __my_node = __decrement_node(__my_node); return tmp; }
        friend constexpr bool operator==(__tree_iterator<T> const& x, __tree_iterator<T> const& y) { return x.__my_node == y.__my_node; }
        constexpr operator bool() const { return __my_node != nullptr; }
    };
    template<typename T>
    struct __tree_const_iterator
    {
        typedef T value_type;
        typedef add_lvalue_reference_t<add_const_t<T>> reference;
        typedef add_pointer_t<add_const_t<T>> pointer;
        typedef ptrdiff_t difference_type;
    private:
        typedef __tree_iterator<T> __iterator_type;
        typedef __tree_const_iterator<T> __const_iterator_type;
        typedef __node_base::__const_ptr __base_ptr;
        typedef __node<T> const* __node_ptr;
        __base_ptr __my_node;
    public:
        constexpr __tree_const_iterator() noexcept : __my_node{} {}
        constexpr explicit __tree_const_iterator(__base_ptr x) noexcept : __my_node{ x } {}
        constexpr __tree_const_iterator(__iterator_type const& i) noexcept : __my_node{ i.get_node() } {}
        extension constexpr __node_ptr get_node() const noexcept { return static_cast<__node_ptr>(__my_node); }
        constexpr pointer base() const noexcept { return get_node()->__get_ptr(); }
        constexpr pointer operator->() const noexcept { return base(); }
        constexpr reference operator*() const noexcept { return *base(); }
        constexpr __const_iterator_type& operator++() noexcept { __my_node = __increment_node(__my_node); return *this; }
        constexpr __const_iterator_type operator++(int) noexcept { __const_iterator_type tmp = *this; __my_node = __increment_node(__my_node); return tmp; }
        constexpr __const_iterator_type& operator--() noexcept { __my_node = __decrement_node(__my_node); return *this; }
        constexpr __const_iterator_type operator--(int) noexcept { __const_iterator_type tmp = *this; __my_node = __decrement_node(__my_node); return tmp; }
        friend constexpr bool operator==(__tree_const_iterator<T> const& x, __tree_const_iterator<T> const& y) noexcept { return x.__my_node == y.__my_node; }
        constexpr operator bool() const noexcept { return __my_node != nullptr; }
    };
    [[gnu::nonnull]] void __insert_and_rebalance(const node_direction dir, __node_base* x, __node_base* p, __node_base& trunk) throw();
    [[gnu::nonnull]][[gnu::returns_nonnull]] __node_base* __rebalance_for_erase(__node_base* const z, __node_base& trunk) throw();
    template<typename T, __valid_comparator<T> CP, allocator_object<__node<T>> A>
    class __tree_base : protected __tree_trunk
    {
    protected: 
        typedef __node_base* __b_ptr;
        typedef __node_base const* __cb_ptr;
        typedef __node<T>* __link;
        typedef __node<T> const* __const_link;
        typedef pair<__link, __link> __pos_pair;
        typedef pair<__link, bool> __res_pair;
        typedef __tree_iterator<T> __iterator;
        typedef __tree_const_iterator<T> __const_iterator;
        typedef T __value_type;
        typedef A __alloc_type;
        typedef CP __compare_type;
        typedef __tree_trunk __trunk_type;
        __alloc_type __alloc{};
        constexpr void __clear_base() noexcept { __count = 0; this->__reset(); }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __pos_pair __pos_for_unique(U const& u);
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __pos_pair __pos_for_equal(U && u);
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __pos_pair __insert_unique_hint_pos(__const_link hint, U const& u);
        template<typename ... Args> requires constructible_from<T, Args...> constexpr __res_pair __emplace_unique(Args&& ... args);
        template<typename ... Args> requires constructible_from<T, Args...> constexpr __res_pair __hint_emplace_unique(__const_link hint, Args&& ... args);
        constexpr static __link __left_of(__link x) noexcept { return static_cast<__link>(x->__my_left); }
        constexpr static __link __right_of(__link x) noexcept { return static_cast<__link>(x->__my_right); }
        constexpr static __const_link __left_of(__const_link x) noexcept { return static_cast<__const_link>(x->__my_left); }
        constexpr static __const_link __right_of(__const_link x) noexcept { return static_cast<__const_link>(x->__my_right); }
        constexpr static __b_ptr __mini(__b_ptr x) noexcept { return __node_base::__min(x); }
        constexpr static __b_ptr __maxi(__b_ptr x) noexcept { return __node_base::__max(x); }
        constexpr static __cb_ptr __mini(__cb_ptr x) noexcept { return __node_base::__min(x); }
        constexpr static __cb_ptr __maxi(__cb_ptr x) noexcept { return __node_base::__max(x); }
        constexpr bool __compare(__b_ptr p, __b_ptr q) { return __compare_type{}(static_cast<__link>(p)->__get_ref(), static_cast<__link>(q)->__get_ref()); }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr bool __compare_r(__cb_ptr p, U const& u) const { return __compare_type{}(static_cast<__const_link>(p)->__get_ref(), u); }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr bool __compare_l(U const& u, __cb_ptr p) const { return __compare_type{}(u, static_cast<__const_link>(p)->__get_ref()); }
        constexpr __link __get_root() noexcept { return static_cast<__link>(this->__trunk.__my_parent); }
        constexpr __link __end() noexcept { return static_cast<__link>(addressof(this->__trunk)); }
        constexpr __const_link __get_root() const noexcept { return static_cast<__const_link>(this->__trunk.__my_parent); }
        constexpr __const_link __end() const noexcept { return static_cast<__const_link>(std::addressof(this->__trunk)); }
        constexpr __b_ptr& __leftmost() noexcept { return this->__trunk.__my_left; }
        constexpr __b_ptr& __rightmost() noexcept { return this->__trunk.__my_right; }
        constexpr __link __l_begin() noexcept {  return static_cast<__link>(this->__trunk.__my_left); }
        constexpr __link __l_rightmost() noexcept {  return static_cast<__link>(this->__trunk.__my_right); }
        constexpr __iterator __begin() noexcept { return __iterator { __l_begin() }; }
        constexpr __const_link __l_begin() const noexcept { return static_cast<__const_link>(this->__trunk.__my_left); }
        constexpr __const_link __l_rightmost() const noexcept { return static_cast<__const_link>(this->__trunk.__my_right); }
        constexpr __const_iterator __begin() const noexcept { return __const_iterator { __l_begin() }; }
        template<typename ... Args> requires constructible_from<T, Args...> constexpr __link __construct_node(Args&& ... args) { __link l = construct_at(__alloc.allocate(1UL)); construct_at(l->__get_ptr(), forward<Args>(args)...); l->__my_color = RED; return l; }
        constexpr void __destroy_node(__b_ptr n) { if(n) { __alloc.deallocate(static_cast<__link>(n), 1); } }
        constexpr __link __insert_node(__b_ptr x, __b_ptr p, __link l) { bool left = (x || p == __end() || __compare_l(l->__get_ref(), p)); __insert_and_rebalance(left ? LEFT : RIGHT, l, p, this->__trunk); this->__count++; return l; } 
        template<std::convertible_to<T> U> constexpr __link __insert(__b_ptr x, __b_ptr p, U const& u) { return __insert_node(x, p, __construct_node(u)); }
        template<std::convertible_to<T> U> constexpr __link __insert(__b_ptr x, __b_ptr p, U&& u) { return __insert_node(x, p, __construct_node(forward<U>(u)));}
        template<std::convertible_to<T> U> constexpr __link __insert_lower(__b_ptr p, U&& u) { bool left = (p == __end() || __compare_l(u, p));  __link l = __construct_node(forward<U>(u));  __insert_and_rebalance(left ? LEFT : RIGHT, l, p, this->__trunk); this->__count++; return l; }
        template<std::convertible_to<T> U> constexpr __link __insert_lower_equal(U&& u) { __link x = __get_root(), y = __end(); while(x) { y = x; x = !__compare_r(x, u) ? __left_of(x) : __right_of(x); } return __insert_lower(y, forward<U>(u)); }
        template<std::convertible_to<T> U> constexpr __res_pair __insert_unique(U const& u) { __pos_pair p = __insert_unique_hint_pos(nullptr, u); if(p.second) return __res_pair { __insert(p.first, p.second, u), true }; return __res_pair{ p.first, false }; }
        template<std::convertible_to<T> U> constexpr __res_pair __insert_unique(U && u) { __pos_pair p = __insert_unique_hint_pos(nullptr, u); if(p.second) return __res_pair { __insert(p.first, p.second, forward<U>(u)), true }; return __res_pair{ p.first, false }; }
        template<std::convertible_to<T> U> constexpr __link __insert_equal(U && u) { __pos_pair p = __pos_for_equal(u); return __insert(p.first, p.second, forward<U>(u)); }
        template<typename U> requires __valid_comparator<CP, T, U> __const_link __lower_bound(__const_link x, __const_link y, U const& u) const noexcept { while(x) if(!__compare_r(x, u)) y = x, x = __left_of(x); else x = __right_of(x); return y; }
        template<typename U> requires __valid_comparator<CP, T, U> __const_link __upper_bound(__const_link x, __const_link y, U const& u) const noexcept { while(x) if(__compare_l(u, x)) y = x, x = __left_of(x); else x = __right_of(x); return y; }
        template<typename U> requires __valid_comparator<CP, T, U> __link __lower_bound(__link x, __link y, U const& u) noexcept { while(x) if(!__compare_r(x, u)) y = x, x = __left_of(x); else x = __right_of(x); return y; }
        template<typename U> requires __valid_comparator<CP, T, U> __link __upper_bound(__link x, __link y, U const& u) noexcept { while(x) if(__compare_l(u, x)) y = x, x = __left_of(x);  else x = __right_of(x); return y; }
        template<std::convertible_to<T> U> constexpr __link __hint_insert_unique(__const_link hint, U const& u) { __pos_pair r = __insert_unique_hint_pos(hint, u); if(r.second) return __insert(r.first, r.second, u); return r.first; }
        template<std::convertible_to<T> U> constexpr __link __hint_insert_unique(__const_link hint, U && u) { __pos_pair r = __insert_unique_hint_pos(hint, u); if(r.second) return __insert(r.first, r.second, forward<U>(u)); return r.first; }
        template<matching_input_iterator<T> IT> constexpr void __insert_range_unique(IT st, IT ed) { for(; st != ed; st++) __insert_unique(*st); }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __link __find_node(U const& u) noexcept { __link result = __lower_bound(__get_root(), __end(), u); return (result == __end() || __compare_l(u, result)) ? __end() : result; }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __const_link __find_node(U const& u) const noexcept { __const_link result = __lower_bound(__get_root(), __end(), u); return (result == __end() || __compare_l(u, result)) ? __end() : result; }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr bool __contains(U const& u) const noexcept { return (this->__find_node(u) != this->__end()); }
        __b_ptr __erase_node(__b_ptr n)  { __b_ptr y = __rebalance_for_erase(n, this->__trunk); __b_ptr result = __increment_node(y); __destroy_node(y); this->__count--; return result; }
        __b_ptr __erase_nodes(__b_ptr first, __b_ptr last) { __b_ptr result = __end(); for(__b_ptr cur = first; cur != last; ++cur) { result = this->__erase_node(cur); } return result; }
        constexpr void __recursive_destroy(__link n) { while(n) { __recursive_destroy(__right_of(n)); __link m = __left_of(n); __destroy_node(n); n = m; } }
        constexpr void __recursive_destroy_base() { __recursive_destroy(__get_root()); }
        constexpr void __clear() { __recursive_destroy_base(); this->__clear_base(); }
    public:
        constexpr size_t size() const noexcept { return this->__count; }
        constexpr bool empty() const noexcept { return !this->size(); }
        constexpr ~__tree_base() { __recursive_destroy_base(); }
        constexpr __tree_base() : __tree_trunk{}, __alloc{} {}
        constexpr __tree_base(__tree_base const& that) : __tree_trunk{ that }, __alloc{} {}
        constexpr __tree_base(__tree_base&& that) : __tree_trunk{ forward<__tree_trunk>(that) }, __alloc{} {}
        template<matching_input_iterator<T> IT> constexpr __tree_base(IT st, IT ed) : __tree_base{} { this->__insert_range_unique(st, ed); }
        constexpr __tree_base& operator=(__tree_base const& that) { __clear();  this->__trunk = that.__trunk; this->__count = that.__count; return *this; }
        constexpr __tree_base& operator=(__tree_base&& that) { __clear(); this->__trunk = that.__trunk; this->__count = that.__count; return *this; }
    };
    template<typename T, __valid_comparator<T> CP, allocator_object<__node<T>> A>
    template<typename U>
    requires __valid_comparator<CP, T, U> 
    constexpr typename __tree_base<T, CP, A>::__pos_pair __tree_base<T, CP, A>::__pos_for_unique(U const& u) 
    {
        __link x = __get_root();
        __link y = __end();
        bool comp = true;
        while(x) { y = x; comp = __compare_l(u, x); x = comp ? __left_of(x) : __right_of(x); }
        __iterator j { y };
        if(comp) { if(j == __begin()) return __pos_pair{ x, y }; else --j; }
        if(__compare_r(j.get_node(), u)) return __pos_pair{ x, y };
        return __pos_pair{ static_cast<__link>(j.get_node()), nullptr };
    }
    template<typename T, __valid_comparator<T> CP, allocator_object<__node<T>> A>
    template<typename U>
    requires __valid_comparator<CP, T, U> 
    constexpr typename __tree_base<T, CP, A>::__pos_pair __tree_base<T, CP, A>::__pos_for_equal(U && u)
    {
        __link x = __get_root();
        __b_ptr y = __end();
        while(x) { y = x; x = !__compare_r(x, u) ? __left_of(x) : __right_of(x); }
        return __pos_pair{ x, y };
    }
    template<typename T, __valid_comparator<T> CP, allocator_object<__node<T>> A>
    template<typename U> 
    requires __valid_comparator<CP, T, U> 
    constexpr typename __tree_base<T, CP, A>::__pos_pair __tree_base<T, CP, A>::__insert_unique_hint_pos(__const_link hint, U const& u) 
    {
        __link pos = const_cast<__link>(hint);
        if(!hint || pos == __end()) { if(this->__count > 0 && __compare_r(__rightmost(), u)) return __pos_pair{ nullptr, __l_rightmost() }; else return __pos_for_unique(u); }
        else if(__compare_l(u, pos))
        {
            __link before =static_cast<__link>(__decrement_node(pos)); 
            if(pos == __l_begin()) return __pos_pair{ __l_begin(), __l_begin() };
            else if(__compare_r(before, u)) { if(!before->__my_right) return __pos_pair{ nullptr, before }; else return __pos_pair{ pos, pos }; } 
            else return __pos_for_unique(u);
        }
        else if(__compare_r(pos, u))
        {
            __link after = static_cast<__link>(__increment_node(pos));
            if(pos == __l_rightmost()) return __pos_pair{ nullptr, __l_rightmost() };
            else if(__compare_l(u, after)) { if(!pos->__my_right) return __pos_pair{ nullptr, pos }; else return __pos_pair{ after, after }; }
            else return __pos_for_unique(u);
        }
        return __pos_pair{ pos, nullptr };
    }
    template <typename T, __valid_comparator<T> CP, allocator_object<__node<T>> A>
    template <typename... Args> 
    requires constructible_from<T, Args...>
    constexpr typename __tree_base<T, CP, A>::__res_pair __tree_base<T, CP, A>::__emplace_unique(Args &&...args)
    {
        __link l = __construct_node(forward<Args>(args)...);
        __pos_pair r = __pos_for_unique(l->__get_ref());
        if(r.second) return __res_pair{ __insert_node(r.first, r.second, l), true };
        else __destroy_node(l);
        return __res_pair{ r.first, false};
    }
    template <typename T, __valid_comparator<T> CP, allocator_object<__node<T>> A>
    template <typename... Args> 
    requires constructible_from<T, Args...>
    constexpr typename __tree_base<T, CP, A>::__res_pair __tree_base<T, CP, A>::__hint_emplace_unique(__const_link hint, Args&& ... args) 
    { 
        __link l = __construct_node(forward<Args>(args)...);
        __pos_pair r = __insert_unique_hint_pos(hint, l->__get_ref());
        if(r.second) return __res_pair{ __insert_node(r.first, r.second, l), true };
        else __destroy_node(l);
        return __res_pair{ r.first, false };
    }
}
#endif