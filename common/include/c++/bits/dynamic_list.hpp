#ifndef __DYN_LIST
#define __DYN_LIST
#include "bits/aligned_buffer.hpp"
#include "bits/stl_iterator.hpp"
#include "bits/stl_algobase.hpp"
#include "memory"
#include "initializer_list"
namespace std
{
    namespace __impl
    {
        struct __list_node_base
        {
            __list_node_base* __prev;
            __list_node_base* __next;
            constexpr __list_node_base() noexcept = default;
            constexpr ~__list_node_base() noexcept = default;
            attribute(nonnull) constexpr __list_node_base(__list_node_base* prev) noexcept :
                __prev  { prev },
                __next  { prev->__next }
                        { __next->__prev = this; __prev->__next = this; }
            attribute(nonnull) constexpr __list_node_base(int, __list_node_base* next) noexcept :
                __prev  { next->__prev },
                __next  { next }
                        { __prev->__next = this; __next->__prev = this; }
            attribute(nonnull) constexpr void __relink_next(__list_node_base* that) noexcept { this->__next = that; that->__prev = this; }
            attribute(nonnull) constexpr void __relink_prev(__list_node_base* that) noexcept { this->__prev = that; that->__next = this; }
            constexpr __list_node_base(__list_node_base&& that) noexcept : __prev(that.__prev), __next(that.__next) { that.__prev = that.__next = nullptr; }
            constexpr __list_node_base& operator=(__list_node_base&& that) noexcept { __prev = that.__prev; __next = that.__next; that.__prev = that.__next = nullptr; return *this; }
        };
        struct __list_base
        {
            typedef add_pointer_t<__list_node_base> __node_base_ptr;
            typedef add_pointer_t<add_const_t<__list_node_base>> __const_node_base_ptr;
            typedef decltype(sizeof(__node_base_ptr)) size_type;
            typedef decltype(declval<__node_base_ptr>() - declval<__node_base_ptr>()) difference_type;
            __list_node_base __head;
            __list_node_base __tail;
            size_type __count;
            constexpr __node_base_ptr __begin() noexcept { return __head.__next; }
            constexpr __const_node_base_ptr __begin() const noexcept { return __head.__next; }
            constexpr __node_base_ptr __end() noexcept { return addressof(__tail); }
            constexpr __const_node_base_ptr __end() const noexcept { return addressof(__tail); }
            constexpr void __reset() noexcept { __head.__next = addressof(__tail); __tail.__prev = addressof(__head); __count = 0; }
            constexpr ~__list_base() noexcept = default;
            constexpr __list_base() noexcept :
                __head  {},
                __tail  {},
                __count { 0 }
                        { __reset(); }
            constexpr __list_base(__list_base&& that) noexcept :
                __head  { move(that.__head) },
                __tail  { move(that.__tail) },
                __count { that.__count }
                        { that.__reset(); }
            constexpr void __move_assign(__list_base&& that) noexcept { this->__head = move(that.__head); this->__tail = move(that.__tail); this->__count = that.__count; that.__reset(); }
        };
        template<typename T>
        struct __list_node : __list_node_base
        {
            typedef add_pointer_t<__list_node<T>> __link;
            typedef add_pointer_t<add_const_t<__list_node<T>>> __const_link;
            ::__impl::__aligned_buffer<T> __data;
            constexpr __list_node(__list_node_base* p) noexcept : __list_node_base(p), __data() {}
            constexpr __list_node(int, __list_node_base* n) noexcept : __list_node_base(0, n), __data() {}
            constexpr T* __ptr() noexcept { return __data.__get_ptr(); }
            constexpr T const* __ptr() const noexcept { return __data.__get_ptr(); }
            constexpr T& __ref() noexcept { return *__ptr(); }
            constexpr T const& __ref() const noexcept { return *__ptr(); }
        };
        template<typename T>
        struct __list_iterator
        {
            typedef T value_type;
            typedef add_lvalue_reference_t<T> reference;
            typedef add_pointer_t<T> pointer;
            typedef ptrdiff_t difference_type;
            typedef bidirectional_iterator_tag iterator_concept;
            typedef input_iterator_tag iterator_category;
        private:
            typedef __list_node_base __base_node_type;
            typedef __list_node<T> __node_type;
            typedef add_pointer_t<__base_node_type> __base_ptr;
            typedef add_pointer_t<__node_type> __node_ptr;
            typedef __list_iterator<T> __iterator_type;
            __base_ptr __my_node;
        public:
            constexpr __list_iterator() noexcept : __my_node() {}
            constexpr explicit __list_iterator(__base_ptr p) : __my_node(p) {}
            extension constexpr __node_ptr get_node() const noexcept { return static_cast<__node_ptr>(__my_node); }
            constexpr pointer base() const noexcept { return get_node()->__ptr(); }
            constexpr pointer operator->() const noexcept { return base(); }
            constexpr reference operator*() const noexcept { return *base(); }
            constexpr __iterator_type& operator++() { __my_node = __my_node->__next; return *this; }
            constexpr __iterator_type operator++(int) { __iterator_type that(__my_node); __my_node = __my_node->__next; return that; }
            constexpr __iterator_type& operator--() { __my_node = __my_node->__prev; return *this; }
            constexpr __iterator_type operator--(int) { __iterator_type that(__my_node); __my_node = __my_node->__prev; return that; }
            friend constexpr bool operator==(__iterator_type const& __this, __iterator_type const& __that) noexcept { return __this.__my_node == __that.__my_node; }
        };
        template<typename T>
        struct __list_const_iterator
        {
            typedef T value_type;
            typedef add_lvalue_reference_t<add_const_t<T>> reference;
            typedef add_pointer_t<add_const_t<T>> pointer;
            typedef ptrdiff_t difference_type;
            typedef bidirectional_iterator_tag iterator_concept;
            typedef input_iterator_tag iterator_category;
        private:
            typedef __list_node_base __base_node_type;
            typedef __list_node<T> __node_type;
            typedef add_pointer_t<add_const_t<__base_node_type>> __base_ptr;
            typedef add_pointer_t<add_const_t<__node_type>> __node_ptr;
            typedef __list_iterator<T> __iterator_type;
            typedef __list_const_iterator<T> __const_iterator_type;
            __base_ptr __my_node;
        public:
            constexpr __list_const_iterator() noexcept : __my_node() {}    
            constexpr explicit __list_const_iterator(__base_ptr p) : __my_node(p) {}
            constexpr __list_const_iterator(__iterator_type const& i) noexcept : __my_node(i.get_node()) {}
            extension constexpr __node_ptr get_node() const noexcept { return static_cast<__node_ptr>(__my_node); }
            constexpr pointer base() const noexcept { return get_node()->__ptr(); }
            constexpr pointer operator->() const noexcept { return base(); }
            constexpr reference operator*() const noexcept { return *base(); }
            constexpr __const_iterator_type& operator++() { __my_node = __my_node->__next; return *this; }
            constexpr __const_iterator_type operator++(int) { __const_iterator_type that(__my_node); __my_node = __my_node->__next; return that; }
            constexpr __const_iterator_type& operator--() { __my_node = __my_node->__prev; return *this; }
            constexpr __const_iterator_type operator--(int) { __const_iterator_type that(__my_node); __my_node = __my_node->__prev; return that; }
            friend constexpr bool operator==(__const_iterator_type const& __this, __const_iterator_type const& __that) noexcept { return __this.__my_node == __that.__my_node; }
        };
        template<typename T, allocator_object<T> AT>
        class __dynamic_list : protected __list_base
        {
            using __base = __list_base;
        protected:
            using __node_type = __list_node<T>;
            using __rebind_alloc = typename AT::template rebind<__node_type>;
            typedef typename __node_type::__link __node_ptr;
            typedef typename __node_type::__const_link __const_node_ptr;
            typedef typename __rebind_alloc::other __allocator;
            using typename __base::__node_base_ptr;
            using typename __base::__const_node_base_ptr;
        public:
            using typename __base::size_type;
            using typename __base::difference_type;
            typedef T value_type;
            typedef __list_iterator<value_type> iterator;
            typedef __list_const_iterator<value_type> const_iterator;
            typedef typename iterator::reference reference;
            typedef typename const_iterator::reference const_reference;
            typedef typename iterator::pointer pointer;
            typedef typename const_iterator::pointer const_pointer;
            typedef std::reverse_iterator<iterator> reverse_iterator;
            typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        protected:
            __allocator __alloc{};
            template<typename ... Args> requires constructible_from<T, Args...> constexpr __node_ptr __create_after(__const_node_base_ptr prev, Args&& ... args);
            template<typename ... Args> requires constructible_from<T, Args...> constexpr __node_ptr __create_before(__const_node_base_ptr next, Args&& ... args);
            constexpr __node_base_ptr __erase(__node_ptr what);
            constexpr void __destroy() { __node_ptr p = static_cast<__node_ptr>(this->__head.__next); for(__node_ptr n = static_cast<__node_ptr>(p->__next); n != addressof(this->__tail); p = n, n = static_cast<__node_ptr>(n->__next)) { __alloc.deallocate(p, 1); } }
            constexpr __node_ptr __put_front(value_type const& v) requires copy_constructible<value_type> { return this->__create_after(addressof(this->__head), v); }
            constexpr __node_ptr __put_front(value_type&& v) requires move_constructible<value_type> { return this->__create_after(addressof(this->__head), move(v)); }  
            constexpr __node_ptr __put_back(value_type const& v) requires copy_constructible<value_type> { return this->__create_before(addressof(this->__tail), v); }
            constexpr __node_ptr __put_back(value_type&& v) requires move_constructible<value_type> { return this->__create_before(addressof(this->__tail), move(v)); }
        public:
            constexpr size_type size() const noexcept { return this->__count; }
            constexpr iterator begin() noexcept { return iterator(this->__begin()); }
            constexpr const_iterator cbegin() const noexcept { return const_iterator(this->__begin()); }
            constexpr const_iterator begin() const noexcept { return cbegin(); }
            constexpr iterator end() noexcept { return iterator(this->__end()); }
            constexpr const_iterator cend() const noexcept { return const_iterator(this->__end()); }
            constexpr const_iterator end() const noexcept { return cend(); }
            constexpr reverse_iterator rbegin() { return reverse_iterator(begin()); }
            constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(cbegin()); }
            constexpr const_reverse_iterator rbegin() const { return crbegin(); }
            constexpr reverse_iterator rend() { return reverse_iterator(end()); }
            constexpr const_reverse_iterator crend() const { return const_reverse_iterator(cend()); }
            constexpr const_reverse_iterator rend() const { return crend(); }
            constexpr AT get_allocator() const noexcept { return AT(__alloc); }
            template<typename ... Args> requires constructible_from<T, Args...> constexpr iterator emplace_front(Args&& ... args) { return iterator(this->__create_after(addressof(this->__head), forward<Args>(args)...)); }
            template<typename ... Args> requires constructible_from<T, Args...> constexpr iterator emplace_back(Args&& ... args) { return iterator(this->__create_before(addressof(this->__tail), forward<Args>(args)...)); }
            template<typename ... Args> requires constructible_from<T, Args...> constexpr iterator emplace(const_iterator pos, Args&& ... args) { return iterator(this->__create_after(pos.get_node(), forward<Args>(args)...)); }
            constexpr iterator insert(const_iterator pos, value_type const& v) requires copy_constructible<value_type> { if(pos == end()) pos--; return iterator(this->__create_after(pos.get_node(), v)); }
            constexpr iterator insert(const_iterator pos, value_type&& v) requires move_constructible<value_type> { if(pos == end()) pos--; return iterator(this->__create_after(pos.get_node(), move(v))); }
            template<matching_input_iterator<T> IT> constexpr iterator insert(const_iterator pos, IT start, IT end) { if(start == end) return iterator(const_cast<__node_ptr>(pos.get_node())); iterator result(this->__create_after(pos.base(), *start++)); __node_ptr p = result.get_node(); for(IT i = start; i != end; ++i) { p = this->__create_after(p, *i); } return result; }
            constexpr void push_front(value_type const& v) requires copy_constructible<value_type> { this->__put_front(v); }
            constexpr void push_front(value_type&& v) requires move_constructible<value_type> { this->__put_front(move(v)); }
            constexpr void push_back(value_type const& v) requires copy_constructible<value_type> { this->__put_back(v); }
            constexpr void push_back(value_type&& v) requires move_constructible<value_type> { this->__put_back(move(v)); }
            constexpr iterator erase(const_iterator what) { if(what != end()) return iterator(this->__erase(const_cast<__node_ptr>(what.get_node()))); return end(); }
            constexpr void clear() { this->__destroy(); this->__reset(); }
            constexpr void pop_back() { if(this->__count) { this->__erase(static_cast<__node_ptr>(this->__tail.__prev)); } }
            constexpr void pop_front() { if(this->__count) { this->__erase(static_cast<__node_ptr>(this->__head.__next)); } }
            constexpr __dynamic_list() noexcept = default;
            constexpr ~__dynamic_list() { this->__destroy(); }
            template<matching_input_iterator<T> IT> constexpr __dynamic_list(IT start, IT end) : __base() { for(IT i = start; i != end; ++i) { this->__create_before(addressof(this->__tail), *i); } }
            constexpr __dynamic_list(initializer_list<value_type> ilist) : __dynamic_list(ilist.begin(), ilist.end()) {}
            constexpr __dynamic_list(__dynamic_list const& that) : __dynamic_list(that.begin(), that.end()) {}
            constexpr __dynamic_list(__dynamic_list&& that) : __base(move(that)) {}
            constexpr __dynamic_list& operator=(__dynamic_list&& that) { clear(); this->__move_assign(move(that)); return *this; }
            constexpr __dynamic_list& operator=(__dynamic_list const& that) requires copy_constructible<value_type> { clear(); for(const_iterator i = that.begin(); i != that.end(); i++) { this->__put_back(*i); } return *this; }
        };
        template <typename T, allocator_object<T> AT>
        template <typename... Args>
        requires constructible_from<T, Args...>
        constexpr typename __dynamic_list<T, AT>::__node_ptr __dynamic_list<T, AT>::__create_after(__const_node_base_ptr prev, Args&& ...args)
        {
            __node_ptr n = construct_at(__alloc.allocate(1), const_cast<__node_base_ptr>(prev));
            construct_at(n->__ptr(), forward<Args>(args)...);
            this->__count++;
            return n;
        }
        template <typename T, allocator_object<T> AT>
        template <typename... Args>
        requires constructible_from<T, Args...>
        constexpr typename __dynamic_list<T, AT>::__node_ptr __dynamic_list<T, AT>::__create_before(__const_node_base_ptr next, Args&& ...args)
        {
            __node_ptr n = construct_at(__alloc.allocate(1), 0, const_cast<__node_base_ptr>(next));
            construct_at(n->__ptr(), forward<Args>(args)...);
            this->__count++;
            return n;
        }
        template <typename T, allocator_object<T> AT>
        constexpr typename __dynamic_list<T, AT>::__node_base_ptr __dynamic_list<T, AT>::__erase(__node_ptr what)
        {
            __node_base_ptr prev = what->__prev;
            __node_base_ptr next = what->__next;
            prev->__relink_next(next);
            next->__relink_prev(prev);
            __alloc.deallocate(what, 1);
            this->__count--;
            return next;
        }
    }
}
#endif