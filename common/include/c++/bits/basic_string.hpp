#ifndef __BASIC_STRING
#define __BASIC_STRING
#include "bits/dynamic_buffer.hpp"
#include "bits/char_traits.hpp"
namespace std
{
	namespace __detail
	{
		template<typename T, typename CT>
		concept __string_view_like = requires(T const& t)
		{
			requires(char_type<CT>);
			{ std::declval<T>().size() } -> std::unsigned_integral;
			{ t.begin() } -> matching_input_iterator<CT>;
			{ t.end() } -> matching_input_iterator<CT>;
		};
	}
	template<char_type CT, char_traits_type<CT> TT = std::char_traits<CT>, allocator_object<CT> AT = std::allocator<CT>>
	class basic_string : protected __impl::__dynamic_buffer<CT, AT, true>
	{
		typedef __impl::__dynamic_buffer<CT, AT, true> __base;
	public:
		typedef TT traits_type;
		typedef typename __base::__value_type value_type;
		typedef typename __base::__allocator_type allocator_type;
		typedef typename __base::__size_type size_type;
		typedef typename __base::__difference_type difference_type;
		typedef typename __base::__reference reference;
		typedef typename __base::__const_reference const_reference;
		typedef typename __base::__pointer pointer;
		typedef typename __base::__const_pointer const_pointer;
		typedef ::__impl::__iterator<pointer, basic_string> iterator;
		typedef ::__impl::__iterator<const_pointer, basic_string> const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		constexpr static size_type npos	= static_cast<size_type>(-1Z);
	protected:
		template<matching_input_iterator<value_type> IT> constexpr static size_type __ldist(IT a, IT b) { return b > a ? static_cast<size_type>(distance(a, b)) : 0UZ; }
		constexpr static int __size_compare(size_type lhs, size_type rhs) noexcept;
	public:
		constexpr pointer data() noexcept { return this->__beg(); }
		constexpr const_pointer data() const noexcept { return this->__beg(); }
		constexpr const_pointer c_str() const noexcept { return data(); }
		constexpr size_type size() const noexcept { return this->__size(); }
		constexpr size_type length() const noexcept { return size(); }
		constexpr size_type max_size() const noexcept { return this->__max_capacity(); }
		constexpr size_type capacity() const noexcept { return this->__capacity(); }
		constexpr allocator_type get_allocator() const noexcept { return this->__get_alloc(); }
		constexpr explicit basic_string(allocator_type const& alloc) noexcept : __base(alloc) {}
		constexpr basic_string() noexcept(noexcept(allocator_type())) : basic_string(allocator_type()) {}
		constexpr basic_string(size_type count, allocator_type const& alloc = allocator_type()) : __base(count + 1UZ, alloc) { this->__setc(0UZ); }
		constexpr basic_string(size_type count, const_reference value, allocator_type const& alloc);
		constexpr basic_string(size_type count, const_reference value) : basic_string(count, value, allocator_type()) {}
		template<matching_input_iterator<value_type> IT> constexpr basic_string(IT start, IT end, allocator_type const& alloc);
		template<matching_input_iterator<value_type> IT> constexpr basic_string(IT start, IT end) : basic_string(start, end, allocator_type()) {}
		template<__detail::__string_view_like<value_type> ST> constexpr basic_string(ST const& str_view, allocator_type const& alloc) : basic_string(str_view.begin(), str_view.end(), alloc) {}
		template<__detail::__string_view_like<value_type> ST> constexpr basic_string(ST const& str_view) : basic_string(str_view.begin(), str_view.end()) {}
		constexpr basic_string(const_pointer str, size_type count, allocator_type const& alloc);
		constexpr basic_string(const_pointer str, size_type count) : basic_string(str, count, allocator_type()) {}
		constexpr basic_string(const_pointer str, allocator_type const& alloc) : basic_string(str, traits_type::length(str), alloc) {}
		constexpr basic_string(const_pointer str) : basic_string(str, allocator_type()) {}
		constexpr basic_string(basic_string const& that, allocator_type const& alloc) : basic_string(that.data(), that.size(), alloc) {}
		constexpr basic_string(basic_string const& that) : basic_string(that, that.__get_alloc()) {}
		constexpr basic_string(basic_string&& that, allocator_type const& alloc) : __base(forward<__base>(that), alloc) {}
		constexpr basic_string(basic_string&& that) : __base(forward<__base>(that)) {}
		constexpr basic_string(basic_string const& that, size_type pos, allocator_type const& alloc) : basic_string(that.c_str() + pos, that.__cur(), alloc) {}
		constexpr basic_string(basic_string const& that, size_type pos) : basic_string(that, pos, that.__get_alloc()) {}
		constexpr basic_string(basic_string const& that, size_type pos, size_type count, allocator_type const& alloc) : basic_string(that.c_str() + pos, that.c_str() + pos + count, alloc) {}
		constexpr basic_string(basic_string const& that, size_type pos, size_type count) : basic_string(that, pos, count, that.__get_alloc()) {}
		constexpr basic_string(initializer_list<value_type> init, allocator_type const& alloc) : basic_string(init.begin(), init.end(), alloc) {}
		constexpr basic_string(initializer_list<value_type> init) : basic_string(init, allocator_type()) {}
		constexpr basic_string& operator=(basic_string const& that) { this->__copy_assign(that); return *this; }
		constexpr basic_string& operator=(basic_string&& that) { this->__move_assign(std::move(that)); return *this; }
		constexpr reference at(size_type i) { return this->__get(i); }
		constexpr const_reference at(size_type i) const { return this->__get(i); }
		constexpr reference operator[](size_type i) { return this->__get(i); }
		constexpr const_reference operator[](size_type i) const { return this->__get(i); }
		constexpr reference front() { return this->__get(0); }
		constexpr const_reference front() const { return this->__get(0); }
		constexpr reference back() { return this->__get_last(); }
		constexpr const_reference back() const { return this->__get_last(); }
		constexpr iterator begin() { return iterator(this->__beg()); }
		constexpr const_iterator cbegin() const { return const_iterator(this->__beg()); }
		constexpr const_iterator begin() const { return cbegin(); }
		constexpr iterator end() { return iterator(this->__cur()); }
		constexpr const_iterator cend() const { return const_iterator(this->__cur()); }
		constexpr const_iterator end() const { return cend(); }
		constexpr reverse_iterator rbegin() { return reverse_iterator(begin()); }
		constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(cbegin()); }
		constexpr const_reverse_iterator rbegin() const { return crbegin(); }
		constexpr reverse_iterator rend() { return reverse_iterator(end()); }
		constexpr const_reverse_iterator crend() const { return const_reverse_iterator(cend()); }
		constexpr const_reverse_iterator rend() const { return crend(); }
		constexpr bool empty() const noexcept { return this->__size() == 0; }
		constexpr void reserve(size_type count) { if(count > this->__capacity()) this->__grow_buffer(size_type(count - this->__capacity())); }
		constexpr void shrink_to_fit() { this->__trim_buffer(); }
		constexpr void clear() { this->__clear(); }
		constexpr iterator insert(const_iterator pos, const_reference value) { return iterator(this->__insert_element(pos.base(), value)); }
		template<std::matching_input_iterator<value_type> IT> constexpr iterator insert(const_iterator pos, IT start, IT end) { return iterator(this->__insert_elements(pos.base(), start, end)); }
		constexpr iterator insert(const_iterator pos, basic_string const& that) { return insert(pos, that.begin(), that.end()); }
		constexpr iterator insert(const_iterator pos, const_pointer value) { return insert(pos, basic_string(value)); }
		constexpr void push_back(const_reference value) { this->__append_element(value); }
		constexpr void pop_back() { this->__erase_at_end(1); }
		constexpr iterator erase(const_iterator pos) { return iterator(this->__erase(pos.base())); }
		constexpr iterator erase(const_iterator start, const_iterator end) { return iterator(this->__erase_range(start.base(), end.base())); }
		constexpr basic_string& replace(size_type pos, size_type count, const_pointer str, size_type count2) { this->__replace_elements(pos, (count < size() - pos ? count : size()), str, count2); return *this; }
		constexpr basic_string& replace(size_type pos, size_type count, basic_string const& that, size_type count2) { return replace(pos, count, that.data(), count2); }
		constexpr basic_string& replace(const_iterator first, const_iterator last, const_pointer str, size_t count) { this->__replace_elements(first.base(), last.base(), str, count); return *this; }
		constexpr basic_string& replace(const_iterator first, const_iterator last, basic_string const& that, size_t count) { return replace(first, last, that.data(), count); }
		constexpr basic_string& replace(size_type pos, size_type count, basic_string const& that) { return replace(pos, count, that, that.size()); }
		constexpr basic_string& replace(size_type pos, size_type count, const_pointer str) { return replace(pos, count, str, traits_type::length(str)); }
		constexpr basic_string& replace(const_iterator first, const_iterator last, basic_string const& that) { return replace(first, last, that, that.size()); }
		constexpr basic_string& replace(const_iterator first, const_iterator last, const_pointer str) { return replace(first, last, str, traits_type::length(str)); }
		constexpr basic_string& append(size_type count, value_type val) { this->__append_elements(count, val); return *this; }
		constexpr basic_string& append(const_pointer str, size_type count) { this->__append_elements(str, str + count); return *this; }
		constexpr basic_string& append(const_pointer str) { return append(str, traits_type::length(str)); }
		constexpr basic_string& append(value_type val) { return append(static_cast<size_type>(1UZ), val); }
		template<std::matching_input_iterator<value_type> IT> constexpr basic_string& append(IT start, IT end) { this->__append_elements(start, end); return *this; }
		constexpr basic_string& append(basic_string const& that) { this->__append_elements(that.begin(), that.end()); return *this; }
		constexpr basic_string& append(initializer_list<value_type> init) { return append(init.begin(), init.end()); }
		constexpr basic_string& operator+=(basic_string const& that) { return append(that); }
		constexpr basic_string& operator+=(const_pointer str) { return append(str); }
		constexpr basic_string& operator+=(value_type val) { return append(val); }
		constexpr void swap(basic_string& that) { this->__swap(that); }
		constexpr bool contains(value_type val) const noexcept { return this->find(val) != npos; }
		constexpr size_type find(basic_string const& that, size_type pos = 0) const noexcept { return this->find(that.data(), pos); }
		constexpr size_type find(const_pointer str, size_type pos, size_type count) const noexcept { return this->find(basic_string(str, count), pos); }
		constexpr size_type find(const_pointer str, size_type pos = 0) const noexcept
		{
			if(this->__out_of_range(this->__get_ptr(pos))) return npos;
			const_pointer result = traits_type::find(data() + pos, str);
			if(result) return static_cast<size_type>(result - data());
			return npos;
		}
		constexpr size_type find(value_type value, size_type pos = 0) const noexcept
		{
			if(this->__out_of_range(this->__get_ptr(pos))) return npos;
			const_pointer result	= traits_type::find(data() + pos, size() - pos, value);
			if(result) return static_cast<size_type>(result - data());
			return npos;
		}
		extension constexpr const_iterator find(const_pointer str, const_iterator pos) const noexcept
		{
			size_type result	= find(str, size_type(pos - begin()));
			if(result == npos) return end();
			return begin() + result;
		}
		extension constexpr const_iterator find(basic_string const& that, const_iterator pos) const noexcept
		{
			size_type result	= find(that, size_type(pos - begin()));
			if(result == npos) return end();
			return begin() + result;
		}
		extension constexpr const_iterator find(value_type val, const_iterator pos) const noexcept
		{
			size_type result	= find(val, size_type(pos - begin()));
			if(result == npos) return end();
			return begin() + result;
		}
		constexpr basic_string& append(basic_string const& that, size_type pos, size_type count = npos)
		{
			if(count == npos || that.begin() + pos + count > that.end())
				return append(that.begin() + pos, that.end());
			else return append(that.begin() + pos, that.begin() + pos + count);
		}
		constexpr int compare(basic_string const& that) const
		{
			const size_type __my_size	= this->size();
			const size_type __your_size	= that.size();
			const size_type __len		= std::min(__my_size, __your_size);
			int result					= traits_type::compare(this->data(), that.data(), __len);
			return (result != 0) ? result : __size_compare(__my_size, __your_size);
		}
		constexpr int compare(const_pointer that) const
		{
			const size_type __my_size	= this->size();
			const size_type __your_size	= traits_type::length(that);
			const size_type __len		= std::min(__my_size, __your_size);
			int result					= traits_type::compare(this->data(), that, __len);
			return (result != 0) ? result : __size_compare(__my_size, __your_size);
		}
		extension constexpr size_type count(value_type val) const
		{
			size_type result{};
			for(const_iterator i = begin(); i != end(); i++) { if(*i == val) result++; }
			return result;
		}
		extension constexpr size_type count_of(basic_string const& that) const
		{
			size_type result{};
			for(const_iterator i = that.begin(); i != that.end(); i++) result += count(*i);
			return result;
		}
		extension constexpr basic_string without_any_of(basic_string const& that) const
		{
			basic_string result(size() - count_of(that), get_allocator());
			for(const_iterator i = begin(); i != end(); i++) { if(!that.contains(*i)) result.append(*i); }
			return result;
		}
	};
	#define __str_template__ template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
	#define __str_itype__ basic_string<CT, TT, AT>
	__str_template__ constexpr int __lexical_cmp(__str_itype__ const& __this, __str_itype__ const& __that) noexcept { return __lex_compare(__this.data(), __this.data() + __this.size(), __that.data(), __that.data() + __that.size()); }
	__str_template__ constexpr __str_itype__ operator+(__str_itype__ const& __this, __str_itype__ const& __that) { __str_itype__ result(__this, __this.get_allocator()); result.append(__that); return result; }
	__str_template__ constexpr __str_itype__ operator+(const CT* __this, __str_itype__ const& __that) { __str_itype__ result(__this, __that.get_allocator()); result.append(__that); return result; }
	__str_template__ constexpr __str_itype__ operator+(__str_itype__ const& __this, const CT* __that) { __str_itype__ result(__this, __this.get_allocator()); result.append(__that); return result; }
	__str_template__ constexpr __str_itype__ operator+(__str_itype__ const& __this, CT __that) { __str_itype__ result(__this, __this.get_allocator()); result.append(__that); return result; }
	__str_template__ constexpr __str_itype__ operator+(CT __this, __str_itype__ const& __that) { __str_itype__ result(1UZ, __this, __that.get_allocator()); result.append(__that); return result; }
	__str_template__ constexpr bool operator==(__str_itype__ const& __this, __str_itype__ const& __that) noexcept { return strncmp(__this.data(), __that.data(), std::min(__this.size(), __that.size())) == 0; }
	__str_template__ constexpr bool operator==(__str_itype__ const& __this, const CT* __that) noexcept { return strncmp(__this.data(), __that, __this.size()) == 0; }
	__str_template__ constexpr bool operator==(const CT* __this, __str_itype__ const& __that) noexcept { return strncmp(__this, __that.data(), __that.size()) == 0; }
	__str_template__ constexpr auto operator<=>(__str_itype__ const& __this, __str_itype__ const& __that) noexcept -> decltype(__detail::__char_traits_cmp_cat<TT>(0)) { return __detail::__char_traits_cmp_cat<TT>(__this.compare(__that)); }
	__str_template__ constexpr auto operator<=>(__str_itype__ const& __this, const CT* __that) noexcept -> decltype(__detail::__char_traits_cmp_cat<TT>(0)) { return __detail::__char_traits_cmp_cat<TT>(__this.compare(__that)); }
	#undef __str_template__
	#undef __str_itype__
	template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
	constexpr int basic_string<CT, TT, AT>::__size_compare(size_type lhs, size_type rhs) noexcept
	{
		const difference_type d	= difference_type(lhs - rhs);
		if(d > numeric_limits<int>::max()) return numeric_limits<int>::max();
		else if (d < numeric_limits<int>::min()) return numeric_limits<int>::min();
		else return static_cast<int>(d);
	}
	template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
	constexpr basic_string<CT, TT, AT>::basic_string(size_type count, const_reference value, allocator_type const& alloc) : __base(count + 1UZ, alloc) {
		this->__set(data(), value, count);
		this->__setc(count);
	}
	template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
	constexpr basic_string<CT, TT, AT>::basic_string(const_pointer str, size_type count, allocator_type const& alloc) : __base(count + 1UZ, alloc) {
		traits_type::copy(this->data(), str, count);
		this->__setc(count);
	}
	template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
	template<matching_input_iterator<typename __impl::__dynamic_buffer<CT, AT, true>::__value_type> IT>
	constexpr basic_string<CT, TT, AT>::basic_string(IT start, IT end, allocator_type const& alloc) : __base(__ldist(start, end) + 1UZ, alloc) {
		this->__transfer(data(), start, end);
		this->__setc(__ldist(start, end));
	}
}
#endif