#ifndef __HASH_MAP
#define __HASH_MAP
#include <bits/hashtable.hpp>
namespace std
{
	extension template<typename KT, typename MT, __detail::__hash_ftor<KT> HT, __detail::__predicate<KT> ET, allocator_object<pair<const KT, MT>> AT>
	class hash_map : protected __impl::__hashtable<KT, pair<const KT, MT>, HT, __detail::__pair_key_extract<const KT, MT>, ET, AT>
	{
		using __base = __impl::__hashtable<KT, pair<const KT, MT>, HT, __detail::__pair_key_extract<const KT, MT>, ET, AT>;
	protected:
		using typename __base::__node_ptr;
		using typename __base::__const_node_ptr;
	public:
		using typename __base::size_type;
		using typename __base::difference_type;
		using typename __base::key_type;
		using typename __base::value_type;
		typedef MT mapped_type;
		using typename __base::allocator_type;
		using typename __base::iterator;
		using typename __base::const_iterator;
		using typename __base::pointer;
		using typename __base::reference;
		using typename __base::const_pointer;
		using typename __base::const_reference;
	private:
		constexpr pair<iterator, bool> __insert_or_assign(__node_ptr n);
	public:
		constexpr hash_map(size_type bucket_ct) : __base(bucket_ct) {}
		constexpr hash_map()							= default;
		constexpr hash_map(hash_map&&)					= default;
		constexpr hash_map(hash_map const&)				= default;
		constexpr hash_map& operator=(hash_map&&)		= default;
		constexpr hash_map& operator=(hash_map const&)	= default;
		constexpr allocator_type get_allocator() const noexcept { return allocator_type(this); }
		constexpr size_type size() const noexcept { return this->__size(); }
		constexpr size_type bucket(key_type const& k) const noexcept { return this->__index(k); }
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
		template<convertible_to<key_type> JT, convertible_to<mapped_type> NT> constexpr pair<iterator, bool> insert(pair<JT, NT>&& p) { return this->__emplace(piecewise_construct, forward_as_tuple(move(p.first)), forward_as_tuple(move(p.second))); }
		template<convertible_to<key_type> JT, convertible_to<mapped_type> NT> constexpr pair<iterator, bool> insert(pair<JT, NT> const& p) { return this->__emplace(piecewise_construct, tuple<JT const&>(p.first), tuple<NT const&>(p.second)); }
		template<std::convertible_to<key_type> JT> constexpr size_type erase(JT const& what) { return this->__erase(what); }
		template<std::convertible_to<key_type> JT> constexpr bool contains(JT const& what) const { return this->__contains(what); }
		template<std::input_iterator IT> requires(constructible_from<value_type, deref_t<IT>>) constexpr void insert(IT start, IT end) { this->__insert(start, end); }
		constexpr iterator erase(const_iterator what) { return this->__erase(what); }
		constexpr void clear() { this->__clear(); }
		template<std::convertible_to<key_type> JT> requires(is_default_constructible_v<mapped_type>) constexpr mapped_type& operator[](JT&& key) { return this->__emplace(piecewise_construct, forward_as_tuple(move(key)), tuple<>()).first->second; }
		template<std::convertible_to<key_type> JT> requires(is_default_constructible_v<mapped_type>) constexpr mapped_type& operator[](JT const& key) { return this->__emplace(piecewise_construct, tuple<JT const&>(key), tuple<>()).first->second; }
		constexpr pair<iterator, bool> insert_or_assign(key_type&& kt, mapped_type&& mt) { return __insert_or_assign(this->__create_node(piecewise_construct, forward_as_tuple(move(kt)), forward_as_tuple(move(mt)))); }
		constexpr pair<iterator, bool> insert_or_assign(key_type const& kt, mapped_type&& mt) { return __insert_or_assign(this->__create_node(piecewise_construct, tuple<key_type const&>(kt), forward_as_tuple(move(mt)))); }
		template<not_self<value_type> PT> requires(constructible_from<value_type, PT&&>) constexpr pair<iterator, bool> insert(PT&& pt) { return this->__emplace(move(pt)); }
	};
	template<typename KT, typename MT, __detail::__hash_ftor<KT> HT, __detail::__predicate<KT> ET, allocator_object<pair<const KT, MT>> AT>
	constexpr pair<typename hash_map<KT, MT, HT, ET, AT>::iterator, bool> hash_map<KT, MT, HT, ET, AT>::__insert_or_assign(__node_ptr n)
	{
		__node_ptr p			= this->__find(n->__ref().first);
		if(p)
		{
			p->__ref().second	= move(n->__ref().second);
			this->__destroy_node(n);
			return make_pair(iterator(p), false);
		}
		return make_pair(this->__insert(n), true);
	}
}
#endif