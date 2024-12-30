#ifndef __STL_TREE
#define __STL_TREE
/*
 * The most important thing eco-activists and programmers have in common is that we both love trees.
 * -- Someone, somewhere, at some time, probably 
 */
#include "bits/move.h"
#include "memory"
#include "bits/iterator_concepts.hpp"
#include "bits/stl_pair.hpp"
#include "bits/stl_algobase.hpp"
namespace __impl
{
    template<typename T> 
    struct __aligned_buffer : std::aligned_storage<sizeof(T), alignof(T)> 
    {
        typename std::aligned_storage<sizeof(T), alignof(T)>::type __my_storage;  
        __aligned_buffer() = default; 
        __aligned_buffer(std::nullptr_t) {}
        constexpr void* __get_addr() noexcept { return static_cast<void*>(&__my_storage); }
        constexpr const void* __get_addr() const noexcept { return static_cast<const void*>(&__my_storage); }
        constexpr T* __get_ptr() noexcept { return static_cast<T*>(__get_addr()); }
        constexpr T const* __get_ptr() const noexcept { return static_cast<T const*>(__get_addr()); }
    };
}
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
    __node_base* __increment_node(__node_base* x) throw();
    __node_base const* __increment_node(__node_base const* x) throw();
    __node_base* __decrement_node(__node_base* x) throw();
    __node_base const* __decrement_node(__node_base const* x) throw();
    struct __tree_trunk
    {
        // This node's parent is the root node; its left is the min node; its right is the max node
        __node_base __trunk; 
        size_t __count;
        constexpr inline void __reset() noexcept
        {
            __trunk.__my_color = RED;
            __trunk.__my_parent = NULL;
            __trunk.__my_left = &__trunk;
            __trunk.__my_right = &__trunk;
            __count = 0;
        }
        constexpr __tree_trunk() noexcept :  __trunk{ RED, NULL, NULL, NULL }, __count{0} { __reset(); }
        constexpr inline void __copy(__tree_trunk const& that) noexcept
        {
            __trunk.__my_color = that.__trunk.__my_color;
            __trunk.__my_parent = that.__trunk.__my_parent;
            __trunk.__my_left = that.__trunk.__my_left;
            __trunk.__my_right = that.__trunk.__my_right;
            if(__trunk.__my_parent) __trunk.__my_parent->__my_parent = &__trunk;
            __count = that.__count;
        }
        constexpr inline void __move(__tree_trunk&& that) noexcept
        {
            __trunk.__my_color = that.__trunk.__my_color;
            __trunk.__my_parent = that.__trunk.__my_parent;
            __trunk.__my_left = that.__trunk.__my_left;
            __trunk.__my_right = that.__trunk.__my_right;
            if(__trunk.__my_parent) __trunk.__my_parent->__my_parent = &__trunk;
            __count = that.__count;
            that.__reset();
        }
        constexpr __tree_trunk(__tree_trunk&& that) noexcept : __tree_trunk{} { if(that.__trunk.__my_parent != NULL) __move(forward<__tree_trunk>(that)); }
        constexpr __tree_trunk(__tree_trunk const& that) noexcept : __tree_trunk{} { if(that.__trunk.__my_parent != NULL) __copy(that); }
        constexpr __tree_trunk& operator=(__tree_trunk const& that) noexcept { if(that.__trunk.__my_parent != NULL) __copy(that); return *this; }
        constexpr __tree_trunk& operator=(__tree_trunk&& that) noexcept { if(that.__trunk.__my_parent != NULL) __move(forward<__tree_trunk>(that)); return *this; }
    };
    template<typename T>
    struct __node : public __node_base
    {
        typedef __node<T>* __link_ptr;
    private:
        ::__impl::__aligned_buffer<T> __my_data{};
    public:
        constexpr __node() {}
        constexpr T* __get_ptr() { return __my_data.__get_ptr(); }
        constexpr const T* __get_ptr() const { return __my_data.__get_ptr(); }
        constexpr T& __get_ref() { return *__get_ptr(); }
        constexpr T const& __get_ref() const { return *__get_ptr(); }
    };
    template<typename T>
    struct __tree_iterator
    {
        typedef T value_type;
        typedef T& reference;
        typedef T* pointer;
        typedef ptrdiff_t difference_type;
        typedef __tree_iterator<T> __it_t;
        typedef __node_base::__ptr __bp_t;
        typedef __node<T>* __lp_t;
        __bp_t __my_node;
        constexpr __tree_iterator() noexcept : __my_node{} {}
        constexpr explicit __tree_iterator(__bp_t x) noexcept : __my_node{x} {}
        constexpr reference operator*() const noexcept { return *(static_cast<__lp_t>(__my_node)->__get_ptr()); }
        constexpr pointer operator->() const noexcept { return static_cast<__lp_t>(__my_node)->__get_ptr(); }
        constexpr __it_t& operator++() noexcept { __my_node = __increment_node(__my_node); return *this; }
        constexpr __it_t operator++(int) noexcept { __it_t tmp = *this; __my_node = __increment_node(__my_node); return tmp; }
        constexpr __it_t& operator--() noexcept { __my_node = __decrement_node(__my_node); return *this; }
        constexpr __it_t operator--(int) noexcept { __it_t tmp = *this; __my_node = __decrement_node(__my_node); return tmp; }
        friend constexpr bool operator==(__tree_iterator<T> const& x, __tree_iterator<T> const& y) { return x.__my_node == y.__my_node; }
        friend constexpr bool operator!=(__tree_iterator<T> const& x, __tree_iterator<T> const& y) { return x.__my_node != y.__my_node; }
        constexpr operator bool() const { return __my_node != NULL; }
        constexpr bool operator!() const { return __my_node == NULL; }
    };
    template<typename T>
    struct __tree_const_iterator
    {
        typedef T value_type;
        typedef T const& reference;
        typedef T const* pointer;
        typedef ptrdiff_t difference_type;
        typedef __tree_iterator<T> __it_t;
        typedef __tree_const_iterator<T> __ci_t;
        typedef __node_base::__const_ptr __bp_t;
        typedef __node<T> const* __lp_t;
        __bp_t __my_node;
        constexpr __tree_const_iterator() noexcept : __my_node{} {}
        constexpr explicit __tree_const_iterator(__bp_t x) noexcept : __my_node{x} {}
        constexpr __tree_const_iterator(__it_t const& i) noexcept : __my_node{i.__my_node} {}
        constexpr reference operator*() const noexcept { return *(static_cast<__lp_t>(__my_node)->__get_ptr()); }
        constexpr pointer operator->() const noexcept { return static_cast<__lp_t>(__my_node)->__get_ptr(); }
        constexpr __ci_t& operator++() noexcept { __my_node = __increment_node(__my_node); return *this; }
        constexpr __ci_t operator++(int) noexcept { __ci_t tmp = *this; __my_node = __increment_node(__my_node); return tmp; }
        constexpr __ci_t& operator--() noexcept { __my_node = __decrement_node(__my_node); return *this; }
        constexpr __ci_t operator--(int) noexcept { __ci_t tmp = *this; __my_node = __decrement_node(__my_node); return tmp; }
        friend constexpr bool operator==(__tree_const_iterator<T> const& x, __tree_const_iterator<T> const& y) noexcept { return x.__my_node == y.__my_node; }
        friend constexpr bool operator!=(__tree_const_iterator<T> const& x, __tree_const_iterator<T> const& y) noexcept { return x.__my_node != y.__my_node; }
        constexpr operator bool() const noexcept { return __my_node != NULL; }
        constexpr bool operator!() const noexcept { return __my_node == NULL; }
    };
    __attribute__((__nonnull__)) void __insert_and_rebalance(const node_direction dir, __node_base* x, __node_base* p, __node_base& trunk) throw();
    __attribute__((__nonnull__, __returns_nonnull__)) __node_base* __rebalance_for_erase(__node_base* const z, __node_base& trunk) throw();
    unsigned int __black_count(const __node_base* node, const __node_base* root);
    template<typename T, allocator_object<__node<T>> A>
    struct __trunk_impl : public __tree_trunk
    {
        A __allocator{};
        constexpr __trunk_impl() : __tree_trunk{} {}
        constexpr __trunk_impl(__trunk_impl&& that) : __tree_trunk{ move(that) } {}
        constexpr __trunk_impl(__trunk_impl const& that) : __tree_trunk{ that }{}
        constexpr __trunk_impl& operator=(__trunk_impl const& that) { __trunk = that.__trunk; __count = that.__count; return *this; }
        constexpr __trunk_impl& operator=(__trunk_impl&& that) { __trunk = move(that).__trunk; __count = move(that).__count; return *this; }
        constexpr void __clear() noexcept { new (static_cast<__node<T>*>(&__trunk)) __node<T>{}; __count = 0; }
    };
    template<typename T, __valid_comparator<T> CP, allocator_object<__node<T>> A>
    class __tree_base
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
        typedef __alloc_rebind<A, __node_base> __base_alloc_type;
        typedef __trunk_impl<T, A> __trunk_type;
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __pos_pair __pos_for_unique(U const& u);
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __pos_pair __pos_for_equal(U && u);
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __pos_pair __insert_unique_hint_pos(__const_link hint, U const& u);
        template<typename ... Args> requires constructible_from<T, Args...> constexpr __link __emplace_unique(Args&& ... args);
        template<typename ... Args> requires constructible_from<T, Args...> constexpr __link __emplace_unique(__const_link hint, Args&& ... args);
        constexpr static __link __left_of(__link x) noexcept { return static_cast<__link>(x->__my_left); }
        constexpr static __link __right_of(__link x) noexcept { return static_cast<__link>(x->__my_right); }
        constexpr static __const_link __left_of(__const_link x) noexcept { return static_cast<__const_link>(x->__my_left); }
        constexpr static __const_link __right_of(__const_link x) noexcept { return static_cast<__const_link>(x->__my_right); }
        constexpr static __b_ptr __mini(__b_ptr x) noexcept { return __node_base::__min(x); }
        constexpr static __b_ptr __maxi(__b_ptr x) noexcept { return __node_base::__max(x); }
        constexpr static __cb_ptr __mini(__cb_ptr x) noexcept { return __node_base::__min(x); }
        constexpr static __cb_ptr __maxi(__cb_ptr x) noexcept { return __node_base::__max(x); }
        __compare_type __comparator{};
        __trunk_type __my_impl{};
        __alloc_type __alloc{};
        __base_alloc_type __a2{};
        constexpr bool __compare(__b_ptr p, __b_ptr q) { return __comparator(static_cast<__link>(p)->__get_ref(), static_cast<__link>(q)->__get_ref()); }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr bool __compare_r(__b_ptr p, U const& u) { return __comparator(static_cast<__link>(p)->__get_ref(), u); }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr bool __compare_l(U const& u, __b_ptr p) { return __comparator(u, static_cast<__link>(p)->__get_ref()); }
        constexpr __link __get_root() noexcept { return static_cast<__link>(__my_impl.__trunk.__my_parent); }
        constexpr __link __end() noexcept { return static_cast<__link>(&__my_impl.__trunk); }
        constexpr __const_link __get_root() const noexcept { return static_cast<__const_link>(__my_impl.__trunk.__my_parent); }
        constexpr __const_link __end() const noexcept { return static_cast<__const_link>(&__my_impl.__trunk); }
        constexpr __b_ptr& __leftmost() noexcept { return __my_impl.__trunk.__my_left; }
        constexpr __b_ptr& __rightmost() noexcept { return __my_impl.__trunk.__my_right; }
        constexpr __link __l_begin() noexcept {  return static_cast<__link>(__my_impl.__trunk.__my_left); }
        constexpr __link __l_rightmost() noexcept {  return static_cast<__link>(__my_impl.__trunk.__my_right); }
        constexpr __iterator __begin() noexcept { return __iterator { __l_begin() }; }
        constexpr __const_link __l_begin() const noexcept { return static_cast<__const_link>(__my_impl.__trunk.__my_left); }
        constexpr __const_link __l_rightmost() const noexcept { return static_cast<__const_link>(__my_impl.__trunk.__my_right); }
        constexpr __const_iterator __begin() const noexcept { return __const_iterator { __l_begin() }; }
        template<std::convertible_to<T> U> constexpr __link __construct_node(U&& u) { __link l = __alloc.allocate(1); construct_at(l->__get_ptr(), forward<U>(u)); l->__my_color = RED; return l; }
        template<typename ... Args> requires constructible_from<T, Args...> constexpr __link __construct_node(Args&& ... args) { __link l = __alloc.allocate(1); construct_at(l->__get_ptr(), forward<Args>(args)...); l->__my_color = RED; return l; }
        constexpr void __destroy_node(__b_ptr n) { if(n) __a2.deallocate(n, 1); }
        constexpr __link __insert_node(__b_ptr x, __b_ptr p, __link l) { __insert_and_rebalance((x != NULL || p == __end() || __compare(l, p)) ? LEFT : RIGHT, l, p, __my_impl.__trunk); __my_impl.__count++; return l; }
        constexpr __link __insert_node_lower(__b_ptr p, __link l) { __insert_and_rebalance((p == __end() || __compare(l, p)) ? LEFT : RIGHT, l, p, __my_impl.__trunk); __my_impl.__count++; return l; }
        template<std::convertible_to<T> U> constexpr __link __insert(__b_ptr x, __b_ptr p, U&& u) { return __insert_node(x, p, __construct_node(forward<U>(u))); }
        template<std::convertible_to<T> U> constexpr __link __insert_lower(__b_ptr p, U&& u) { return __insert_node_lower(p, __construct_node(forward<U>(u))); }
        template<std::convertible_to<T> U> constexpr __link __insert_lower_equal(U&& u) { __link x = __get_root(), y = __end(); while(x) { y = x; x = !__compare_r(x, u) ? __left_of(x) : __right_of(x); } return __insert_lower(y, forward<U>(u)); }
        template<std::convertible_to<T> U> constexpr __res_pair __insert_unique(U && u) { __pos_pair p = __pos_for_unique(u); if(p.second) return __res_pair { __insert(p.first, p.second, forward<U>(u)), true }; return __res_pair{ p.first, false }; }
        template<std::convertible_to<T> U> constexpr __link __insert_equal(U && u) { __pos_pair p = __pos_for_equal(u); return __insert(p.first, p.second, forward<U>(u)); }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __const_link __lower_bound(__const_link x, __const_link y, U const& u) const { while(x)  if(!__compare_r(x, u)) y = x, x = __left_of(x); else x = __right_of(x); return y; }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __const_link __upper_bound(__const_link x, __const_link y, U const& u) const { while(x) if(__compare_l(u, x)) y = x, x = __left_of(x); else x = __right_of(x);  return y; }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __link __lower_bound(__link x, __link y, U const& u) { while(x) if(!__compare_r(x, u)) y = x, x = __left_of(x); else x = __right_of(x); return y; }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __link __upper_bound(__link x, __link y, U const& u) { while(x) if(__compare_l(u, x)) y = x, x = __left_of(x);  else x = __right_of(x); return y; }
        template<std::convertible_to<T> U> constexpr __link __insert_unique(__const_link hint, U&& u) { __pos_pair r = __insert_unique_hint_pos(hint, u); if(r.second) return __insert(r.first, r.second, forward<U>(u)); return r.first; }
        template<matching_input_iterator<T> JT> constexpr void __insert_range(JT st, JT ed) { for(; st != ed; st++) __insert_unique(*st); }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __link __find_node(U const& u) { __link result = __lower_bound(__get_root(), __end(), u); return (result == __end() || __compare_l(u, result)) ? __end() : result; }
        template<typename U> requires __valid_comparator<CP, T, U> constexpr __const_link __find_node(U const& u) const {  __const_link result = __lower_bound(__get_root(), __end(), u); return (result == __end() || __compare_l(u, result)) ? __end() : result; }
        void __erase_node(__node_base* n)  { __link y = static_cast<__link>(__rebalance_for_erase(n, __my_impl.__trunk)); __destroy_node(y); __my_impl.__count--; }
        constexpr void __recursive_destroy(__b_ptr b) { if(b->__my_left) __recursive_destroy(b->__my_left); if(b->__my_right) __recursive_destroy(b->__my_right); __destroy_node(b); }
        constexpr void __clear() { __recursive_destroy(__my_impl.__trunk.__my_left); __recursive_destroy(__my_impl.__trunk.__my_right); __my_impl.__clear(); }
    public:
        constexpr size_t size() const noexcept { return __my_impl.__count; }
        constexpr ~__tree_base() { __recursive_destroy(&__my_impl.__trunk); }
        constexpr __tree_base() : __comparator{}, __my_impl{}, __alloc{}, __a2{}  {}
        constexpr __tree_base(__tree_base const& that) : __comparator{}, __my_impl{ that.__my_impl }, __alloc{}, __a2{} {}
        constexpr __tree_base(__tree_base&& that) : __comparator{}, __my_impl{ forward<__trunk_type>(that.__my_impl) }, __alloc{}, __a2{} {}
        constexpr __tree_base& operator=(__tree_base const& that) { __recursive_destroy(&__my_impl.__trunk); __my_impl = that.__my_impl; return *this; }
        constexpr __tree_base& operator=(__tree_base&& that) { __recursive_destroy(&__my_impl.__trunk); __my_impl = forward<__trunk_type>(that.__my_impl); return *this; }
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
        if(__compare_r(j.__my_node, u)) return __pos_pair{x, y};
        return __pos_pair{ static_cast<__link>(j.__my_node), NULL };
    }
    template<typename T, __valid_comparator<T> CP, allocator_object<__node<T>> A>
    template<typename U>
    requires __valid_comparator<CP, T, U> 
    constexpr typename __tree_base<T, CP, A>::__pos_pair __tree_base<T, CP, A>::__pos_for_equal(U && u)
    {
        __link x = __get_root();
        __b_ptr y = __end();
        while(x)
        {
            y = x;
            x = !__compare_r(x, u) ? __left_of(x) : __right_of(x);
        }
        return __pos_pair{ x, y };
    }
    template<typename T, __valid_comparator<T> CP, allocator_object<__node<T>> A>
    template<typename U> 
    requires __valid_comparator<CP, T, U> 
    constexpr typename __tree_base<T, CP, A>::__pos_pair __tree_base<T, CP, A>::__insert_unique_hint_pos(__const_link hint, U const& u) 
    {
        __link pos = const_cast<__link>(hint);
        if(pos == __end())
        {
            if(__my_impl.__count > 0 && __compare_r(__rightmost(), u)) return __pos_pair{ NULL, __l_rightmost() };
            else return __pos_for_unique(u);
        }
        else if(__compare_l(u, pos))
        {
            __link before = pos;
            if(pos == __l_begin()) return __pos_pair{ __l_begin(), __l_begin() };
            else if(__compare_r(static_cast<__link>(__decrement_node(before)), u))
            {
                if(!__right_of(before)) return __pos_pair{ NULL, before };
                else return __pos_pair{pos, pos};
            }
            else return __pos_for_unique(u);
        }
        else if(__compare_r(pos, u))
        {
            __link after = pos;
            if(pos == __l_rightmost()) return __pos_pair{ NULL, __l_rightmost() };
            else if(__compare_l(u, static_cast<__link>(__increment_node(after))))
            {
                if(!__right_of(pos)) return __pos_pair{ NULL, pos };
                else return __pos_pair{ after, after };
            }
            else return __pos_for_unique(u);
        }
        return __pos_pair{ pos, NULL };
    }
    template <typename T, __valid_comparator<T> CP, allocator_object<__node<T>> A>
    template <typename... Args> 
    requires constructible_from<T, Args...>
    constexpr typename __tree_base<T, CP, A>::__link __tree_base<T, CP, A>::__emplace_unique(Args &&...args)
    {
        __link l = __construct_node(forward<Args>(args)...);
        __pos_pair r = __pos_for_unique(l->__get_ref());
        if(r.second) return __insert_node(r.first, r.second, l);
        __destroy_node(l);
        return r.first;
    }
    template <typename T, __valid_comparator<T> CP, allocator_object<__node<T>> A>
    template <typename... Args> 
    requires constructible_from<T, Args...>
    constexpr typename __tree_base<T, CP, A>::__link __tree_base<T, CP, A>::__emplace_unique(__const_link hint, Args&& ... args) 
    { 
        __link l = __construct_node(forward<Args>(args)...);
        __pos_pair r = __insert_unique_hint_pos(hint, l->__get_ref());
        if(r.second) return __insert_node(r.first, r.second, l);
        __destroy_node(l);
        return r.first;
    }
}
#endif