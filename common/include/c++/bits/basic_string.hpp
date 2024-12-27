#ifndef __BASIC_STRING
#define __BASIC_STRING
#include "bits/dynamic_buffer.hpp"
#include "bits/stl_iterator.hpp"
#include "bits/ios_base.hpp"
#include "bits/stl_algobase.hpp"
namespace std
{
    namespace __detail
    {
        template<typename T> struct __is_totally_ordered : false_type {};
        template<std::totally_ordered T> struct __is_totally_ordered<T> : true_type {};
        template<typename CT> struct __is_char_type : __and_<__or_<is_trivial<CT>, is_standard_layout<CT>>, __not_<is_array<CT>>, __is_totally_ordered<CT>> {};
        template<typename CT> constexpr inline bool __is_char_type_v = __is_char_type<CT>::value;
    }
    template<typename CT> concept char_type = __detail::__is_char_type_v<CT>;
    namespace __impl
    {
        template<std::char_type CT> constexpr const CT* __find_impl(CT const* hs, CT const* ne) noexcept
        {
            CT c = ne[0];
            if (!c) return hs;
            for (; hs[0]; hs++)
            {
                if (hs[0] != c) continue;
                size_t i;
                for (i = 1; ne[i]; i++) if (hs[i] != ne[i]) break;
                if (!ne[i]) return hs;
            }
            return NULL;
        }
    }
    template<std::char_type CT> constexpr CT* find(const CT* ptr, size_t n, CT c) 
    { 
        if constexpr(sizeof(CT) == 1) asm volatile("repne scasb" : "+D"(ptr) : "a"(c), "c"(n) : "memory");
        else if constexpr(sizeof(CT) == 2) asm volatile("repne scasw" : "+D"(ptr) : "a"(c), "c"(n) : "memory");
        else if constexpr(sizeof(CT) == 4) asm volatile("repne scasl" : "+D"(ptr) : "a"(c), "c"(n) : "memory");
        else if constexpr(sizeof(CT) == 8) asm volatile("repne scasq" : "+D"(ptr) : "a"(c), "c"(n) : "memory");
        else for(size_t i = 0; i < n && *ptr != c; i++, ++ptr);
        return const_cast<CT*>(ptr);
    }
    template<std::char_type CT> constexpr size_t strnlen(const CT* str, size_t max) { return size_t(std::find(str, max, CT(0)) - str); }
    template<std::char_type CT> constexpr size_t strlen(const CT* str) { return strnlen(str, size_t(-1)); }
    template<std::integral T> constexpr inline void* memset(void* ptr, T val, size_t n) { arrayset(ptr, val, n); return ptr; }
    template<std::char_type CT> constexpr CT* memset(CT* ptr, CT c, size_t n) { arrayset<CT>(ptr, c, n); return ptr; }
    template<std::char_type CT> constexpr void assign(CT& c1, CT const& c2) { c1 = c2; }
    template<std::char_type CT> constexpr CT to_char_type(int i) { return static_cast<CT>(i); }
    template<std::char_type CT> constexpr int to_int_type(CT c) { return static_cast<int>(c); }
    template<std::char_type CT> constexpr bool lt(CT a, CT b) { return a < b; }
    template<> constexpr bool lt<char>(char a, char b) { return static_cast<unsigned char>(a) < static_cast<unsigned char>(b); }
    template<std::char_type CT> constexpr bool eq(CT a, CT b) { return a == b; }
    template<> constexpr bool eq<char>(char a, char b) { return static_cast<unsigned char>(a) == static_cast<unsigned char>(b); }
    template<std::char_type CT> constexpr CT* strcpy(CT* dest, const CT* src) { arraycopy<CT>(dest, src, std::strlen(src)); }
    template<std::char_type CT> constexpr CT* strncpy(CT* dest, const CT* src, size_t n) { arraycopy<CT>(dest, src, std::strnlen(src, n)); return dest; }
    constexpr int memcmp(const void* s1, const void* s2, size_t n) { return __builtin_memcmp(s1, s2, n); }
    template<std::char_type CT> constexpr int strncmp(const CT* s1, const CT* s2, size_t n) 
    { 
        bool lt, gt;
        if constexpr(sizeof(CT) == 2) asm volatile(" repe cmpsw \n setl %1 \n setg %2 " : "+D"(s1), "=g"(lt), "=g"(gt) : "S"(s2), "c"(n) : "memory");
        else if constexpr(sizeof(CT) == 4) asm volatile(" repe cmpsl \n setl %1 \n setg %2 " : "+D"(s1), "=g"(lt), "=g"(gt) : "S" (s2), "c"(n) : "memory");
        else if constexpr(sizeof(CT) == 8) asm volatile(" repe cmpsq \n setl %1 \n setg %2 " : "+D"(s1), "=g"(lt), "=g"(gt) : "S"(s2), "c"(n) : "memory");
        else asm volatile(" repe cmpsb \n setl %1 \n setg %2 " : "+D"(s1), "=g"(lt), "=g"(gt) : "S"(s2), "c"(n * sizeof(CT)) : "memory");
        return lt ? -1 : gt ? 1 : 0;
    }
    template<std::char_type CT> constexpr int strcmp(const CT* s1, const CT* s2) { return std::strncmp(s1, s2, std::strlen(s1)); }
    template<std::char_type CT> constexpr const CT* find(const CT* ptr, const CT* what) noexcept { return __impl::__find_impl(ptr, what); }
    typedef int64_t streamoff;
    template<typename ST>
    class fpos
    {
        ST __state{};
        streamoff __offs{0};
    public:
        ST state() const { return __state; }
        void state(ST st) { __state = st; }
        fpos() = default;
        fpos(streamoff offs) : __state{}, __offs{offs}{}
        fpos(fpos<ST> const&) = default;
        fpos(fpos<ST>&&) = default;
        fpos<ST>& operator=(fpos<ST> const&) = default;
        fpos<ST>& operator=(fpos<ST>&&) = default;
        operator streamoff() const {return __offs;}
        bool operator==(fpos const& that){ return this->__state == that.__state && this->__offs == that.__offs; }
        bool operator!=(fpos const& that){ return !(*this == that); }
        fpos<ST>& operator+=(streamoff off) { __offs += off; return *this; }
        fpos<ST>& operator-=(streamoff off) { __offs -= off; return *this; }
        fpos<ST> operator+(streamoff off) const { fpos<ST> r{*this}; r += off; return r; }
        fpos<ST> operator-(streamoff off) const { fpos<ST> r{*this}; r -= off; return r; }
        streamoff operator-(fpos<ST> const& that) const { return this->__offs - that.__offs; }
        template<typename PT>
        friend PT* operator+(PT* you, fpos<ST> const& me) { return (you + me.__offs); }
    };
    template<typename ST>
    fpos<ST> operator+(streamoff off, fpos<ST> const& p) { return p + off; }
    typedef fpos<mbstate_t> fpos_t;
    template<std::char_type CT>
    struct char_traits
    {
        typedef CT char_type;
        typedef int int_type;
        typedef streamoff off_type;
        typedef mbstate_t state_type;
        typedef fpos<mbstate_t> pos_type;
        static char_type* assign(char_type* ptr, char_type c, size_t n) { if(!n) return ptr; return std::memset(ptr, c, n); }
        static void assign(char_type& c1, char_type const& c2) noexcept { c1 = c2; }
        static bool eq(char_type a, char_type b) noexcept { return a == b; }
        static bool lt(char_type a, char_type b) noexcept { return a < b; }
        static char_type const* find(const char_type* ptr, size_t n, char_type const& c) noexcept { return std::find(ptr, n, c); }
        static char_type const* find(const char_type* ptr, const char_type* what) noexcept { return std::find(ptr, what); }
        static int_type to_int_type(char_type c) noexcept { return static_cast<int>(c); }
        static char_type to_char_type(int_type i) noexcept { return static_cast<char_type>(i);}
        static char_type* copy(char_type* dest, const char_type* src, size_t n) { return std::strncpy(dest, src, n); }
        static char_type* move(char_type* dest, char_type* src, size_t n) { std::strncpy(dest, src, n); std::memset(src, 0, n); return dest; }
        static size_t length(const char_type* str) noexcept { return std::strlen(str); }
        static int_type eof() noexcept { return -1; }
        static int_type not_eof(int_type e) noexcept { return e > 0 ? e : (e * -1); }
    };
    typedef char_traits<char>::pos_type streampos;
    template<typename TT, typename CT> concept char_traits_type = std::char_type<CT> && requires(CT& c1, CT const& c2, CT* ptr, CT* pt2, size_t n)
    {
        { typename TT::char_type{} } -> std::same_as<CT>;
        { typename TT::int_type{} } -> std::integral;
        { typename TT::off_type{} } -> std::signed_integral;
        { TT::assign(ptr, c2, n) } -> std::same_as<CT*>;
        { TT::move(ptr, pt2, n) } -> std::same_as<CT*>;
        { TT::copy(ptr, pt2, n) } -> std::same_as<CT*>;
        { TT::find(ptr, c2, n) } -> std::same_as<const CT*>;
        { TT::to_int_type(c2) } -> std::same_as<typename TT::int_type>;
        { TT::to_char_type(0) } -> std::same_as<CT>;
        { TT::find(ptr, pt2) } -> std::same_as<const CT*>; 
        { TT::length(ptr) } -> std::same_as<decltype(sizeof(CT))>;
        { TT::lt(c1, c2) } -> std::__detail::__boolean_testable;
        { TT::eq(c1, c2) } -> std::__detail::__boolean_testable;
        { TT::eof() } -> std::signed_integral;
        { TT::eof() } -> std::integral;
        TT::assign(c1, c2);
        typename TT::state_type;
        typename TT::pos_type;
    };
    template<char_type CT, char_traits_type<CT> TT = std::char_traits<CT>, allocator_object<CT> AT = std::allocator<CT>>
    class basic_string : protected std::__impl::__dynamic_buffer<CT, AT>
    {
        typedef __impl::__dynamic_buffer<CT, AT> __base;
    public:
        typedef TT traits_type;
        typedef CT value_type;
        typedef AT allocator_type;
        typedef decltype(sizeof(CT)) size_type;
        typedef decltype(declval<CT*>() - declval<CT*>()) difference_type;
        typedef CT& reference;
        typedef CT const& const_reference;
        typedef CT* pointer;
        typedef CT const* const_pointer;
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
    public:
        constexpr pointer data() noexcept { return this->__access(); }
        constexpr const_pointer data() const noexcept { return this->__access(); }
        constexpr const_pointer c_str() const noexcept { return data(); }
        constexpr size_type size() const noexcept { return this->__size(); }
        constexpr size_type length() const noexcept { return size(); }
        constexpr size_type max_size() const noexcept { return 0xFFFFFFFFFFFFFFFFUL; }
        constexpr size_type capacity() const noexcept { return this->__capacity(); }
        constexpr allocator_type get_allocator() const noexcept { return this->__allocator; }
        constexpr explicit basic_string(allocator_type const& alloc) noexcept : __base { 1, alloc } {}
        constexpr basic_string() noexcept(noexcept(allocator_type())) : basic_string { allocator_type() } {}
        constexpr basic_string(size_type count, allocator_type const& alloc = allocator_type{}) : __base{count + 1, alloc} {}
        constexpr basic_string(size_type count, value_type value, allocator_type const& alloc = allocator_type{}) : __base{ count + 1, alloc } { __set(this->__access(), value, count); this->__advance(count); }
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
        constexpr explicit basic_string(std::initializer_list<value_type> init, allocator_type const& alloc = allocator_type{}) : __base{ init, alloc } {}
        constexpr reference at(size_type i) { return this->__get(i); }
        constexpr const_reference at(size_type i) const { return this->__get(i); }
        constexpr reference operator[](size_type i) { return this->__get(i); }
        constexpr const_reference operator[](size_type i ) const { return this->__get(i); }
        constexpr reference front() { return this->__get(0); }
        constexpr const_reference front() const { return this->__get(0); }
        constexpr reference back() { return this->__get_last(); }
        constexpr const_reference back() const { return this->__get_last(); }
        constexpr iterator begin() { return iterator{ this->__access() }; }
        constexpr const_iterator cbegin() const { return const_iterator{ this->__access() }; }
        constexpr const_iterator begin() const { return cbegin(); }
        constexpr iterator end() { return iterator{ this->__access_end() }; }
        constexpr const_iterator cend() const { return const_iterator{ this->__access_end() }; }
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
        constexpr size_type find(const_pointer str, size_type pos = 0) const noexcept { const_pointer result = traits_type::find(data() + pos, str); if(result) return size_type(result - data()); return npos; }
        constexpr size_type find(basic_string const& that, size_type pos = 0) const noexcept { return find(that.data(), pos); }
        constexpr size_type find(const_pointer str, size_type pos, size_type count) const noexcept { return find(basic_string{ str, count }, pos); }
        constexpr size_type find(value_type value, size_type pos = 0) const noexcept { const_pointer result = traits_type::find(data() + pos, size() - pos, value); }
    };
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr int __lexical_cmp(basic_string<CT, TT, AT> const& __this, basic_string<CT, TT, AT> const& __that) noexcept { return __lex_compare(__this.data(), __this.data() + __this.size(), __that.data(), __that.data() + __that.size()); }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr basic_string<CT, TT, AT> operator+(basic_string<CT, TT, AT> const& __this, basic_string<CT, TT, AT> const& __that) { basic_string<CT, TT, AT> result{ __this }; result.append(__that); return result; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr basic_string<CT, TT, AT> operator+(const CT* __this, basic_string<CT, TT, AT> const& __that) { return basic_string<CT, TT, AT>{ __this, __that.get_allocator() } + __that; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr basic_string<CT, TT, AT> operator+(basic_string<CT, TT, AT> const& __this, const CT* __that) { return __this + basic_string<CT, TT, AT>{ __that, __this.get_allocator() }; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr basic_string<CT, TT, AT> operator+(basic_string<CT, TT, AT> const& __this, CT __that) { return __this + basic_string<CT, TT, AT>{ basic_string<CT, TT, AT>::size_type(1), __that, __this.get_allocator() }; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr basic_string<CT, TT, AT> operator+(CT __this, basic_string<CT, TT, AT> const& __that) { return basic_string<CT, TT, AT>{ basic_string<CT, TT, AT>::size_type(1), __this, __that.get_allocator()  } + __that; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr bool operator==(basic_string<CT, TT, AT> const& __this, basic_string<CT, TT, AT> const& __that) noexcept { return strncmp(__this.data(), __that.data(), std::min(__this.size(), __that.size())) == 0; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr bool operator<(basic_string<CT, TT, AT> const& __this, basic_string<CT, TT, AT> const& __that) noexcept { return __lexical_cmp(__this, __that) < 0; }
    template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> constexpr bool operator>(basic_string<CT, TT, AT> const& __this, basic_string<CT, TT, AT> const& __that) noexcept { return __lexical_cmp(__this, __that) > 0; }
}
#endif