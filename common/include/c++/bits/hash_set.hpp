#ifndef __HASH_SET
#define __HASH_SET
#include <bits/hashtable.hpp>
namespace std
{
	template<typename T, typename V> concept __alt_construct = is_constructible_v<T, V> && !is_same_v<T, V>;
	extension template<typename T, typename KT, __detail::__hash_ftor<KT> HT, __detail::__predicate<KT> ET, allocator_object<T> AT, __detail::__key_extract<KT, T> XT>
	class hash_set : protected __impl::__hashtable<KT, T, HT, XT, ET, AT>
	{
		using __base = __impl::__hashtable<KT, T, HT, XT, ET, AT>;
	protected:
		using typename __base::__node_ptr;
		using typename __base::__const_node_ptr;
	public:
		using typename __base::size_type;
		using typename __base::difference_type;
		using typename __base::key_type;
		using typename __base::value_type;
		using typename __base::allocator_type;
		using typename __base::iterator;
		using typename __base::const_iterator;
		using typename __base::pointer;
		using typename __base::reference;
		using typename __base::const_pointer;
		using typename __base::const_reference;
		constexpr hash_set(size_type bucket_ct) : __base(bucket_ct) {}
		constexpr hash_set()							= default;
		constexpr hash_set(hash_set const&)				= default;
		constexpr hash_set(hash_set&&)					= default;
		constexpr hash_set& operator=(hash_set const&)	= default;
		constexpr hash_set& operator=(hash_set&&)		= default;
		constexpr allocator_type get_allocator() const noexcept { return allocator_type(this); }
		constexpr size_type bucket(key_type const& k) const noexcept { return this->__index(k); }
		constexpr size_type size() const noexcept { return this->__size(); }
		constexpr iterator find(key_type const& k) { return iterator(this->__find(k)); }
		constexpr const_iterator find(key_type const& k) const { return const_iterator(this->__find(k)); }
		constexpr iterator begin() noexcept { return iterator(this->__begin()); }
		constexpr const_iterator cbegin() const noexcept { return const_iterator(this->__begin()); }
		constexpr const_iterator begin() const noexcept { return cbegin(); }
		constexpr iterator end() noexcept { return iterator(nullptr); }
		constexpr const_iterator cend() const noexcept { return const_iterator(nullptr); }
		constexpr const_iterator end() const noexcept { return const_iterator(nullptr); }
		constexpr iterator begin(size_type bucket) noexcept { return iterator(this->__begin(bucket)); }
		constexpr const_iterator cbegin(size_type bucket) const noexcept { return const_iterator(this->__begin(bucket)); }
		constexpr const_iterator begin(size_type bucket) const noexcept { return cbegin(bucket); }
		constexpr iterator end(size_type bucket) noexcept { return iterator(this->__end(bucket)); }
		constexpr const_iterator cend(size_type bucket) const noexcept { return const_iterator(this->__end(bucket)); }
		constexpr const_iterator end(size_type bucket) const noexcept { return cend(bucket); }
		extension iterator before(iterator what) { return this->__before(what); }
		extension const_iterator cbefore(const_iterator what) const { return this->__before(what); }
		extension const_iterator before(const_iterator what) const { return cbefore(what); }
		constexpr float max_load_factor() const noexcept { return this->__get_max_load(); }
		constexpr void max_load_factor(float f) noexcept { this->__set_max_load(f); }
		constexpr float load_factor() const noexcept { return this->__current_load(); }
		constexpr void rehash(size_type target) { this->__rehash_to_size(target); }
		constexpr void reserve(size_type target) { this->__reserve(target); }
		template<typename ... Args> requires(constructible_from<value_type, Args...>) pair<iterator, bool> emplace(Args&& ... args) { return this->__emplace(forward<Args>(args)...); }
		template<std::convertible_to<value_type> WT> requires(copy_constructible<value_type>) constexpr pair<iterator, bool> insert(WT const& wt) { return this->__insert(wt); }
		template<std::convertible_to<value_type> WT> requires(move_constructible<value_type>) constexpr pair<iterator, bool> insert(WT&& wt) { return this->__insert(move(wt)); }
		template<std::convertible_to<key_type> JT> constexpr size_type erase(JT const& what) { return this->__erase(what); }
		template<std::convertible_to<key_type> JT> constexpr bool contains(JT const& what) const { return this->__contains(what); }
		template<std::input_iterator IT> requires(constructible_from<value_type, deref_t<IT>>) constexpr void insert(IT start, IT end) { this->__insert(start, end); }
		constexpr value_type& operator[](key_type const&) requires(__alt_construct<value_type, key_type const&>);
		constexpr value_type& operator[](key_type&&) requires(__alt_construct<value_type, key_type&&>);
		constexpr iterator erase(const_iterator what) { return this->__erase(what); }
		constexpr void clear() { this->__clear(); }
	};
	template<typename T, typename KT, __detail::__hash_ftor<KT> HT, __detail::__predicate<KT> ET, allocator_object<T> AT, __detail::__key_extract<KT, T> XT>
	constexpr typename __impl::__hashtable<KT, T, HT, XT, ET, AT>::value_type& hash_set<T, KT, HT, ET, AT, XT>::operator[](key_type const& kt)
	requires(__alt_construct<typename __impl::__hashtable<KT, T, HT, XT, ET, AT>::value_type, typename __impl::__hashtable<KT, T, HT, XT, ET, AT>::key_type const&>)
	{
		iterator i	= find(kt);
		if(i != end()) return *i;
		return *(emplace(kt).first);
	}
	template<typename T, typename KT, __detail::__hash_ftor<KT> HT, __detail::__predicate<KT> ET, allocator_object<T> AT, __detail::__key_extract<KT, T> XT>
	constexpr typename __impl::__hashtable<KT, T, HT, XT, ET, AT>::value_type& hash_set<T, KT, HT, ET, AT, XT>::operator[](key_type&& kt)
	requires(__alt_construct<typename __impl::__hashtable<KT, T, HT, XT, ET, AT>::value_type, typename __impl::__hashtable<KT, T, HT, XT, ET, AT>::key_type&&>)
	{
		iterator i	= find(kt);
		if(i		!= end()) return *i;
		return *(emplace(move(kt)).first);
	}
}
#endif