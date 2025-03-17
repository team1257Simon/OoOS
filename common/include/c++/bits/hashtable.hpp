#ifndef __HASHTABLE
#define __HASHTABLE
#include "bits/move.h"
#include "bits/aligned_buffer.hpp"
#include "bits/stl_iterator.hpp"
#include "memory"
#include "tuple"
#include "bits/functional_compare.hpp"
namespace std
{
    namespace __detail
    {
        template<typename FT, typename KT> concept __hash_ftor = is_default_constructible_v<FT> && unsigned_integral<decltype(declval<FT>()(declval<KT const&>()))>;
        template<typename FT, typename KT, typename VT> concept __key_extract = is_default_constructible_v<FT> && is_same_v<decltype(declval<FT>()(declval<VT const&>())), KT const&>;
        template<typename FT, typename KT> concept __predicate = is_default_constructible_v<FT> && __boolean_testable<decltype(declval<FT>()(declval<KT const&>(), declval<KT const&>()))>;
        template<typename KT, typename MT> struct __pair_key_extract { constexpr KT const& operator()(pair<KT, MT> const& p) const noexcept { return p.first; } };
        template<typename T> struct __identity_key { constexpr T const& operator()(T const& t) const noexcept { return t; } };
    }
    namespace __impl
    {
        struct __hash_node_base
        {
            __hash_node_base* __next = nullptr; // next node in the chain (i.e. same bucket)
            constexpr __hash_node_base() noexcept = default;
            constexpr __hash_node_base(__hash_node_base* n) noexcept : __next{ n } {}
        };
        struct __hashtable_base
        {
            typedef decltype(sizeof(__hash_node_base)) size_type;
            typedef __hash_node_base* __base_ptr;
            typedef __hash_node_base const* __const_base_ptr;
            typedef __base_ptr __bucket;
            typedef __bucket* __buckets_ptr;
            __hash_node_base __root;
            __bucket __singularity = nullptr;
            size_type __element_count = 0UL;
            size_type __bucket_count = 1UL;
            size_type __after_root_idx = 0UL;
            __buckets_ptr __my_buckets = std::addressof(__singularity);            
            float __max_load{ 1.0F };
            constexpr __hashtable_base() = default;
            constexpr bool __is_singularity() const noexcept { return __my_buckets == std::addressof(__singularity); }
            constexpr float __load(size_type added) const noexcept { return (__element_count + added) / double(__bucket_count); }
            constexpr float __current_load() const noexcept { return __load(0); }
            constexpr bool __need_rehash(size_type added) const noexcept { return __is_singularity() || __load(added) >= __max_load; }
            constexpr size_type __next_bucket_ct(size_type added) const noexcept { return __bucket_count * (1UL + static_cast<unsigned int>(__builtin_ceilf(__load(added) / __max_load))); }
            constexpr __buckets_ptr __allocate_buckets(size_type n) { __buckets_ptr result = std::allocator<__bucket>().allocate(n); if(is_constant_evaluated()) { for(size_t i = 0; i < n; i++) { construct_at(addressof(result[i])); } } return result; }
            constexpr size_type __size() const noexcept { return __element_count; }
            constexpr size_type __range(size_type idx) const noexcept { return idx % __bucket_count; }
            constexpr size_type __range(size_type idx, size_type max) const noexcept { if(!max) return 0UL; return idx % max; }
            constexpr void __reset() noexcept { __root.__next = nullptr; __after_root_idx = 0UL; __element_count = 0UL; __bucket_count = 1UL; __singularity = nullptr; __my_buckets = std::addressof(__singularity); }
            constexpr void __deallocate_buckets() { if(!__builtin_expect(__is_singularity(), false)) { std::allocator<__bucket>().deallocate(__my_buckets, __bucket_count); } }
            constexpr void __clear() { __deallocate_buckets(); __reset(); }
            constexpr void __init_buckets(size_type n) { if(__builtin_expect(n < 2, false)) { __my_buckets = std::addressof(__singularity); __singularity = nullptr; } else { __my_buckets = __allocate_buckets(n); } }
            constexpr void __insert_at(__buckets_ptr buckets, size_type idx, __base_ptr n) 
            { 
                if(buckets[idx]) { n->__next = buckets[idx]->__next; buckets[idx]->__next = n; }
                else
                {
                    n->__next = __root.__next;
                    __root.__next = n;
                    if(n->__next) { buckets[__after_root_idx] = n; }
                    __after_root_idx = idx;
                    buckets[idx] = std::addressof(__root);
                }
            }
            constexpr void __remove_first_at(__buckets_ptr buckets, size_type idx, __base_ptr n_next, size_type next_bucket)
            {
                if(!n_next) { buckets[idx] = nullptr; }
                else if(idx != next_bucket) 
                { 
                    buckets[next_bucket] = buckets[idx]; 
                    buckets[idx] = nullptr; 
                    if(idx == __after_root_idx) __after_root_idx = next_bucket;
                }
            }
            constexpr void __insert_at(size_type idx, __base_ptr n) { __insert_at(__my_buckets, idx, n); }
            constexpr void __remove_first_at(size_type idx, __base_ptr n_next, size_type next_bucket) { __remove_first_at(__my_buckets, idx, n_next, next_bucket); }
            constexpr __hashtable_base(size_type ct) : __bucket_count(ct) { __init_buckets(ct); }
            constexpr ~__hashtable_base() { __deallocate_buckets(); }
        };
        template<typename VT>
        struct __hash_node : __hash_node_base
        {
            ::__impl::__aligned_buffer<VT> __data;
            typedef __hash_node<VT>* __link;
            typedef __hash_node<VT> const* __const_link;
            constexpr __hash_node() noexcept = default;
            constexpr __link __get_next() noexcept { return static_cast<__link>(this->__next); }
            constexpr __const_link __get_next() const noexcept { return static_cast<__const_link>(this->__next); }
            constexpr VT* __ptr() noexcept { return __data.__get_ptr(); }
            constexpr VT const* __ptr() const noexcept { return __data.__get_ptr(); }
            constexpr VT& __ref() noexcept { return *__ptr(); }
            constexpr VT const& __ref() const noexcept { return *__ptr(); }
        };
        template<typename T>
        struct __hashtable_iterator
        {
            typedef T value_type;
            typedef add_lvalue_reference_t<T> reference;
            typedef add_pointer_t<T> pointer;
            typedef ptrdiff_t difference_type;
            typedef forward_iterator_tag iterator_concept;
        private:    
            typedef __hashtable_iterator<T> __iterator_type;
            typedef __hash_node_base* __base_ptr;
            typedef __hash_node<T>* __node_ptr;
            __base_ptr __my_node;
        public:
            constexpr __hashtable_iterator() noexcept : __my_node() {}
            constexpr explicit __hashtable_iterator(__base_ptr n) noexcept : __my_node(n) {}
            extension constexpr __node_ptr get_node() const noexcept { return static_cast<__node_ptr>(__my_node); }
            constexpr pointer base() const noexcept { return get_node()->__ptr(); }
            constexpr pointer operator->() const noexcept { return base(); }
            constexpr reference operator*() const noexcept { return *base(); }
            constexpr __iterator_type& operator++() { __my_node = __my_node->__next; return *this; }
            constexpr __iterator_type operator++(int) { __iterator_type that(__my_node); __my_node = __my_node->__next; return that; }
            friend constexpr bool operator==(__iterator_type const& __this, __iterator_type const& __that) noexcept { return __this.__my_node == __that.__my_node; }
            constexpr operator bool() const noexcept { return __my_node != nullptr; }
        };
        template<typename T>
        struct __hashtable_const_iterator
        {
            typedef T value_type;
            typedef add_lvalue_reference_t<add_const_t<T>> reference;
            typedef add_pointer_t<add_const_t<T>> pointer;
            typedef ptrdiff_t difference_type;
            typedef forward_iterator_tag iterator_concept;
        private:            
			typedef __hashtable_iterator<T> __iterator_type;
            typedef __hashtable_const_iterator<T> __const_iterator_type;
            typedef __hash_node_base* __base_ptr;
            typedef __hash_node<T> const* __node_ptr;
            __base_ptr __my_node;
        public:
            constexpr __hashtable_const_iterator() noexcept : __my_node() {}
            constexpr explicit __hashtable_const_iterator(__base_ptr n) noexcept : __my_node(n) {}
            constexpr __hashtable_const_iterator(__iterator_type const& i) noexcept : __my_node(i.__my_node) {}
            extension constexpr __node_ptr get_node() const noexcept { return static_cast<__node_ptr>(__my_node); }
            constexpr pointer base() const noexcept { return get_node()->__ptr(); }
            constexpr pointer operator->() const noexcept { return base(); }
            constexpr reference operator*() const noexcept { return *base(); }
            constexpr __const_iterator_type& operator++() { __my_node = __my_node->__next; return *this; }
            constexpr __const_iterator_type operator++(int) { __const_iterator_type that(__my_node); __my_node = __my_node->__next; return that; }
            friend constexpr bool operator==(__hashtable_const_iterator<T> const& __this, __hashtable_const_iterator<T> const& __that) noexcept { return __this.__my_node == __that.__my_node; }
            constexpr operator bool() const noexcept { return __my_node != nullptr; }
        };
        template<typename KT, typename VT, __detail::__hash_ftor<KT> HT, __detail::__key_extract<KT, VT> XT, __detail::__predicate<KT> ET, allocator_object<__hash_node<VT>> AT>
        struct __hashtable : __hashtable_base
        {
            typedef KT key_type;
            typedef VT value_type;
            typedef AT allocator_type;
            typedef __hashtable_iterator<value_type> iterator;
            typedef __hashtable_const_iterator<value_type> const_iterator;
            using typename __hashtable_base::size_type;
            using typename __hashtable_base::__bucket;
            using typename __hashtable_base::__base_ptr;
            using typename __hashtable_base::__const_base_ptr;
            using typename __hashtable_base::__buckets_ptr;
            typedef __hash_node<value_type> __node_type;
            typedef __node_type* __node_ptr;
            typedef __node_type const* __const_node_ptr;
            typedef HT __hash_fn;
            typedef XT __extr_fn;
            typedef ET __equals_fn;
            allocator_type __alloc{};
            constexpr static key_type const& __key_of(__const_node_ptr n) { return __extr_fn()(n->__ref()); }
            constexpr static bool __is_equal(key_type const& k1, key_type const& k2) { return __equals_fn()(k1, k2); }
            static size_type __hash_code(__const_node_ptr n) const { return __hash_fn()(__extr_fn()(n->__ref())); }
            static size_type __hash_code(key_type const& k) const { return __hash_fn()(k); }
            static size_type __hash_code(value_type const& v) const { return __hash_fn()(__extr_fn()(v)); }
            constexpr __node_ptr __allocate_node() { return construct_at(__alloc.allocate(1UL)); }
            template<typename ... Args> requires constructible_from<value_type, Args...> constexpr __node_ptr __create_node(Args&& ... args) { __node_ptr n = __allocate_node(); construct_at(n->__ptr(), forward<Args>(args)...); return n; }
            constexpr void __destroy_node(__node_ptr n) { __alloc.deallocate(n, 1UL); }
            constexpr __node_ptr __begin() noexcept { return static_cast<__node_ptr>(this->__root.__next); }
            constexpr __const_node_ptr __begin() const noexcept { return static_cast<__const_node_ptr>(this->__root.__next); }
            constexpr __node_ptr __advance_chain(__base_ptr chain) noexcept { return chain ? static_cast<__node_ptr>(chain->__next) : nullptr; }
            constexpr __const_node_ptr __advance_chain(__const_base_ptr chain) const noexcept { return chain ? static_cast<__const_node_ptr>(chain->__next) : nullptr; }
            size_type __index(__const_node_ptr n) const { return this->__range(__hash_code(n)); }
            size_type __index(key_type const& k) const { return this->__range(__hash_code(k)); }
            size_type __index(value_type const& v) const { return this->__range(__hash_code(v)); }
            __base_ptr __find_before(key_type const& what) { size_type idx = __index(what); __base_ptr prev = this->__my_buckets[idx]; for(__node_ptr n = __advance_chain(prev); n && idx == __index(n); prev = n, n = n->__get_next()) { if(__is_equal(what, __key_of(n))) return prev; } return nullptr; }
            __const_base_ptr __find_before(key_type const& what) const { size_type idx = __index(what); __const_base_ptr prev = this->__my_buckets[idx]; for(__const_node_ptr n = __advance_chain(prev); n && idx == __index(n); prev = n, n = n->__get_next()) { if(__is_equal(what, __key_of(n))) return prev; } return nullptr; }
            __base_ptr __get_before(size_type idx, __const_node_ptr n) { __base_ptr prev = this->__my_buckets[idx]; for(__node_ptr p = __advance_chain(prev); p; prev = p, p = p->__get_next()) { if(n == p) return prev; } return nullptr; }
            __node_ptr __find(key_type const& what) { return __advance_chain(__find_before(what)); }
            __const_node_ptr __find(key_type const& what) const { return __advance_chain(__find_before(what)); }
            void __run_rehash(size_type target_count);
            iterator __insert_node(size_type hash, __base_ptr n, size_type added) { if(this->__need_rehash(added)) { __run_rehash(this->__next_bucket_ct(added)); } this->__insert_at(this->__range(hash), n); this->__element_count++; return iterator(n); }
            iterator __erase_node(size_type idx, __base_ptr prev, __node_ptr n) { if(prev == this->__my_buckets[idx]) { this->__remove_first_at(idx, n->__next, n->__next ? __index(n->__get_next()) : 0UL); } else if(n->__next) { size_t subs_idx = __index(n->__get_next()); if(idx != subs_idx) this->__my_buckets[subs_idx] = prev; } prev->__next = n->__next; iterator result(n->__next); __destroy_node(n); this->__element_count--; return result; }
            pair<iterator, bool> __insert_node(__node_ptr n, size_type added = 1UL) { if(__node_ptr p = __find(__key_of(n))) { __destroy_node(n); return make_pair(p, false); } return make_pair(__insert_node(__hash_code(n), n, added), true); }
            iterator __insert(__node_ptr n, size_type added = 1UL) { return __insert_node(__hash_code(n), n, added); }
            pair<iterator, bool> __insert(value_type const& val) requires copy_constructible<value_type> { return __insert_node(__create_node(val)); }
            pair<iterator, bool> __insert(value_type&& val) requires move_constructible<value_type> { return __insert_node(__create_node(move(val))); }
            template<typename ... Args> requires constructible_from<value_type, Args...> constexpr pair<iterator, bool> __emplace(Args&& ... args) { return __insert_node(__create_node(forward<Args>(args)...)); }
            iterator __erase(const_iterator what) { __base_ptr prev = __get_before(what.get_node()); __node_ptr n = __advance_chain(prev); return __erase_node(__index(n), prev, n); }
            size_type __erase(key_type const& what) { __base_ptr prev = __find_before(what); if(__node_ptr n = __advance_chain(prev)) { __erase_node(__index(n), prev, n); return 1UL; } return 0UL; }
            float __get_max_load() const noexcept { return this->__max_load; }
            void __set_max_load(float val) noexcept { this->__max_load = val; }
            constexpr __hashtable() = default;
            constexpr __hashtable(size_type ct) : __hashtable_base(ct) {}
            constexpr ~__hashtable() = default;
            constexpr size_type __size() const noexcept { return this->__element_count; }
            // TODO: void __run_rehash_multi(size_type target_count);
            // TODO: iterator __insert_node_multi(__node_ptr n, size_type added);
        };
        template <typename KT, typename VT, __detail::__hash_ftor<KT> HT, __detail::__key_extract<KT, VT> XT, __detail::__predicate<KT> ET, allocator_object<__hash_node<VT>> AT>
        void __hashtable<KT, VT, HT, XT, ET, AT>::__run_rehash(size_type target_count) 
        {
            if(__builtin_expect(target_count < 2UL, false)) target_count = 2UL;
            if(this->__is_singularity()) { this->__init_buckets(target_count); } 
            else
            { 
                __buckets_ptr nbkts = this->__allocate_buckets(target_count); 
                __node_ptr orig = __begin();
                this->__after_root_idx = 0; 
                this->__root.__next = nullptr;
                for(__node_ptr p = orig; p; p = p->__get_next()) this->__insert_at(nbkts, this->__range(__hash_code(p), target_count), p);
                this->__deallocate_buckets();
                this->__my_buckets = nbkts;
            }
            this->__bucket_count = target_count;
        }
    }
    template<typename KT, typename MT, __detail::__hash_ftor<KT> HT, __detail::__predicate<KT> ET = equal_to<void>, allocator_object<pair<const KT, MT>> AT = std::allocator<pair<const KT, MT>>>
    class hash_map : __impl::__hashtable<KT, pair<const KT, MT>, HT, __detail::__pair_key_extract<const KT, MT>, ET, AT>
    {
        using __base = __impl::__hashtable<KT, pair<const KT, MT>, HT, __detail::__pair_key_extract<const KT, MT>, ET, AT>;
        using typename __base::__node_ptr;
        using typename __base::__const_node_ptr;
    public:
        using typename __base::size_type;
        using typename __base::key_type;
        using typename __base::value_type;
        typedef MT mapped_type;
        using typename __base::allocator_type;
        using typename __base::iterator;
        using typename __base::const_iterator;
    private:
        pair<iterator, bool> __insert_or_assign(__node_ptr n) { __node_ptr p = this->__find(n->__ref().first); if(p) { p->__ref().second = move(n->__ref().second); this->__destroy_node(n); return make_pair(iterator(p), false); } return make_pair(iterator(n), true); }
    public:
        constexpr hash_map() = default;
        constexpr hash_map(size_type bucket_ct) : __base(bucket_ct) {}
        size_type size() const noexcept { return this->__size(); }
        iterator find(key_type const& k) { return iterator(this->__find(k)); }
        const_iterator find(key_type const& k) const { return const_iterator(this->__find(k)); }
        iterator begin() { return iterator(this->__begin()); }
        const_iterator cbegin() const { return const_iterator(this->__begin()); }
        const_iterator begin() const { return cbegin(); }
        iterator end() { return iterator(nullptr); }
        const_iterator cend() const { return const_iterator(nullptr); }
        const_iterator end() const { return const_iterator(nullptr); }
        template<typename ... Args> requires constructible_from<value_type, Args...> constexpr pair<iterator, bool> emplace(Args&& ... args) { return this->__emplace(forward<Args>(args)...); }
        pair<iterator, bool> insert(value_type const& v) requires copy_constructible<value_type> { return this->__insert(v); }
        pair<iterator, bool> insert(value_type && v) requires move_constructible<value_type> { return this->__insert(move(v)); }
        pair<iterator, bool> insert_or_assign(key_type && kt, mapped_type && mt) { return __insert_or_assign(this->__create_node(piecewise_construct, forward_as_tuple(move(kt)), forward_as_tuple(move(mt)))); }
        pair<iterator, bool> insert_or_assign(key_type const& kt, mapped_type && mt) { return __insert_or_assign(this->__create_node(piecewise_construct, tuple<key_type const&>(kt), forward_as_tuple(move(mt)))); }
        mapped_type& operator[](key_type && kt) requires is_default_constructible_v<mapped_type> { __node_ptr n = this->__find(kt); if(!n) { n = this->__insert_node(this->__create_node(piecewise_construct, forward_as_tuple(move(kt)), tuple<>())); } return n->__ref().second; }
        mapped_type& operator[](key_type const& kt) requires is_default_constructible_v<mapped_type> { __node_ptr n = this->__find(kt); if(!n) { n = this->__insert_node(this->__create_node(piecewise_construct, tuple<key_type const&>(kt), tuple<>())); } return n->__ref().second; }
    };
}
#endif