#ifndef __HASH_TABLE
#define __HASH_TABLE
#include "bits/stl_function.hpp" // __invocable_as
#include "bits/aligned_buffer.hpp"
#include "memory" // construct_at
#include "bits/functional_compare.hpp"
#include "bits/stl_pair.hpp"
#include "bits/hashtable_policy.hpp"
namespace std
{
    namespace __detail
    {
        template<typename FT, typename KT> concept __hash_functor = __invocable_as<FT, size_t, std::decay_t<KT>>::value && is_default_constructible_v<FT>;
        template<typename FT, typename VT, typename KT> concept __key_functor = __invocable_as<FT, KT, std::decay_t<VT>>::value && is_default_constructible_v<FT>;
        template<typename FT, typename KT> concept __predicate = requires(FT ft, KT kt, KT kt2) { { ft(kt, kt2) } -> __boolean_testable; } && is_default_constructible_v<FT>;
        template<typename FT> concept __hash_range_functor = __invocable_as<FT, size_t, size_t, size_t>::value;
        template<typename PT> concept __hash_policy = requires(PT pt, size_t m, size_t n, size_t i) { { pt.max_load_factor() } -> floating_point; { pt.__next_bucket(m) } -> unsigned_integral; { pt.__buckets_for_elements(m) } -> unsigned_integral; { pt.__need_rehash(m, n, i) } -> same_as<pair<bool, size_t>>; typename PT::__state_type; { pt.__state() } -> same_as<typename PT::__state_type>; } && requires(PT pt, typename PT::__state_type s) { pt.__recall_state(s); } && is_default_constructible_v<PT>;
    }
    template<__detail::__hash_policy PT> struct __policy_state_guard
    {
        typedef typename PT::__state_type __state;
        PT* __guarded_obj;
        __state __remembered_state;
        constexpr __policy_state_guard(PT& __obj) : __guarded_obj{ &__obj }, __remembered_state{ __obj.__state() } {}
        constexpr ~__policy_state_guard() { if(__guarded_obj) __guarded_obj->__recall_state(__remembered_state); }
    };
    template<typename VT> struct __hash_node
    {
        ::__impl::__aligned_buffer<VT> __my_data;
        __hash_node<VT>* __next_node; 
        constexpr __hash_node() noexcept : __my_data{}, __next_node{ nullptr } {}
        constexpr VT* __get_ptr() { return __my_data.__get_ptr(); }
        constexpr const VT* __get_ptr() const { return __my_data.__get_ptr(); }
        constexpr VT& __get_ref() { return *__get_ptr(); }
        constexpr const VT& __get_ref() const { return *__get_ptr(); }
    };
    template<typename VT> 
    struct __hash_bucket_t
    {
        typedef __hash_node<VT> __node;
        typedef __node* __node_ptr;
        __node_ptr __start_node;
        __node_ptr __end_node;
        constexpr operator bool() const noexcept { return bool(__start_node); }
        constexpr __hash_bucket_t() noexcept = default;
    };
    template<typename VT> 
    struct __hash_table_iterator
    {
        typedef __hash_node<remove_const_t<VT>> __node;
        typedef __hash_bucket_t<remove_const_t<VT>> __bucket;
        typedef conditional_t<is_const_v<VT>, __node const*, __node*> __node_ptr;
        typedef conditional_t<is_const_v<VT>, __bucket const*, __bucket*> __buckets_ptr;
        typedef VT* __value_ptr;
        typedef VT& __value_ref;
        __node_ptr __my_node;
        __buckets_ptr __target_table;
        size_t __current_idx;
        size_t __table_size;
        constexpr __value_ptr base() const noexcept { return __my_node->__get_ptr(); }
        constexpr __value_ptr operator->() const noexcept { return __my_node->__get_ptr(); }
        constexpr __value_ref operator*() const noexcept { return __my_node->__get_ref(); }
        constexpr __hash_table_iterator& operator++() noexcept { if(__my_node->__next_node) __my_node = __my_node->__next_node; else { __current_idx++; __my_node = __current_idx < __table_size ? __target_table[__current_idx].__start_node : nullptr; } return *this; }
        constexpr __hash_table_iterator operator++(int) noexcept { __hash_table_iterator that{ *this }; ++(*this); return that; }
        friend constexpr bool operator==(__hash_table_iterator<VT> const& __this, __hash_table_iterator<VT> const& __that) noexcept { return __this.__my_node == __that.__my_node; }
    };
    template<typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, __detail::__hash_policy PT, __detail::__hash_range_functor RT, allocator_object<__hash_node<VT>> AT>
    struct __hash_table_base
    {
        typedef KT key_type;
        typedef VT value_type;
        typedef AT allocator_type;
        typedef ET key_equal;
        typedef decltype(sizeof(VT)) size_type;
        typedef HT __hash;
        typedef XT __key_extract;
        typedef PT __policy_type;
        typedef RT __hash_range;
        typedef __hash_bucket_t<VT> __hash_bucket;        
        typedef __hash_node<VT> __node_type;
        typedef __node_type* __node_ptr;
        typedef __node_type const* __node_const_ptr;
        typedef __hash_bucket* __buckets_ptr;
        typedef typename allocator_type::rebind<__hash_bucket> __bucket_alloc;
        typedef __hash_table_iterator<VT> iterator;
        typedef __hash_table_iterator<const VT> const_iterator;
        typedef __policy_state_guard<__policy_type> __policy_guard;
         __policy_type __my_policy;
        size_type __bucket_count;
        size_type __elem_count;
        __hash_bucket __singularity;
        __buckets_ptr __my_buckets;
        __hash __my_hash;
        constexpr static size_type __small_size_thresh() noexcept { return 0UL; /* During testing this will be off; once the structure is verified to work, we'll raise it to approximately the value of the constant that's erased from the O(1) time complexity */ }
        constexpr size_type __get_hash(value_type const& vt) { return __my_hash(__key_extract{}(vt)); }
        constexpr size_type __range_check_at_size(size_type n, size_type l) { return __hash_range{}(n, l); }
        constexpr size_type __bucket_index_at_size(value_type const& vt, size_type nbuckets) { return __range_check_at_size(__get_hash(vt), nbuckets); }
        constexpr size_type __bucket_index_at_size(__node_ptr p, size_type nbuckets) { return __bucket_index_at_size(p->__get_ref(), nbuckets); }
        constexpr __node_ptr __find_in_chain(key_type const& vt, __hash_bucket* start) { for(__node_ptr p = start->__start_node; p; p = p->__next_node) { if(key_equal{}(vt, __key_extract{}(p->__get_ref()))) return p; } return nullptr; }
        constexpr __node_const_ptr __find_in_chain(key_type const& vt, __hash_bucket const* start) { for(__node_const_ptr p = start->__start_node; p; p = p->__next_node) { if(key_equal{}(vt, __key_extract{}(p->__get_ref()))) return p; } return nullptr; }
        constexpr pair<__node_ptr, __node_ptr> __find_before(key_type const& vt, __hash_bucket* start) { for(__node_ptr p = nullptr, q = start->__start_node; q; p = q, q = q->__next_node) { if(key_equal{}(vt, __key_extract{}(q->__get_ref()))) return make_pair(p, q); } return pair<__node_ptr, __node_ptr>(nullptr, nullptr); }
        constexpr pair<__node_ptr, __node_ptr> __find_before(__node_ptr n, __hash_bucket* start) { return __find_before(__key_extract{}(n->__get_ref()), start); }
        constexpr void __insert_at_bucket(__buckets_ptr bptr, __node_ptr n) { n->__next_node = bptr->__start_node; bptr->__start_node = n; if(!bptr->__end_node) { bptr->__end_node = n; } }
        __node_ptr __first() noexcept { return __my_buckets[0].__start_node; }
        __node_const_ptr __first() const noexcept { return __my_buckets[0].__start_node; }
        __node_ptr __last() noexcept { return __my_buckets[__bucket_count - 1].__end_node; }
        __node_const_ptr __last() const noexcept { return __my_buckets[__bucket_count - 1].__end_node; }
        constexpr size_type __range_check(size_type n) { return __range_check_at_size(n, __bucket_count); }
        constexpr size_type __bucket_index(value_type const& vt) { if(__my_buckets == &__singularity) return 0; return __range_check(__get_hash(vt)); }
        constexpr size_type __bucket_index(__node_ptr n) { if(n) return __bucket_index(n->__get_ref()); return __bucket_count; }
        constexpr __buckets_ptr __bucket_at_index(value_type const& vt) { if(__my_buckets == &__singularity) return __my_buckets; return __my_buckets + __bucket_index(vt); }
        constexpr iterator __iterator(__node_ptr np) { return iterator{ np, __my_buckets, __bucket_index(np), __bucket_count }; }
        constexpr const_iterator __const_iterator(__node_const_ptr np) const { return const_iterator{ np, __my_buckets, __bucket_index(np), __bucket_count }; }
        constexpr __buckets_ptr __allocate_buckets(size_type how_many) { if(__builtin_expect(how_many == 1, false)) { __singularity.__start_node = __singularity.__end_node = nullptr; return &__singularity; } return __bucket_alloc{}.allocate(how_many); }
        constexpr void __destroy_node(__node_ptr n) noexcept { allocator_type{}.deallocate(n, 1); }
        constexpr void __deallocate_chain(__buckets_ptr chain_start) noexcept { for(__node_ptr n = chain_start->__start_node, m = n; n; n = m) { m = n->__next_node; __destroy_node(n); } }
        constexpr void __deallocate_buckets(__buckets_ptr bkts, size_type how_many) { if(__builtin_expect(bkts == &__singularity, false)) return; for(size_t i = 0; i < how_many; i++) { __deallocate_chain(__my_buckets + i); } __bucket_alloc{}.deallocate(bkts, how_many); }
        constexpr void __deallocate_buckets() { __deallocate_buckets(__my_buckets, __bucket_count); }
        constexpr void __clear() { __deallocate_buckets(); __elem_count = 0; __my_buckets = __allocate_buckets(__bucket_count); }
        constexpr void __insert_at(size_type index, __node_ptr n) { __insert_at_bucket(__my_buckets + index, n); }
        constexpr void __remove_at(size_type index, __node_ptr prev, __node_ptr n) { if(prev) { prev->__next_node = n->__next_node; if(n == __my_buckets[index].__end_node) __my_buckets[index].__end_node = prev; } else { __my_buckets[index].__start_node = n->__next_node; if(n == __my_buckets[index].__end_node) __my_buckets[index].__end_node = n->__next_node; } }
        template<typename ... Args> requires constructible_from<value_type, Args...> constexpr __node_ptr __construct_node(Args&& ... args) { __node_ptr result = allocator_type{}.allocate(1); construct_at(result->__get_ptr(), forward<Args>(args)...); return result; }
        template<convertible_to<key_type> LT> constexpr __node_ptr __find(LT const& lt) { return __find_in_chain(lt, __bucket_at_index(lt)); }
        template<convertible_to<key_type> LT> constexpr __const_node_ptr __find(LT const& lt) const { return __find_in_chain(lt, __bucket_at_index(lt)); }
        template<convertible_to<key_type> LT> constexpr bool __contains(LT const& lt) const { return bool(__find(lt)); }
        constexpr void __run_rehash(size_type target);
        constexpr void __reserve_at_least(size_type n) { if(__bucket_count < n) { __run_rehash(n); } }        
        constexpr __node_ptr __insert_node(size_type index, __node_ptr node, size_type n_elem = 1);
        template<convertible_to<value_type> WT> constexpr pair<__node_ptr, bool> __insert_unique(WT&& wt);
        template<typename ... Args> constexpr pair<__node_ptr, bool> __emplace_unique(Args&& ... args);
        constexpr void __deep_copy(__hash_table_base const& that) { for(size_type i = 0; i < __bucket_count; i++) { for(__node_ptr n = that.__my_buckets[0].__start_node; n; n = n->__next_node) { __insert_at_bucket(this->__bucket_at_index(n->__get_ref()), n); } } }
        template<convertible_to<key_type> LT> constexpr void __erase_node(LT const& lt) { pair<__node_ptr, __node_ptr> p = __find_before(lt, __bucket_at_index(lt)); if(p.second) { __remove_at(__bucket_index(lt), p.first, p.second); __destroy_node(p.second); __elem_count--; } }
        constexpr void __erase_node(__node_ptr n) { if(n) __erase_node(n->__get_ref()); }
        constexpr iterator __begin() noexcept { return __iterator(__first()); }
        constexpr const_iterator __cbegin() const noexcept { return __const_iterator(__first()); }
        constexpr iterator __end() noexcept { return __iterator(nullptr); }
        constexpr const_iterator __cend() const noexcept { return __const_iterator(nullptr); }
        constexpr __hash_table_base() noexcept : __my_policy{}, __bucket_count{ 1UL }, __elem_count{ 0 }, __singularity{}, __my_buckets{ &__singularity }, __my_hash{} {}
        constexpr __hash_table_base(size_type init_count) : __my_policy{}, __bucket_count{ init_count }, __elem_count{ 0 }, __singularity{}, __my_buckets{ __allocate_buckets(init_count) }, __my_hash{} {}
        template<typename ... Args> requires constructible_from<__policy_type, Args...> constexpr __hash_table_base(size_type init_count, Args&& ... args) : __my_policy{ forward<Args>(args)... }, __bucket_count{ init_count }, __elem_count{ 0 }, __singularity{}, __my_buckets{ __allocate_buckets(init_count) }, __my_hash{} {}
        constexpr __hash_table_base(__hash_table_base const& that) : __my_policy{ that.__my_policy }, __bucket_count{ that.__bucket_count }, __elem_count{ that.__elem_count }, __singularity{}, __my_buckets{ this->__allocate_buckets(that.__bucket_count) }, __my_hash{} { this->__deep_copy(that); }
        constexpr __hash_table_base& operator=(__hash_table_base const& that) { this->__deallocate_buckets(); this->__bucket_count = that.__bucket_count; this->__elem_count = that.__elem_count; this->__my_policy = that.__my_policy; this->__deep_copy(that); return *this; }
        constexpr __hash_table_base(__hash_table_base&&) = default;
        constexpr __hash_table_base& operator=(__hash_table_base&&) = default;
        constexpr ~__hash_table_base() { __deallocate_buckets(); }
    };
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, __detail::__hash_policy PT, __detail::__hash_range_functor RT, allocator_object<__hash_node<VT>> AT>
    constexpr void __hash_table_base<KT, VT, HT, XT, ET, PT, RT, AT>::__run_rehash(size_type target)
    {
        __buckets_ptr nbuck = __allocate_buckets(target);
        for(size_type i = 0; i < __bucket_count; i++) { for(__node_ptr n = __my_buckets[i].__start_node; n; n = n->__next_node) { __insert_at_bucket(nbuck + __bucket_index_at_size(n, target), n); } }
        __deallocate_buckets();
        __my_buckets = nbuck;
        __bucket_count = target;
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, __detail::__hash_policy PT, __detail::__hash_range_functor RT, allocator_object<__hash_node<VT>> AT>
    constexpr typename __hash_table_base<KT, VT, HT, XT, ET, PT, RT, AT>::__node_ptr __hash_table_base<KT, VT, HT, XT, ET, PT, RT, AT>::__insert_node(size_type index, __node_ptr node, size_type n_elem)
    {   
        __policy_guard g{ __my_policy };
        pair<bool, size_t> __do_rehash = __my_policy.__need_rehash(__bucket_count, __elem_count, n_elem);
        if(__do_rehash.first)
        {
            __run_rehash(__do_rehash.second);
            index = __bucket_index(node);
        }
        g.__guarded_obj = nullptr;
        __insert_at(index, node);
        __elem_count++;
        return node;
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, __detail::__hash_policy PT, __detail::__hash_range_functor RT, allocator_object<__hash_node<VT>> AT>
    template <convertible_to<VT> WT>
    constexpr pair<typename __hash_table_base<KT, VT, HT, XT, ET, PT, RT, AT>::__node_ptr, bool> __hash_table_base<KT, VT, HT, XT, ET, PT, RT, AT>::__insert_unique(WT&& wt)
    {
        if(__elem_count > __small_size_thresh()) { if(__node_ptr n = __find(lt)) { return make_pair(n, false); } }
        else if(__node_ptr n = __find_in_chain(lt, &__singularity)) { return make_pair(n, false); }
        return __insert_node(__bucket_index(lt), __construct_node(forward<LT>(wt)));
    }
    template <typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, __detail::__hash_policy PT, __detail::__hash_range_functor RT, allocator_object<__hash_node<VT>> AT>
    template <typename... Args>
    constexpr pair<typename __hash_table_base<KT, VT, HT, XT, ET, PT, RT, AT>::__node_ptr, bool> __hash_table_base<KT, VT, HT, XT, ET, PT, RT, AT>::__emplace_unique(Args&& ... args)
    {
        __node_ptr n = __construct_node(forward<Args>(args)...);
        if(__elem_count > __small_size_thresh()) { if(__node_ptr m = __find(n->__get_ref())) { __destroy_node(n); return make_pair(m, false); } }
        else if(__node_ptr m = __find_in_chain(n->__get_ref(), &__singularity)) { __destroy_node(n); return make_pair(m, false); }
        return __insert_node(__bucket_index(n), n);
    }
    template<typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<KT, VT> XT, __detail::__predicate<KT> ET = std::equal_to<void>, __detail::__hash_policy PT = __impl::__prime_number_policy, __detail::__hash_range_functor RT = __impl::__modulus_hash_range, allocator_object<__hash_node<VT>> AT = std::allocator<__hash_node<VT>>>
    class hash_table : protected __hash_table_base<KT, VT, HT, XT, ET, PT, RT, AT>
    {
        using __base = __hash_table_base<KT, VT, HT, XT, ET, PT, RT, AT>;
    protected:
        using typename __base::__hash;
        using typename __base::__hash_range;
        using typename __base::__key_extract;
        using typename __base::__hash_bucket;
        using typename __base::__policy_type;
        using typename __base::__node_ptr;
        using typename __base::__node_const_ptr;
        using typename __base::__buckets_ptr;
    public:
        using typename __base::key_type;
        using typename __base::value_type;
        using typename __base::allocator_type;
        using typename __base::key_equal;
        using typename __base::size_type;
        using typename __base::iterator;
        using typename __base::const_iterator;
        constexpr hash_table() noexcept : __base{} {}
        constexpr hash_table(size_type init_count) : __base{ init_count } {}
        template<typename ... Args> requires constructible_from<PT, Args...> constexpr hash_table(size_type init_count, Args&& ... args) : __base{ init_count, forward<Args>(args)... } {}
        constexpr size_type size() const noexcept { return this->__elem_count; }
        constexpr void reserve(size_type n) { this->__reserve_at_least(n); }
        template<convertible_to<value_type> WT> constexpr pair<iterator, bool> insert(WT && wt) { pair<__node_ptr, bool> result = this->__insert_unique(forward<WT>(wt)); return make_pair(this->__iterator(result.first), result.second); }
        template<convertible_to<value_type> WT> requires copy_constructible<WT> constexpr pair<iterator, bool> insert(WT const& wt) { pair<__node_ptr, bool> result = this->__insert_unique(WT(wt)); return make_pair(this->__iterator(result.first), result.second); }
        template<typename ... Args> requires constructible_from<value_type, Args...> constexpr pair<iterator, bool> emplace(Args&& ... args) {  pair<__node_ptr, bool> result = this->__emplace_unique(forward<Args>(args)...); return make_pair(this->__iterator(result.first), result.second); }
        template<convertible_to<key_type> LT> constexpr iterator find(LT const& lt) { return this->__iterator(this->__find(lt)); }
        template<convertible_to<key_type> LT> constexpr const_iterator find(LT const& lt) const { return this->__const_iterator(this->__find(lt)); }
        constexpr void erase(const_iterator i) { this->__erase_node(const_cast<__node_ptr>(i.__my_node)); }
        template<convertible_to<key_type> LT> constexpr bool erase(LT const& lt) { __node_ptr n = this->__find(lt); if(n) { this->__erase_node(n); return true; } return false; }
        constexpr iterator begin() noexcept { return this->__begin(); }
        constexpr iterator end() noexcept { return this->__end(); }
        constexpr const_iterator cbegin() const noexcept { return this->__cbegin(); }
        constexpr const_iterator cend() const noexcept { return this->__cend(); }
        constexpr const_iterator begin() const noexcept { return this->__cbegin(); }
        constexpr const_iterator end() const noexcept { return this->__cend(); }
    };
    template<typename VT, __detail::__hash_functor<VT> HT, __detail::__predicate<VT> ET = std::equal_to<void>, __detail::__hash_policy PT = __impl::__prime_number_policy, __detail::__hash_range_functor RT = __impl::__modulus_hash_range, allocator_object<__hash_node<VT>> AT = std::allocator<__hash_node<VT>>>
    using hash_set = hash_table<VT, VT, HT, __impl::__identity_key<VT>, ET, PT, RT, AT>;
    template<typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__predicate<KT> ET = std::equal_to<void>, __detail::__hash_policy PT = __impl::__prime_number_policy, __detail::__hash_range_functor RT = __impl::__modulus_hash_range, allocator_object<__hash_node<pair<KT, VT>>> AT = std::allocator<__hash_node<pair<KT, VT>>>>
    using hash_map = hash_table<KT, pair<KT, VT>, HT, __impl::__pair_key_extract<KT, VT>, ET, PT, RT, AT>;
}

#endif