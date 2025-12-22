#ifndef __STL_QUEUE
#define __STL_QUEUE
#include <bits/dynamic_queue.hpp>
namespace std
{
	extension namespace ext
	{
		template<typename T, allocator_object<T> A> struct stale_settings { unsigned int op_threshold; typename std::__impl::__dynamic_queue<T, A>::__difference_type size_threshold; };
		template<typename T, allocator_object<T> A = std::allocator<T>>
		class resettable_queue : protected std::__impl::__dynamic_queue<T, A>
		{
			typedef std::__impl::__dynamic_queue<T, A> __base;
			bool __enable_trim_stale = false;
			// copy- and move-assign and construct functions implicitly inherit from base
		protected:
			virtual void on_modify_queue() override { if(__enable_trim_stale) { this->__trim_stale(); } }
		public:
			typedef T value_type;
			typedef typename __base::__allocator_type allocator_type;
			typedef typename __base::__size_type size_type;
			typedef typename __base::__difference_type difference_type;
			typedef typename __base::__reference reference;
			typedef typename __base::__const_reference const_reference;
			typedef typename __base::__pointer pointer;
			typedef typename __base::__const_pointer const_pointer;
			typedef ::__impl::__iterator<pointer, resettable_queue> iterator;
			typedef ::__impl::__iterator<const_pointer, resettable_queue> const_iterator;
			typedef std::reverse_iterator<iterator> reverse_iterator;
			typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
			constexpr resettable_queue() noexcept = default;
			constexpr resettable_queue(size_type st_cap, allocator_type const& alloc = allocator_type{}) : __base(st_cap, alloc) {}
			constexpr void reserve(size_type ncap) { if(this->__qcapacity() < ncap && ! this->__q_grow_buffer(static_cast<size_type>(ncap - this->__qcapacity()))) throw std::runtime_error{ "failed to allocate buffer" }; }
			constexpr void clear() { this->__qclear(); }
			constexpr size_type size() const noexcept { return this->__qrem(); }
			constexpr bool empty() const noexcept { return !this->__qsize(); }
			constexpr bool at_end() const noexcept { return !this->__qrem(); }
			constexpr void restart() noexcept { this->__qrst(); }
			constexpr reference front() { return *(this->__qbeg()); }
			constexpr const_reference front() const { return *(this->__qbeg()); }
			constexpr reference next() { return *(this->__qcur()); }
			constexpr const_reference next() const { return *(this->__qcur()); }
			constexpr reference back() { return *(this->__end() - 1); }
			constexpr const_reference back() const { return *(this->__end() - 1); }
			constexpr iterator begin() { return iterator(this->__qbeg()); }
			constexpr const_iterator cbegin() const { return const_iterator(this->__qbeg()); }
			constexpr const_iterator begin() const { return cbegin(); }
			extension constexpr iterator current() { return iterator(this->__qcur()); }
			extension constexpr const_iterator ccurrent() const { return const_iterator(this->__qcur()); }
			extension constexpr const_iterator current() const { return ccurrent(); }
			constexpr iterator end() { return iterator(this->__end()); }
			constexpr const_iterator cend() const { return const_iterator(this->__end()); }
			constexpr const_iterator end() const { return cend(); }
			constexpr reverse_iterator rbegin() { return reverse_iterator(end()); }
			constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
			constexpr const_reverse_iterator rbegin() const { return crbegin(); }
			extension constexpr reverse_iterator rcurrent() { return reverse_iterator(ccurrent()); }
			extension constexpr const_reverse_iterator crcurrent() const { return const_reverse_iterator(ccurrent()); }
			extension constexpr const_reverse_iterator rcurrent() const { return crcurrent(); }
			constexpr reverse_iterator rend() { return reverse_iterator(begin()); }
			constexpr const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }
			constexpr const_reverse_iterator rend() const { return crend(); }
			constexpr void push(value_type const& value) { this->__push_elements(value, 1UL); }
			constexpr void push(value_type&& value) { this->__push_elements(move(value), 1UL); }
			constexpr void swap(resettable_queue& that) noexcept { this->__qswap(that); }
			template<typename ... Args> requires(std::constructible_from<T, Args...>) constexpr reference emplace(Args&& ... args) { return *(this->__emplace_element(forward<Args>(args)...)); }
			constexpr reference pop() { pointer result = this->__pop_next(); if(result) return *result; throw std::out_of_range("[std::ext::resettable_queue] nothing to pop"); }
			constexpr size_type erase(const_iterator start, const_iterator end) { return this->__erase_elements(start.base(), std::distance(start, end)); }
			constexpr size_type erase(const_iterator where) { return this->__erase_elements(where.base()); }
			constexpr iterator insert(const_iterator where, const_reference what, size_type how_many = 1UL) { pointer result = this->__insert(where.base(), what, how_many); if(result) return iterator(result); return end(); }
			extension constexpr iterator find(const_reference what, bool include_stale = false) noexcept requires(equality_comparable<value_type>) { for(iterator i = include_stale ? begin() : current(); i != end(); i++) { if(*i == what) return i; } return end(); }
			extension constexpr const_iterator find(const_reference what, bool include_stale = false) const noexcept requires(equality_comparable<value_type>) { for(const_iterator i = include_stale ? begin() : current(); i != end(); i++) { if(*i == what) return i; } return end(); }
			extension constexpr pointer unpop() noexcept { return this->__unpop(); }
			extension constexpr size_type trim() { return this->__force_trim(); }
			extension constexpr void set_trim_stale(bool enable = true) noexcept { __enable_trim_stale = enable; }
			extension constexpr void set_stale_settings(stale_settings<T, A> const& settings) noexcept { __enable_trim_stale = true; this->__set_stale_op_threshold(settings.op_threshold); this->__set_stale_size_threshold(settings.size_threshold); }
			extension constexpr size_type transfer(resettable_queue& to_whom, const_iterator start, const_iterator end) { to_whom.__push_elements(start.base(), end.base()); return erase(start, end); }
			extension constexpr size_type transfer(resettable_queue& to_whom, const_iterator what) { to_whom.__push_elements(what); return erase(what); }
			extension constexpr size_type transfer(resettable_queue& to_whom, size_type how_many) { to_whom.__push_elements(this->__qcur(), this->__qcur() + how_many); return this->__erase_elements(this->__qcur(), how_many); }
			extension constexpr size_type transfer(resettable_queue& to_whom) { return this->transfer(to_whom, 1UZ); }
			extension template<equality_comparable_to<value_type> U> constexpr iterator find_like(U const& what, bool include_stale = false) noexcept { for(iterator i = include_stale ? begin() : current(); i != end(); i++) { if(*i == what) return i; } return end(); }
			extension template<equality_comparable_to<value_type> U> constexpr const_iterator find_like(U const& what, bool include_stale = false) const noexcept { for(const_iterator i = include_stale ? begin() : current(); i != end(); i++) { if(*i == what) return i; } return end(); }
		};
	}
}
#endif