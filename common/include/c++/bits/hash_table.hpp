#ifndef __HASH_TABLE
#define __HASH_TABLE
#include "bits/stl_function.hpp" // __invocable_as
#include "bits/aligned_buffer.hpp"
#include "memory" // construct_at
namespace std
{
    namespace __detail
    {
        template<typename FT, typename KT> concept __hash_functor = __invocable_as<FT, size_t, std::decay_t<KT>>::value && is_default_constructible_v<FT>;
        template<typename FT, typename VT, typename KT> concept __key_functor = __invocable_as<FT, KT, std::decay_t<VT>>::value && is_default_constructible_v<FT>;
        template<typename FT, typename KT> concept __predicate = requires(FT ft, KT kt, KT kt2) { { ft(kt, kt2) } -> __boolean_testable; } && is_default_constructible_v<FT>;
        template<typename FT> concept __hash_range_functor = __invocable_as<FT, size_t, size_t, size_t>::value;
        template<typename PT> concept __hash_policy = requires(PT pt, size_t m, size_t n, size_t i) { { pt.max_load_factor() } -> floating_point; { pt.__next_bucket(m) } -> unsigned_integral; { pt.__buckets_for_elements(m) } -> unsigned_integral; { pt.__need_rehash(m, n, i) } -> __boolean_testable; } && is_default_constructible_v<PT>;
    }
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
        constexpr __value_ptr operator->() const noexcept { return __my_node->__get_ptr(); }
        constexpr __value_ref operator*() const noexcept { return __my_node->__get_ref(); }
        constexpr __hash_table_iterator& operator++() noexcept { if(__my_node->__next_node) __my_node = __my_node->__next_node; else { __current_idx++; __my_node = __current_idx < __table_size ? __target_table[__current_idx].__start_node : nullptr; } return *this; }
        constexpr __hash_table_iterator operator++(int) noexcept { __hash_table_iterator that{ *this }; ++(*this); return that; }
        friend constexpr bool operator==(__hash_table_iterator<VT> const& __this, __hash_table_iterator<VT> const& __that) noexcept { return __this.__my_node == __that.__my_node; }
    };
    template<typename KT, typename VT, __detail::__hash_functor<KT> HT, __detail::__key_functor<VT, KT> XT, __detail::__predicate<KT> ET, __detail::__hash_policy PT, __detail::__hash_range_functor RT, allocator_object<__hash_node<VT>> AT>
    struct __hash_table
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
        constexpr static size_type __get_hash(value_type const& vt) { return __hash{}(__key_extract{}(vt)); }
        constexpr static size_type __range_check_at_size(size_type n, size_type l) { return __hash_range{}(n, l); }
        constexpr static size_type __bucket_index_at_size(value_type const& vt, size_type nbuckets) { return __range_check_at_size(__get_hash(vt), nbuckets); }
        constexpr static __node_ptr __find_in_chain(value_type const& vt, __hash_bucket const& start) { for(__node_ptr b = start.__start_node; b; b = b->__next_node) { if(key_equal{}(vt, __key_extract{}(b->__get_ref()))) return b; } return nullptr; }
        __policy_type __my_policy;
        size_type __bucket_count;
        size_type __elem_count;
        __hash_bucket __singularity;
        __buckets_ptr __my_buckets{ &__singularity };
        constexpr size_type __range_check(size_type n) { return __range_check_at_size(n, __bucket_count); }
        constexpr size_type __bucket_index(value_type const& vt) { return __range_check(__get_hash(vt)); }
        constexpr iterator __iterator(__node_ptr np) { return iterator{ np, __my_buckets, __bucket_index(np->__get_ref()),  __bucket_count }; }
        constexpr const_iterator __const_iterator(__node_const_ptr np) const { return const_iterator{ np,  __my_buckets, __bucket_index(np->__get_ref()),  __bucket_count }; }
        constexpr __buckets_ptr __allocate_buckets(size_type how_many) { if(__builtin_expect(how_many == 1, false)) { __singularity = nullptr; return &__singularity; } return __bucket_alloc{}.allocate(how_many); }
        constexpr void __deallocate_buckets(__buckets_ptr bkts, size_type how_many) { if(__builtin_expect(bkts == &__singularity, false)) return; __bucket_alloc{}.deallocate(bkts, how_many); }
        constexpr void __insert_at(size_type index, __node_ptr n) { n->__next_node = __my_buckets[index].__start_node; __my_buckets[index].__start_node = n; if(!__my_buckets[index].__end_node) { __my_buckets[index].__end_node = n; } }
    };
}

#endif