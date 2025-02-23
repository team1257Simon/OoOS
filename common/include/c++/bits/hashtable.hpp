#ifndef __HASH_TABLE
#define __HASH_TABLE
#include "bits/stl_function.hpp" // __invocable_as
#include "bits/aligned_buffer.hpp"
#include "memory" // construct_at
#include "initializer_list"
#include "bits/iterator_concepts.hpp"
#include "bits/functional_compare.hpp"
#include "tuple"
#include "bits/hashtable_policy.hpp"
namespace std
{
    namespace __detail
    {
        template<typename FT, typename KT> concept __hash_functor = is_default_constructible_v<FT> && requires(FT ft, KT kt) { { ft(kt) } -> std::unsigned_integral; };
        template<typename FT, typename VT, typename KT> concept __key_functor = is_default_constructible_v<FT> && requires(FT const& ft, VT const& vt) { { ft(vt) } -> std::convertible_to<KT>; };
        template<typename FT, typename KT> concept __predicate = requires(FT ft, KT kt, KT kt2) { { ft(kt, kt2) } -> __boolean_testable; } && is_default_constructible_v<FT>;
    }
    template<typename KT, __detail::__hash_functor<KT> HT> struct __hash_traits
    {
        typedef HT __hash_type;
        typedef __impl::__modulus_hash_range __range_type;
        typedef __impl::__prime_number_policy __policy_type;
        constexpr static size_t __small_size_threshold() { return 0; }
    };
    template<typename PT> struct __policy_state_guard
    {
        typedef typename PT::__state_type __state;
        PT* __guarded_obj;
        __state __remembered_state;
        __policy_state_guard(PT& __obj) : __guarded_obj{ &__obj }, __remembered_state{ __obj.__state() } {}
        ~__policy_state_guard() { if(__guarded_obj) __guarded_obj->__recall_state(__remembered_state); }
    };
    struct __hash_node_base
    {
        __hash_node_base* __next;
        __hash_node_base() noexcept : __next{} {}
        __hash_node_base(__hash_node_base* nxt) noexcept : __next { nxt } {}
    };
    template<typename VT> struct __hash_node_value
    {
        ::__impl::__aligned_buffer<VT> __my_data;
        [[gnu::always_inline]] VT* __get_ptr() { return __my_data.__get_ptr(); }
        [[gnu::always_inline]] const VT* __get_ptr() const { return __my_data.__get_ptr(); }
        [[gnu::always_inline]] VT& __get_ref() { return *__get_ptr(); }
        [[gnu::always_inline]] const VT& __get_ref() const { return *__get_ptr(); }
    };
    template<typename VT> struct __hash_node : public __hash_node_base, public __hash_node_value<VT> { __hash_node* __get_next() const noexcept { return static_cast<__hash_node*>(this->__next); } };
    template<typename VT> struct __hash_node_iterator_base
    {
        using __node_type = __hash_node<VT>;
        __node_type* __cur;
        __hash_node_iterator_base() : __cur(nullptr) {}
        __hash_node_iterator_base(__node_type* n) : __cur{ n } {}
        void __increment() { __cur = __cur->__get_next(); }
        friend bool operator==(__hash_node_iterator_base const& __this, __hash_node_iterator_base const& __that) noexcept { return __this.__cur == __that.__cur; }
    };
    template<typename VT> 
    struct __hash_node_iterator : __hash_node_iterator_base<VT>
    {
        using __base = __hash_node_iterator_base<VT>;
        using typename __base::__node_type;
        typedef VT value_type;
        typedef ptrdiff_t difference_type;
        typedef forward_iterator_tag iterator_category;
        typedef value_type* pointer;
        typedef value_type& reference;
        __hash_node_iterator() = default;
        explicit __hash_node_iterator(__node_type* n) : __base{ n } {}
        pointer base() const noexcept { return this->__cur->__get_ptr(); }
        reference operator*() const noexcept { return this->__cur->__get_ref(); }
        pointer operator->() const noexcept { return this->base(); }
        __hash_node_iterator& operator++() noexcept { this->__increment(); return this; }
        __hash_node_iterator operator++(int) noexcept { __hash_node_iterator that(*this); this->__increment(); return that; }
    };
    template<typename VT> 
    struct __hash_node_const_iterator : __hash_node_iterator_base<VT>
    {
        using __base = __hash_node_iterator_base<VT>;
        using typename __base::__node_type;
        typedef VT value_type;
        typedef ptrdiff_t difference_type;
        typedef forward_iterator_tag iterator_category;
        typedef value_type const* pointer;
        typedef value_type const& reference;
        __hash_node_const_iterator() = default;
        explicit __hash_node_const_iterator(__node_type* n) : __base{ n } {}
        __hash_node_const_iterator(__hash_node_iterator<VT> const& that) : __base{ that.__cur } {}
        pointer base() const noexcept { return this->__cur->__get_ptr(); }
        reference operator*() const noexcept { return this->__cur->__get_ref(); }
        pointer operator->() const noexcept { return this->base(); }
        __hash_node_const_iterator& operator++() noexcept { this->__increment(); return this; }
        __hash_node_const_iterator operator++(int) noexcept { __hash_node_iterator that(*this); this->__increment(); return that; }
    };
    template<typename KT, typename VT, allocator_object<VT> AT>
    struct __hashtable_alloc
    {
        typedef KT __key_type;
        typedef VT __value_type;
        typedef size_t __size_type;
        typedef ptrdiff_t __difference_type;
        typedef __hash_node_base* __node_base_ptr;
        typedef __node_base_ptr* __buckets_ptr;
        typedef __hash_node<VT>* __node_ptr;
        typedef __hash_node<VT> const* __node_const_ptr;
        typedef __alloc_rebind<AT, __hash_node_base*> __bucket_ptr_alloc;
        typedef __alloc_rebind<AT, __hash_node<VT>> __node_alloc;
        __bucket_ptr_alloc __my_bucket_alloc;
        __node_alloc __my_node_alloc;
        __buckets_ptr __allocate_buckets_base(__size_type n) { __buckets_ptr result = __my_bucket_alloc.allocate(n); for(size_t i = 0; i < n; i++) { construct_at(addressof(result[i])); } return result; }
        template<typename ... Args> requires constructible_from<VT, Args...> __node_ptr __construct_node(Args&& ... args) { __node_ptr n = __my_node_alloc.allocate(1UL); construct_at(n->__get_ptr(), forward<Args>(args)...); return n; }
        void __deallocate_buckets_base(__buckets_ptr ptr, __size_type n) { __my_bucket_alloc.deallocate(ptr, n); }
        void __deallocate_node(__node_ptr n) { __my_node_alloc.deallocate(n, 1); }
        __hashtable_alloc() = default;
    };
    template<typename KT, typename VT, allocator_object<VT> AT> 
    struct __scoped_hash_node
    {
        typedef typename __hashtable_alloc<KT, VT, AT>::__node_ptr __node_ptr;
        __node_ptr __my_node;
        __hashtable_alloc<KT, VT, AT>* __alloc;
        __scoped_hash_node(__node_ptr n, __hashtable_alloc<KT, VT, AT>* a) : __my_node{ n }, __alloc{ a } {}
        __scoped_hash_node(__scoped_hash_node const&) = delete;
        __scoped_hash_node& operator=(__scoped_hash_node const&) = delete;
        template<typename ... Args> requires constructible_from<VT, Args...> __scoped_hash_node(__hashtable_alloc<KT, VT, AT>* a, Args&& ... args) : __my_node{ a->__construct_node(forward<Args>(args)...) }, __alloc{ a } {}
        ~__scoped_hash_node() { if(__my_node) __alloc->__deallocate_node(__my_node); }
    };
    template<typename KT, typename VT, __detail::__hash_functor<KT> HT>
    struct __hashtable_calc
    {
        typedef typename __hash_traits<KT, HT>::__hash_type __hash_fn;        
        typedef typename __hash_traits<KT, HT>::__range_type __range_fn;
        typedef size_t __size_type;
        __hash_fn __hash;
        __range_fn __range;
        __size_type __range_check(__size_type hash, __size_type count) const noexcept { return __range(hash, count); }
        template<convertible_to<KT> LT> __size_type __hash_code(LT const& __key) const noexcept { return __hash(__key); }
        __hashtable_calc() = default;
        __hashtable_calc(__hash_fn const& h) : __hash{ h }, __range{} {}
    };
    template<typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
    struct __hashtable_base : public __hashtable_alloc<KT, VT, AT>, public __hashtable_calc<KT, VT, HT>
    {
        typedef __hashtable_alloc<KT, VT, AT> __alloc_base;
        typedef __hashtable_calc<KT, VT, HT> __calc_base;
        using typename __alloc_base::__key_type;
        using typename __alloc_base::__value_type;
        using typename __alloc_base::__size_type;
        using typename __alloc_base::__difference_type;
        using typename __alloc_base::__node_base_ptr;
        using typename __alloc_base::__buckets_ptr;
        using typename __alloc_base::__node_ptr;
        using typename __alloc_base::__node_const_ptr;
        using typename __calc_base::__hash_fn;        
        using typename __calc_base::__range_fn;         
        typedef XT __key_fn;
        typedef ET __equal_fn;
        typedef typename __hash_traits<KT, HT>::__policy_type __policy_type;
        typedef __policy_state_guard<__policy_type> __policy_guard;
        typedef __hash_node_iterator<VT> __iterator;
        typedef __hash_node_const_iterator<VT> __const_iterator;
        typedef __scoped_hash_node<KT, VT, AT> __scoped_node;
    private:
        __hash_node_base __root_node;
        __node_base_ptr __singularity = nullptr;
        __size_type __bucket_count = 1;
        __size_type __element_count = 0;
        __policy_type __policy;
        __buckets_ptr __my_buckets = &__singularity;
    public:
        constexpr __size_type __size() const noexcept { return __element_count; }
        constexpr __size_type __num_buckets() const noexcept { return __bucket_count; }
        constexpr static size_t __small_size_threshold() noexcept { return __hash_traits<KT, HT>::__small_size_threshold(); }
        template<convertible_to<KT> LT, convertible_to<KT> JT> bool __check_eq(LT const& __this, JT const& __that) const noexcept { return __equal_fn()(__this, __that); }
        template<convertible_to<VT> WT> __key_type const& __key_of(WT const& wt) const noexcept { return __key_fn()(wt); }
        __node_ptr __begin() const noexcept { return static_cast<__node_ptr>(__root_node.__next); }
        __size_type __range_check(__size_type hash) const noexcept { return this->__calc_base::__range_check(hash, __bucket_count); } 
        template<convertible_to<KT> LT> __size_type __bucket_key_index(LT const& lt, __size_type bucket_count) const noexcept { return this->__calc_base::__range_check(this->__hash_code(lt), bucket_count); }
        template<convertible_to<KT> LT> __size_type __bucket_key_index(LT const& lt) const noexcept { return __bucket_key_index(this->__hash_code(lt), __bucket_count); } 
        template<convertible_to<VT> WT> __size_type __bucket_index(WT const& val, __size_type bucket_count) const noexcept { return __bucket_key_index(__key_of(val), bucket_count); }             
        __size_type __bucket_index(__hash_node<VT> const& node, __size_type bucket_count) const noexcept { return __bucket_index(node.__get_ref(), bucket_count); } 
        template<convertible_to<VT> WT> __size_type __bucket_index(WT const& val) const noexcept { return this->__bucket_index(val, __bucket_count); }
        __size_type __bucket_index(__hash_node<VT> const& node) const noexcept { return this->__bucket_index(node.__get_ref()); }
        void __update_root() { if(__node_ptr p = __begin()) { __my_buckets[__bucket_index(*p)] = &__root_node; } }
        void __update_root(__node_ptr n) { __root_node.__next = n; __update_root(); }
        bool __is_singularity(__buckets_ptr bkts) const { return __builtin_expect(bkts == &__singularity, false); }
        bool __is_singularity() const { return __is_singularity(__my_buckets); }
        __buckets_ptr __allocate_buckets(__size_type n) { if(__builtin_expect(__is_singularity(), false)) { __singularity = nullptr; return &__singularity; } return this->__allocate_buckets_base(n); }
        void __deallocate_buckets(__buckets_ptr ptr, __size_type n) { if(__builtin_expect(__is_singularity(ptr), false)) return; this->__deallocate_buckets_base(ptr, n); }
        void __deallocate_buckets() { __deallocate_buckets(__my_buckets, __bucket_count); }
        __node_ptr __bucket_at(__size_type index) { __node_base_ptr n = __my_buckets[index]; return n ? static_cast<__node_ptr>(n->__next) : nullptr; }
        template<convertible_to<KT> LT> __node_base_ptr __find_before(__size_type idx, LT const& lt);
        template<convertible_to<KT> LT> __node_base_ptr __find_before(LT const& lt);
        template<convertible_to<KT> LT> __node_ptr __find_node(__size_type idx, LT const& lt) { if(__node_base_ptr bp = __find_before(idx, lt)) { return static_cast<__node_ptr>(bp->__next); } return nullptr; }
        template<convertible_to<KT> LT> __node_ptr __find_node(LT const& lt) { if(__node_base_ptr bp = __find_before(lt)) { return static_cast<__node_ptr>(bp->__next); } return nullptr; }
        template<convertible_to<KT> LT> __iterator __find(LT const& lt) { if(__element_count < __small_size_threshold()) { return __iterator(__find_node(lt)); } else return __iterator(__find_node(__bucket_key_index(lt), lt)); }
        void __insert_node_at(__size_type index, __node_ptr n);
        void __remove_first_node(__size_type index, __node_ptr next_node, __size_type next_bucket_index) { if(!next_node) __my_buckets[index] = nullptr; else if(next_bucket_index != index) { __my_buckets[next_bucket_index] = __my_buckets[index]; __my_buckets[index] = nullptr; } }
        __node_base_ptr __get_previous(__size_type bucket, __node_ptr node) { for(__node_base_ptr prev = __my_buckets[bucket]; prev; prev = prev->__next) { if(node == prev->__next) return prev; } return nullptr; }
        void __run_rehash(__size_type bucket_ct);
        __iterator __insert_unique_node(__size_type hash, __node_ptr node, __size_type total_elems = 1);
        template<typename ... Args> requires constructible_from<VT, Args...> pair<__iterator, bool> __emplace_unique(Args&& ... args);
        template<convertible_to<VT> WT> requires move_constructible<VT> pair<__iterator, bool> __insert_unique(WT&& wt);
        template<convertible_to<VT> WT> requires copy_constructible<VT> pair<__iterator, bool> __insert_unique(WT const& wt);
        __iterator __erase_node(__size_type index, __node_base_ptr prev, __node_ptr n);
        template<convertible_to<KT> LT> __size_type __erase_unique(LT const& lt);
        void __reset() { __policy.__reset_state(); __bucket_count = 1; __singularity = nullptr; __my_buckets = &__singularity; __root_node.__next = nullptr; __element_count = 0; }
        void __deallocate_nodes(__node_ptr from) { __node_ptr n = from; __node_ptr p; while(n) { p = n; n = n->__get_next(); this->__deallocate_node(p); } }
        void __clear() { __deallocate_nodes(__begin()); __deallocate_buckets(); __reset(); }
        void __move_assign(__hashtable_base&& that) { if(__builtin_expect(addressof(that) == this, false)) return; __deallocate_nodes(__begin()); __deallocate_buckets(); this->__policy = that.__policy; if(that.__is_singularity()) { this->__singularity = that.__singularity; __my_buckets = &__singularity; } else { __my_buckets = that.__my_buckets; } this->__bucket_count = that.__bucket_count; this->__root_node.__next = that.__root_node.__next; this->__element_count = that.__element_count; this->__update_root(); that.__reset(); }
        __hashtable_base() : __alloc_base{}, __calc_base{} { __bucket_count = __policy.__next_bucket(__bucket_count); __my_buckets = this->__allocate_buckets(__bucket_count); }
        ~__hashtable_base() { __clear(); }
        __hashtable_base(__hash_fn const& h) : __alloc_base{}, __calc_base{ h } {}
    };
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
    void __hashtable_base<KT, VT, HT, XT, ET, AT>::__insert_node_at(__size_type index, __node_ptr n)
    {
        if(__my_buckets[index]) { n->__next = __my_buckets[index]->__next; __my_buckets[index]->__next = n; }
        else
        {
            n->__next = __root_node.__next;
            __root_node.__next = n;
            if(n->__next) __my_buckets[__bucket_index(n->__get_next()->__get_ref())] = n;
            __my_buckets[index] = &__root_node;
        }
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
    template <std::convertible_to<KT> LT>
    typename __hashtable_base<KT, VT, HT, XT, ET, AT>::__node_base_ptr __hashtable_base<KT, VT, HT, XT, ET, AT>::__find_before(__size_type idx, LT const &lt)
    {
        __node_base_ptr prev = __my_buckets[idx];
        if(!prev) return nullptr;
        for(__node_ptr p = static_cast<__node_ptr>(prev->__next); p; p = p->__get_next()) { if(this->__check_eq(lt, __key_of(p->__get_ref()))) return prev; if(!p->__next || __bucket_index(p->__get_next()->__get_ref()) != idx) break; prev = p; }
        return nullptr;
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
    template <std::convertible_to<KT> LT>
    typename __hashtable_base<KT, VT, HT, XT, ET, AT>::__node_base_ptr __hashtable_base<KT, VT, HT, XT, ET, AT>::__find_before(LT const &lt)
    {
        __node_base_ptr prev = &__root_node;
        for(__node_ptr p = static_cast<__node_ptr>(prev->__next); p; p = p->__get_next()) { if(this->__check_eq(lt, __key_of(p->__get_ref()))) return prev; prev = p; }
        return nullptr;
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
    void __hashtable_base<KT, VT, HT, XT, ET, AT>::__run_rehash(__size_type bucket_ct)
    {
        __buckets_ptr __new_buckets = __allocate_buckets(bucket_ct);
        __node_ptr p = __begin();
        __root_node.__next = nullptr;
        size_t root_bucket = 0;
        while(p)
        {
            __node_ptr nxt = p->__get_next();
            size_t bkt = __bucket_index(p->__get_ref(), bucket_ct);
            if(__new_buckets[bkt]) { p->__next = __new_buckets[bkt]->__next; __new_buckets[bkt]->__next = p; }
            else
            {
                p->__next = __root_node.__next;
                __root_node.__next = p;
                __new_buckets[bkt] = &__root_node;
                if(p->__next) __new_buckets[root_bucket] = p;
                root_bucket = bkt;
            }
            p = nxt;
        }
        __deallocate_buckets();
        __bucket_count = bucket_ct;
        __my_buckets = __new_buckets;
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
    typename __hashtable_base<KT, VT, HT, XT, ET, AT>::__iterator __hashtable_base<KT, VT, HT, XT, ET, AT>::__insert_unique_node(__size_type hash, __node_ptr node, __size_type total_elems)
    {
        __policy_guard guard(__policy);
        pair<bool, size_t> need = __policy.__need_rehash(__bucket_count, __element_count, total_elems);
        if(need.first) __run_rehash(need.second);
        __size_type idx = __range_check(hash);
        guard.__guarded_obj = nullptr;
        __insert_node_at(idx, node);
        ++__element_count;
        return __iterator(node);
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
    template <typename... Args>
    requires constructible_from<VT, Args...>
    pair<typename __hashtable_base<KT, VT, HT, XT, ET, AT>::__iterator, bool> __hashtable_base<KT, VT, HT, XT, ET, AT>::__emplace_unique(Args &&...args)
    {
        using __res = pair<__iterator, bool>;
        __scoped_node n{ this, forward<Args>(args)... };
        __key_type const& k = __key_of(*(n.__my_node));
        __size_type s = __element_count;
        if(s < __small_size_threshold()) { for(__node_ptr i = __begin(); i; i = i->__get_next()) { if(__check_eq(k, __key_of(i->__get_ref()))) return __res{ __iterator(i), false }; } }
        __size_type hash = this->__hash_code(k);
        __size_type idx = __range_check(hash);
        if(s >= __small_size_threshold()) { if(__node_ptr p = __find_node(idx, k)) { return __res{ __iterator(p), false }; } }
        __iterator result = __insert_unique_node(hash, n.__my_node);
        n.__my_node = nullptr;
        return __res{ result, true };
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
    template <convertible_to<VT> WT>
    requires move_constructible<VT>
    pair<typename __hashtable_base<KT, VT, HT, XT, ET, AT>::__iterator, bool> std::__hashtable_base<KT, VT, HT, XT, ET, AT>::__insert_unique(WT&& wt)
    {
        using __res = pair<__iterator, bool>;
        __key_type const& k = __key_of(wt);
        __size_type s = __element_count;
        if(s < __small_size_threshold()) { for(__node_ptr i = __begin(); i; i = i->__get_next()) { if(__check_eq(k, __key_of(i->__get_ref()))) return __res{ __iterator(i), false }; } }
        __size_type hash = this->__hash_code(k);
        __size_type idx = __range_check(hash);
        if(s >= __small_size_threshold()) { if(__node_ptr p = __find_node(idx, k)) { return __res{ __iterator(p), false }; } }
        __scoped_node n{ this->__construct_node(move(wt)), this };
        __iterator result = __insert_unique_node(hash, n.__my_node);
        n.__my_node = nullptr;
        return __res{ result, true };
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
    template <convertible_to<VT> WT>
    requires copy_constructible<VT>
    pair<typename __hashtable_base<KT, VT, HT, XT, ET, AT>::__iterator, bool> std::__hashtable_base<KT, VT, HT, XT, ET, AT>::__insert_unique(WT const& wt)
    {
        using __res = pair<__iterator, bool>;
        __key_type const& k = __key_of(wt);
        __size_type s = __element_count;
        if(s < __small_size_threshold()) { for(__node_ptr i = __begin(); i; i = i->__get_next()) { if(__check_eq(k, __key_of(i->__get_ref()))) return __res{ __iterator(i), false }; } }
        __size_type hash = __hash_code(k);
        __size_type idx = __range_check(hash);
        if(s >= __small_size_threshold()) { if(__node_ptr p = __find_node(idx, k)) { return __res{ __iterator(p), false }; } }
        __scoped_node n{ this->__construct_node(wt), this };
        __iterator result = __insert_unique_node(hash, n.__my_node);
        n.__my_node = nullptr;
        return __res{ result, true };
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
    typename __hashtable_base<KT, VT, HT, XT, ET, AT>::__iterator __hashtable_base<KT, VT, HT, XT, ET, AT>::__erase_node(__size_type idx, __node_base_ptr prev, __node_ptr n)
    {
        if(__my_buckets[idx] == prev) __remove_first_node(idx, n->__get_next(), n->__next ? __bucket_index(*(n->__get_next())) : 0);
        else if(n->__next) { __size_type next = __bucket_index(*(n->__get_next())); if(idx != next) __my_buckets[next] = prev; }
        prev->__next = n->__next;
        __iterator result(n->__get_next());
        this->__deallocate_node(n);
        this->__element_count--;
        return result;
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
    template <convertible_to<KT> LT>
    typename __hashtable_base<KT, VT, HT, XT, ET, AT>::__size_type std::__hashtable_base<KT, VT, HT, XT, ET, AT>::__erase_unique(LT const &lt)
    {
        __node_base_ptr prev;
        __node_ptr n;
        __size_type bkt;
        if(__element_count < __small_size_threshold())
        {
            prev = __find_before(lt);
            if(!prev) return 0;
            n = static_cast<__node_ptr>(prev->__next);
            bkt = __bucket_index(*n);
        }
        else
        {
            bkt = __bucket_key_index(lt);
            prev = __find_before(bkt, lt);
            if(!prev) return 0;
            n = static_cast<__node_ptr>(prev->__next);
        }
        __erase_node(bkt, prev, n);
        return 1;
    }
}
#endif