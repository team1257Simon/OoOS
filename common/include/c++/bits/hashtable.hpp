#ifndef __HASHTABLE
#define __HASHTABLE
#include <bits/aligned_buffer.hpp>
#include <bits/stl_iterator.hpp>
#include <bits/stl_algobase.hpp>
#include <bits/functional_compare.hpp>	// equal_to
#include <memory>						// construct_at
#include <tuple>						// piecewise_construct, pair
#include <initializer_list>
namespace std
{
	namespace __detail
	{
		template<typename FT, typename KT> concept __hash_ftor = is_default_constructible_v<FT> && unsigned_integral<decltype(declval<FT>()(declval<KT const&>()))>;
		template<typename FT, typename KT, typename VT> concept __key_extract = is_default_constructible_v<FT> && convertible_to<decltype(declval<FT>()(declval<VT const&>())), KT const&>;
		template<typename FT, typename KT> concept __predicate = is_default_constructible_v<FT> && __boolean_testable<decltype(declval<FT>()(declval<KT const&>(), declval<KT const&>()))>;
		template<typename KT, typename MT> struct __pair_key_extract { constexpr KT const& operator()(pair<KT, MT> const& p) const noexcept { return p.first; } };
		template<typename T> struct __identity_key { constexpr T const& operator()(T const& t) const noexcept { return t; } };
	}
	namespace __impl
	{
		struct __hash_node_base
		{
			__hash_node_base* __next				= nullptr;	// next node in the chain (i.e. same bucket)
			constexpr __hash_node_base() noexcept	= default;
			constexpr __hash_node_base(__hash_node_base* n) noexcept : __next(n) {}
		};
		struct __hashtable_base
		{
			typedef __hash_node_base* __base_ptr;
			typedef __hash_node_base const* __const_base_ptr;
			typedef __base_ptr __bucket;
			typedef __bucket* __buckets_ptr;
			typedef decltype(sizeof(__hash_node_base)) size_type;
			__hash_node_base __root{};
			__bucket __singularity			= nullptr;
			size_type __element_count		= 0UZ;
			size_type __bucket_count		= 1UZ;
			size_type __after_root_idx		= 0UZ;
			__buckets_ptr __my_buckets		= addressof(__singularity);
			float __max_load				= 1.0F;
			constexpr __hashtable_base()	= default;
			constexpr bool __is_singularity() const noexcept { return __bucket_count < 2UZ || __my_buckets == addressof(__singularity); }
			constexpr float __load(size_type added) const noexcept { return (__element_count + added) / double(__bucket_count); }
			constexpr float __current_load() const noexcept { return __load(0UZ); }
			constexpr bool __need_rehash(size_type added) const noexcept { return __bucket_count < 2UZ || __load(added) >= __max_load; }
			constexpr size_type __next_bucket_ct(size_type added) const noexcept { return max(5UZ, __bucket_count) * (1UZ + static_cast<unsigned int>(__builtin_ceilf(__load(added) / __max_load))); }
			constexpr size_type __size() const noexcept { return __element_count; }
			constexpr size_type __range(size_type idx) const noexcept { return idx % __bucket_count; }
			constexpr size_type __range(size_type idx, size_type max) const noexcept { if(__unlikely(!max)) return 0UZ; return idx % max; }
			constexpr void __deallocate_buckets() { if(!__is_singularity()) { allocator<__bucket>().deallocate(__my_buckets, __bucket_count); } }
			constexpr void __insert_at(__buckets_ptr buckets, size_type idx, __base_ptr n);
			constexpr void __remove_first_at(__buckets_ptr buckets, size_type idx, __base_ptr n_next, size_type next_bucket);
			constexpr void __insert_at(size_type idx, __base_ptr n) { __insert_at(__my_buckets, idx, n); }
			constexpr void __remove_first_at(size_type idx, __base_ptr n_next, size_type next_bucket) { __remove_first_at(__my_buckets, idx, n_next, next_bucket); }
			constexpr __hashtable_base(size_type ct) : __bucket_count(ct) { this->__init_buckets(ct); }
			constexpr __buckets_ptr __allocate_buckets(size_type n)
			{
				__buckets_ptr result	= allocator<__bucket>().allocate(n);
				array_zero(result, n);
				return result;
			}
			constexpr void __reset() noexcept
			{
				__root.__next		= nullptr;
				__after_root_idx	= 0UZ;
				__element_count		= 0UZ;
				__bucket_count		= 1UZ;
				__singularity		= nullptr;
				__my_buckets		= std::addressof(__singularity);
			}
			constexpr void __init_buckets(size_type n)
			{
				if(__unlikely(n < 2UZ))
				{
					__my_buckets	= addressof(__singularity);
					__singularity	= nullptr;
					return;
				}
				__my_buckets		= __allocate_buckets(n);
			}
			constexpr void __move_assign(__hashtable_base&& that) noexcept
			{
				this->__root.__next		= that.__root.__next;
				this->__element_count	= that.__element_count;
				this->__bucket_count	= that.__bucket_count;
				this->__after_root_idx	= that.__after_root_idx;
				this->__my_buckets		= that.__my_buckets;
				that.__reset();
			}
			constexpr __hashtable_base(__hashtable_base&& that) noexcept :
				__root				{ that.__root },
				__element_count		{ that.__element_count },
				__bucket_count		{ that.__bucket_count },
				__after_root_idx	{ that.__after_root_idx },
				__my_buckets		{ that.__my_buckets }
									{ that.__reset(); }
		};
		constexpr void __hashtable_base::__insert_at(__buckets_ptr buckets, size_type idx, __base_ptr n)
		{
			if(buckets[idx]) { n->__next = buckets[idx]->__next; buckets[idx]->__next = n; }
			else
			{
				n->__next			= __root.__next;
				__root.__next		= n;
				if(n->__next) { buckets[__after_root_idx] = n; }
				__after_root_idx	= idx;
				buckets[idx]		= std::addressof(__root);
			}
		}
		constexpr void __hashtable_base::__remove_first_at(__buckets_ptr buckets, size_type idx, __base_ptr n_next, size_type next_bucket)
		{
			if(!n_next) { buckets[idx]	= nullptr; }
			else if(idx != next_bucket)
			{
				buckets[next_bucket]	= buckets[idx];
				buckets[idx]			= nullptr;
				if(idx == __after_root_idx)
					__after_root_idx	= next_bucket;
			}
		}
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
			typedef input_iterator_tag iterator_category;
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
			typedef input_iterator_tag iterator_category;
		private:
			typedef __hashtable_iterator<T> __iterator_type;
			typedef __hashtable_const_iterator<T> __const_iterator_type;
			typedef __hash_node_base const* __base_ptr;
			typedef __hash_node<T> const* __node_ptr;
			__base_ptr __my_node;
		public:
			constexpr __hashtable_const_iterator() noexcept : __my_node() {}
			constexpr explicit __hashtable_const_iterator(__base_ptr n) noexcept : __my_node(n) {}
			constexpr __hashtable_const_iterator(__iterator_type const& i) noexcept : __my_node(i.get_node()) {}
			extension constexpr __node_ptr get_node() const noexcept { return static_cast<__node_ptr>(__my_node); }
			constexpr pointer base() const noexcept { return get_node()->__ptr(); }
			constexpr pointer operator->() const noexcept { return base(); }
			constexpr reference operator*() const noexcept { return *base(); }
			constexpr __const_iterator_type& operator++() { __my_node = __my_node->__next; return *this; }
			constexpr __const_iterator_type operator++(int) { __const_iterator_type that(__my_node); __my_node = __my_node->__next; return that; }
			friend constexpr bool operator==(__hashtable_const_iterator<T> const& __this, __hashtable_const_iterator<T> const& __that) noexcept { return __this.__my_node == __that.__my_node; }
			constexpr operator bool() const noexcept { return __my_node != nullptr; }
		};
		template<typename VT, allocator_object<VT> AT> using __hash_node_allocator = typename AT::template rebind<__hash_node<VT>>::other;
		template<typename KT, typename VT, __detail::__hash_ftor<KT> HT, __detail::__key_extract<KT, VT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
		struct __hashtable : __hashtable_base, __hash_node_allocator<VT, AT>
		{
			typedef __hashtable_base __base;
			typedef KT key_type;
			typedef VT value_type;
			typedef AT allocator_type;
			typedef HT hasher;
			typedef ET key_equal;
			typedef __hashtable_iterator<value_type> iterator;
			typedef __hashtable_const_iterator<value_type> const_iterator;
			typedef typename iterator::reference reference;
			typedef typename const_iterator::reference const_reference;
			typedef typename iterator::pointer pointer;
			typedef typename const_iterator::pointer const_pointer;
			typedef decltype(declval<pointer>() - declval<pointer>()) difference_type;
			using typename __base::size_type;
			using typename __base::__bucket;
			using typename __base::__base_ptr;
			using typename __base::__const_base_ptr;
			using typename __base::__buckets_ptr;
			typedef __hash_node<value_type> __node_type;
			typedef __node_type* __node_ptr;
			typedef __node_type const* __const_node_ptr;
			typedef XT __key_extract;
			typedef __hash_node_allocator<VT, AT> __node_alloc;
			constexpr static key_type const& __key_of(__const_node_ptr n) { return __key_extract()(n->__ref()); }
			constexpr static bool __is_equal(key_type const& k1, key_type const& k2) { return key_equal()(k1, k2); }
			constexpr size_type __hash_code(__const_node_ptr n) const { return hasher()(__key_extract()(n->__ref())); }
			constexpr size_type __hash_code(key_type const& k) const { return hasher()(k); }
			constexpr size_type __hash_code(value_type const& v) const requires(!std::is_same_v<key_type const&, value_type const&>) { return hasher()(__key_extract()(v)); }
			constexpr size_type __index(__const_node_ptr n) const { return this->__range(__hash_code(n)); }
			constexpr size_type __index(key_type const& k) const { return this->__range(__hash_code(k)); }
			constexpr size_type __index(value_type const& v) const requires(!std::is_same_v<key_type const&, value_type const&>) { return this->__range(__hash_code(v)); }
			constexpr __node_ptr __allocate_node() { __node_ptr n = this->allocate(1UZ); if consteval { construct_at(n); } return n; }
			constexpr __node_ptr __begin() noexcept { return static_cast<__node_ptr>(this->__root.__next); }
			constexpr __const_node_ptr __begin() const noexcept { return static_cast<__const_node_ptr>(this->__root.__next); }
			constexpr __node_ptr __advance_chain(__base_ptr chain) noexcept { return chain ? static_cast<__node_ptr>(chain->__next) : nullptr; }
			constexpr __const_node_ptr __advance_chain(__const_base_ptr chain) const noexcept { return chain ? static_cast<__const_node_ptr>(chain->__next) : nullptr; }
			constexpr __node_ptr __begin(size_type idx) noexcept { return this->__advance_chain(this->__my_buckets[idx]); }
			constexpr __const_node_ptr __begin(size_type idx) const noexcept { return this->__advance_chain(this->__my_buckets[idx]); }
			constexpr __base_ptr __get_before(__node_ptr n) { return this->__get_before(__index(n), n); }
			constexpr __const_base_ptr __get_before(__const_node_ptr n) const { return this->__get_before(this->__index(n), n); }
			constexpr __node_ptr __find(key_type const& what) { return this->__advance_chain(this->__find_before(what)); }
			constexpr __const_node_ptr __find(key_type const& what) const { return this->__advance_chain(this->__find_before(what)); }
			constexpr bool __contains(key_type const& what) const { return this->__find(what) != nullptr; }
			constexpr void __run_rehash(size_type target_count);
			constexpr iterator __erase_node(size_type idx, __base_ptr prev, __node_ptr n);
			constexpr iterator __insert(__node_ptr n, size_type added = 1UZ) { return this->__insert_node(__hash_code(n), n, added); }
			constexpr pair<iterator, bool> __insert(value_type const& val) requires(copy_constructible<value_type>) { return this->__insert_node(this->__create_node(val)); }
			constexpr pair<iterator, bool> __insert(value_type&& val) requires(move_constructible<value_type>) { return this->__insert_node(this->__create_node(move(val))); }
			constexpr void __insert(initializer_list<value_type> ini) requires(copy_constructible<value_type>) { this->__insert(ini.begin(), ini.end()); }
			template<typename ... Args> requires(constructible_from<value_type, Args...>)
			constexpr pair<iterator, bool> __emplace(Args&& ... args) { return this->__insert_node(this->__create_node(forward<Args>(args)...)); }
			constexpr float __get_max_load() const noexcept { return this->__max_load; }
			constexpr void __set_max_load(float val) noexcept { this->__max_load = val; }
			constexpr void __insert_all(__hashtable const& that) { this->__insert(const_iterator(that.__begin()), const_iterator(nullptr)); }
			constexpr size_type __target_count_at_least(size_type cnt) const noexcept { return max(cnt, static_cast<size_type>(__builtin_ceilf(this->__element_count / this->__max_load))); }
			constexpr __hashtable(size_type ct) : __base(ct), __node_alloc() {}
			constexpr __hashtable(size_type ct, allocator_type const& that) : __base(ct), __node_alloc(that) {}
			constexpr __hashtable() : __base(8UZ), __node_alloc() {}
			constexpr __hashtable(initializer_list<value_type> ini) requires(copy_constructible<value_type>) : __hashtable(ini.size()) { this->__insert(ini); }
			template<input_iterator IT, sentinel_for<IT> ST> requires(constructible_from<value_type, iter_reference_t<IT>>)
			constexpr __hashtable(IT first, ST last) : __hashtable(static_cast<size_type>(distance(first, last))) { this->__insert(first, last); }
			constexpr __hashtable(__hashtable&& that) : __base(std::move(that)), __node_alloc(std::forward<__node_alloc>(that)) {}
			constexpr __hashtable(__hashtable const& that) : __base(that.__size()), __node_alloc(that) { this->__insert_all(that); }
			constexpr ~__hashtable() { __deallocate_nodes(); __deallocate_buckets(); }
			constexpr size_type __size() const noexcept { return this->__element_count; }
			constexpr void __rehash_to_size(size_type target) { __run_rehash(__target_count_at_least(target)); }
			constexpr void __reserve(size_type target) { __rehash_to_size(static_cast<size_type>(__builtin_ceilf(target / this->__max_load))); }
			template<typename ... Args> requires(constructible_from<value_type, Args...>)
			constexpr __node_ptr __create_node(Args&& ... args)
			{
				__node_ptr n = this->__allocate_node();
				construct_at(n->__ptr(), forward<Args>(args)...);
				return n;
			}
			constexpr void __destroy_node(__node_ptr n)
			{
				if constexpr(!std::is_trivially_destructible_v<value_type>)
					n->__ref().~value_type();
				this->deallocate(n, 1UZ);
			}
			constexpr __node_ptr __end(size_type idx) noexcept
			{
				__node_ptr n;
				for(n = this->__begin(idx); n && this->__index(n) == idx; n = this->__advance_chain(n));
				return n;
			}
			constexpr __const_node_ptr __end(size_type idx) const noexcept
			{
				__const_node_ptr n;
				for(n = this->__begin(idx); n && this->__index(n) == idx; n = this->__advance_chain(n));
				return n;
			}
			constexpr __base_ptr __find_before(key_type const& what)
			{
				size_type idx		= this->__index(what);
				__base_ptr prev		= this->__my_buckets[idx];
				for(__node_ptr n	= this->__advance_chain(prev); n && idx == this->__index(n); prev = n, n = n->__get_next()) 
					if(__is_equal(what, __key_of(n)))
						return prev;
				return nullptr;
			}
			constexpr __const_base_ptr __find_before(key_type const& what) const
			{
				size_type idx			= this->__index(what);
				__const_base_ptr prev	= this->__my_buckets[idx];
				for(__const_node_ptr n	= this->__advance_chain(prev); n && idx == this->__index(n); prev = n, n = n->__get_next())
					if(__is_equal(what, __key_of(n)))
						return prev;
				return nullptr;
			}
			constexpr __base_ptr __get_before(size_type idx, __const_node_ptr n)
			{
				__base_ptr prev		= this->__my_buckets[idx];
				for(__node_ptr p	= this->__advance_chain(prev); p && idx == this->__index(p); prev = p, p = p->__get_next())
					if(n == p) return prev;
				return nullptr;
			}
			constexpr __const_base_ptr __get_before(size_type idx, __const_node_ptr n) const
			{
				__const_base_ptr prev	= this->__my_buckets[idx];
				for(__const_node_ptr p	= this->__advance_chain(prev); p && idx == this->__index(p); prev = p, p = p->__get_next())
					if(n == p) return prev;
				return nullptr;
			}
			constexpr iterator __insert_node(size_type hash, __base_ptr n, size_type added)
			{
				if(this->__need_rehash(added)) 
					__run_rehash(this->__next_bucket_ct(added));
				this->__insert_at(this->__range(hash), n);
				this->__element_count++;
				return iterator(n);
			}
			constexpr pair<iterator, bool> __insert_node(__node_ptr n, size_type added = 1UZ)
			{
				__node_ptr p = this->__find(__key_of(n));
				if(p) {
					this->__destroy_node(n);
					return make_pair(iterator(p), false);
				}
				return make_pair(this->__insert_node(__hash_code(n), n, added), true);
			}
			template<input_iterator IT, sentinel_for<IT> ST> requires(constructible_from<value_type, iter_reference_t<IT>>)
			constexpr void __insert(IT first, ST last)
			{
				size_type n		= static_cast<size_type>(distance(first, last));
				for(size_type i	= 0UZ; i < n; i++, ++first)
					this->__insert_node(this->__create_node(*first), static_cast<size_type>(n - i));
			}
			constexpr iterator __erase(const_iterator what)
			{
				__base_ptr prev	= this->__get_before(this->__index(what.get_node()), what.get_node());
				__node_ptr n	= this->__advance_chain(prev);
				return this->__erase_node(this->__index(n), prev, n);
			}
			constexpr size_type __erase(key_type const& what)
			{
				__base_ptr prev	= this->__find_before(what);
				if(__node_ptr n	= this->__advance_chain(prev)) {
					this->__erase_node(this->__index(n), prev, n);
					return 1UZ;
				}
				return 0UZ;
			}
			constexpr iterator __before(iterator what)
			{
				if(__base_ptr p = this->__get_before(what.get_node()); p != std::addressof(this->__root))
					return iterator(p);
				else return iterator(nullptr);
			}
			constexpr const_iterator __before(const_iterator what) const
			{
				if(__const_base_ptr p = this->__get_before(what.get_node()); p != std::addressof(this->__root))
					return const_iterator(p);
				else return const_iterator(nullptr);
			}
			constexpr void __deallocate_nodes()
			{
				__node_ptr n = this->__begin();
				for(__node_ptr s = this->__advance_chain(n); n; n = s, s = this->__advance_chain(s))
					this->__destroy_node(n);
			}
			constexpr void __clear()
			{
				__deallocate_nodes();
				this->__root.__next		= nullptr;
				this->__after_root_idx	= 0UZ;
				this->__element_count	= 0UZ;
				array_zero(__my_buckets, __bucket_count);
			}
			constexpr __hashtable& operator=(__hashtable const& that)
			{
				__clear();
				__insert(const_iterator(that.__begin()), const_iterator(nullptr));
				if constexpr(__has_copy_propagate<AT>)
					*static_cast<__node_alloc>(this)	= that;
				return *this;
			}
			constexpr __hashtable& operator=(__hashtable&& that)
			{
				__clear();
				this->__move_assign(move(that));
				if constexpr(__has_move_propagate<AT>)
					*static_cast<__node_alloc>(this)	= std::forward<__node_alloc>(that);
				that.__reset();
				that.__init_buckets(2UZ);
				return *this;
			}
			// TODO (if needed): void __run_rehash_multi(size_type target_count);
			// TODO (if needed): iterator __insert_node_multi(__node_ptr n, size_type added);
		};
		template <typename KT, typename VT, __detail::__hash_ftor<KT> HT, __detail::__key_extract<KT, VT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
		constexpr void __hashtable<KT, VT, HT, XT, ET, AT>::__run_rehash(size_type target_count)
		{
			if(__unlikely(target_count < 2UZ)) target_count = 2UZ;
			__buckets_ptr nbkts		= this->__allocate_buckets(target_count);
			__node_ptr orig			= __begin();
			this->__after_root_idx	= 0UZ;
			this->__root.__next		= nullptr;
			for(__node_ptr p = orig; p; p = p->__get_next()) this->__insert_at(nbkts, this->__range(__hash_code(p), target_count), p);
			this->__deallocate_buckets();
			this->__my_buckets		= nbkts;
			this->__bucket_count	= target_count;
		}
		template <typename KT, typename VT, __detail::__hash_ftor<KT> HT, __detail::__key_extract<KT, VT> XT, __detail::__predicate<KT> ET, allocator_object<VT> AT>
		constexpr typename __hashtable<KT, VT, HT, XT, ET, AT>::iterator __hashtable<KT, VT, HT, XT, ET, AT>::__erase_node(size_type idx, __base_ptr prev, __node_ptr n)
		{
			if(__unlikely(!n)) return iterator(nullptr);
			if(prev == this->__my_buckets[idx]) { this->__remove_first_at(idx, n->__next, n->__next ? __index(n->__get_next()) : 0UZ); }
			else if(n->__next)
			{
				size_t subs_idx						= __index(n->__get_next());
				if(idx != subs_idx)
					this->__my_buckets[subs_idx]	= prev;
			}
			prev->__next							= n->__next;
			iterator result(n->__next);
			__destroy_node(n);
			this->__element_count--;
			return result;
		}
	}
}
#endif