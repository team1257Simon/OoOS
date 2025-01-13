#ifndef __BASIC_STRING
#define __BASIC_STRING
#include "bits/dynamic_buffer.hpp"
#include "bits/char_traits.hpp"
namespace std
{
    template<char_type CT, char_traits_type<CT> TT = std::char_traits<CT>, allocator_object<CT> AT = std::allocator<CT>>
    class basic_string : protected __impl::__dynamic_buffer<CT, AT>
    {
        typedef __impl::__dynamic_buffer<CT, AT> __base;
    public:
        typedef TT traits_type;
        typedef CT value_type;
        typedef typename __base::__alloc_type allocator_type;
        typedef typename __base::__size_type size_type;
        typedef typename __base::__diff_type difference_type;
        typedef typename __base::__ref reference;
        typedef typename __base::__const_ref const_reference;
        typedef typename __base::__ptr pointer;
        typedef typename __base::__const_ptr const_pointer;
        typedef ::__impl::__iterator<pointer, basic_string> iterator;
        typedef ::__impl::__iterator<const_pointer, basic_string> const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        constexpr static size_type npos = size_type(-1);
    protected:
        virtual void __set(CT* where, CT const& val, size_t n) override { traits_type::assign(where, val, n); }
        virtual void __zero(CT* where, size_t n) override { traits_type::assign(where, 0, n); }
        virtual void __copy(CT* where, CT const* src, size_t n) override { traits_type::copy(where, src, n); }
        virtual void __on_modify() override { if(this->__size() == this->__capacity()) this->__grow_buffer(1); }
        constexpr static int __size_compare(size_type lhs, size_type rhs) noexcept
        {
            const difference_type d = difference_type(lhs - rhs);
            if(d > std::numeric_limits<int>::max()) return std::numeric_limits<int>::max();
            else if (d < std::numeric_limits<int>::min()) return std::numeric_limits<int>::min();
            else return int(d);
        }
    public:
        constexpr pointer data() noexcept { return this->__beg(); }
        constexpr const_pointer data() const noexcept { return this->__beg(); }
        constexpr const_pointer c_str() const noexcept { return data(); }
        constexpr size_type size() const noexcept { return this->__size(); }
        constexpr size_type length() const noexcept { return size(); }
        constexpr size_type max_size() const noexcept { return this->__max_capacity(); }
        constexpr size_type capacity() const noexcept { return this->__capacity(); }
        constexpr allocator_type get_allocator() const noexcept { return this->__allocator; }
        constexpr explicit basic_string(allocator_type const& alloc) noexcept : __base { 1, alloc } {}
        constexpr basic_string() noexcept(noexcept(allocator_type())) : basic_string { allocator_type() } {}
        constexpr basic_string(size_type count, allocator_type const& alloc = allocator_type{}) : __base{count + 1, alloc} {}
        constexpr basic_string(size_type count, value_type value, allocator_type const& alloc = allocator_type{}) : __base{ count + 1, alloc } { __set(this->__beg(), value, count); this->__advance(count); }
        template<std::matching_input_iterator<value_type> IT> constexpr basic_string(IT start, IT end, allocator_type const& alloc = allocator_type{}) : __base{ size_type(end - start + 1), alloc } { this->__transfer(data(), start, end); this->__advance(size_t(end - start)); }
        constexpr basic_string(const_pointer str, size_type count, allocator_type const& alloc = allocator_type{}) : basic_string{ str, str + count, alloc } {}
        constexpr basic_string(const_pointer str, allocator_type const& alloc = allocator_type{}) : basic_string{ str, traits_type::length(str), alloc } {}
        constexpr basic_string(basic_string const& that) : __base{ static_cast<__base const&>(that) } {}
        constexpr basic_string(basic_string&& that) : __base{ forward<__base>(that) } {}
        constexpr basic_string(basic_string const& that, allocator_type const& alloc) : __base{ static_cast<__base const&>(that), alloc } {}
        constexpr basic_string(basic_string&& that, allocator_type const& alloc) : __base{ forward<__base>(that), alloc } {}
        constexpr basic_string(basic_string const& that, size_type pos, allocator_type const& alloc = allocator_type{}) : __base{ static_cast<__base const&>(that), pos, alloc } {}
        constexpr basic_string(basic_string const& that, size_type pos, size_type count, allocator_type const& alloc = allocator_type{}) : __base{ static_cast<__base const&>(that), pos, count, alloc } {}
        constexpr ~basic_string() { this->__destroy(); }
        constexpr basic_string& operator=(basic_string const& that) { this->__clear(); this->__allocate_storage(that.size() + 1); this->__copy(this->data(), that.data(), that.size()); this->__advance(that.size()); return *this; }
        constexpr basic_string& operator=(basic_string&& that) {  this->__clear(); this->__allocate_storage(that.size() + 1); this->__copy(this->data(), that.data(), that.size()); this->__advance(that.size()); that.__clear(); return *this; }
        constexpr explicit basic_string(std::initializer_list<value_type> init, allocator_type const& alloc = allocator_type{}) : __base{ init, alloc } {}
        constexpr reference at(size_type i) { return this->__get(i); }
        constexpr const_reference at(size_type i) const { return this->__get(i); }
        constexpr reference operator[](size_type i) { return this->__get(i); }
        constexpr const_reference operator[](size_type i ) const { return this->__get(i); }
        constexpr reference front() { return this->__get(0); }
        constexpr const_reference front() const { return this->__get(0); }
        constexpr reference back() { return this->__get_last(); }
        constexpr const_reference back() const { return this->__get_last(); }
        constexpr iterator begin() { return iterator{ this->__beg() }; }
        constexpr const_iterator cbegin() const { return const_iterator{ this->__beg() }; }
        constexpr const_iterator begin() const { return cbegin(); }
        constexpr iterator end() { return iterator{ this->__cur() }; }
        constexpr const_iterator cend() const { return const_iterator{ this->__cur() }; }
        constexpr const_iterator end() const { return cend(); }
        constexpr reverse_iterator rbegin() { return reverse_iterator{ begin() }; }
        constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator{ cbegin() }; }
        constexpr const_reverse_iterator rbegin() const { return crbegin(); }
        constexpr reverse_iterator rend() { return reverse_iterator{ end() }; }
        constexpr const_reverse_iterator crend() const { return const_reverse_iterator{ cend() }; }
        constexpr const_reverse_iterator rend() const { return crend(); }
        constexpr bool empty() const noexcept { return this->__size() == 0; }
        constexpr void reserve(size_type count) { if(count > this->__capacity()) this->__grow_buffer(size_type(count - this->__capacity())); }
        constexpr void shrink_to_fit() { this->__trim_buffer(); }
        constexpr void clear() { this->__clear(); }
        constexpr iterator insert(const_iterator pos, const_reference value) { return iterator { this->__insert_element(pos.base(), value) }; }
        template<std::matching_input_iterator<value_type> IT> constexpr iterator insert(const_iterator pos, IT start, IT end) { return iterator{ this->template __insert_elements<IT>(pos.base(), start, end) }; }
        constexpr iterator insert(const_iterator pos, basic_string const& that) { return insert(pos, that.begin(), that.end()); }
        constexpr iterator insert(const_iterator pos, const_pointer value) { return insert(pos, basic_string { value }); }
        constexpr void push_back(const_reference value) { this->__append_element(value); }
        constexpr void pop_back() { this->__erase_at_end(1); }
        constexpr iterator erase(const_iterator pos) { return iterator { this->__erase(pos.base()) }; }
        constexpr iterator erase(const_iterator start, const_iterator end) { return iterator { this->__erase_range(start.base(), end.base()) }; }
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
        constexpr basic_string& append(value_type val) { this->__append_element(val); return *this; }
        constexpr basic_string& append(basic_string const& that) { return append(that.data(), that.size()); }
        template<std::matching_input_iterator<value_type> IT> constexpr basic_string& append(IT start, IT end) { this->template __append_elements<IT>(start, end); return *this; }
        constexpr basic_string& append(initializer_list<value_type> init) { return append(init.begin(), init.end()); }
        constexpr basic_string& append(basic_string const& that, size_type pos, size_type count = npos) { if(count == npos || that.begin() + pos + count > that.end()) return append(that.begin() + pos, that.end()); else return append(that.begin() + pos, that.begin() + pos + count); }
        constexpr basic_string& operator+=(basic_string const& that) { return append(that); }
        constexpr basic_string& operator+=(const_pointer str) { return append(str); }
        constexpr basic_string& operator+=(value_type val) { return append(val); }
        constexpr void swap(basic_string& that) { this->__swap(that); }
        constexpr size_type find(const_pointer str, size_type pos = 0) const noexcept { const_pointer result = traits_type::find(data() + pos, str); if(result) { return size_type(result - data()); } return npos; }
        constexpr size_type find(basic_string const& that, size_type pos = 0) const noexcept { return find(that.data(), pos); }
        constexpr size_type find(const_pointer str, size_type pos, size_type count) const noexcept { return find(basic_string{ str, count }, pos); }
        constexpr size_type find(value_type value, size_type pos = 0) const noexcept { const_pointer result = traits_type::find(data() + pos, size() - pos, value); if(result)  { return size_type(result - data()); } return npos; }
        constexpr int compare(basic_string const& that) const { const size_type __my_size = this->size(); const size_type __your_size = that.size(); const size_type __len = std::min(__my_size, __your_size); int result = traits_type::compare(this->data(), that.data(), __len); return (result != 0) ? result : __size_compare(__my_size, __your_size); }
        constexpr int compare(const_pointer that) const { const size_type __my_size = this->size(); const size_type __your_size = traits_type::length(that); const size_type __len = std::min(__my_size, __your_size); int result = traits_type::compare(this->data(), that, __len); return (result != 0) ? result : __size_compare(__my_size, __your_size); }
    };
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr int __lexical_cmp(basic_string<CT, TT, AT> const& __this, basic_string<CT, TT, AT> const& __that) noexcept { return __lex_compare(__this.data(), __this.data() + __this.size(), __that.data(), __that.data() + __that.size()); }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr basic_string<CT, TT, AT> operator+(basic_string<CT, TT, AT> const& __this, basic_string<CT, TT, AT> const& __that) { basic_string<CT, TT, AT> result{ __this }; result.append(__that); return result; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr basic_string<CT, TT, AT> operator+(const CT* __this, basic_string<CT, TT, AT> const& __that) { return basic_string<CT, TT, AT>{ __this, __that.get_allocator() } + __that; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr basic_string<CT, TT, AT> operator+(basic_string<CT, TT, AT> const& __this, const CT* __that) { return __this + basic_string<CT, TT, AT>{ __that, __this.get_allocator() }; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr basic_string<CT, TT, AT> operator+(basic_string<CT, TT, AT> const& __this, CT __that) { return __this + basic_string<CT, TT, AT>{ basic_string<CT, TT, AT>::size_type(1), __that, __this.get_allocator() }; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr basic_string<CT, TT, AT> operator+(CT __this, basic_string<CT, TT, AT> const& __that) { return basic_string<CT, TT, AT>{ basic_string<CT, TT, AT>::size_type(1), __this, __that.get_allocator()  } + __that; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr bool operator==(basic_string<CT, TT, AT> const& __this, basic_string<CT, TT, AT> const& __that) noexcept { return strncmp(__this.data(), __that.data(), std::min(__this.size(), __that.size())) == 0; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> inline auto operator<=>(basic_string<CT, TT, AT> const& __this, basic_string<CT, TT, AT> const& __that) noexcept -> decltype(__detail::__char_traits_cmp_cat<TT>(0)) { return __detail::__char_traits_cmp_cat<TT>(__this.compare(__that)); }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> inline auto operator<=>(basic_string<CT, TT, AT> const& __this, const CT* __that) noexcept -> decltype(__detail::__char_traits_cmp_cat<TT>(0)) { return __detail::__char_traits_cmp_cat<TT>(__this.compare(__that)); }
}
#endif