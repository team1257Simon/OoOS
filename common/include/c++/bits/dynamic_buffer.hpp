/**
 * Base structs for the contiguous STL containers such as vector, streambuf, and string, implementing a lot of the shared functionality of those types.
 * Like much of the namespace std the libk will see, there are things here that are not constexpr that normally would be. 
 * Calls to the heap allocator, for instance, rely on the kernel frame pointer which resides somewhere that can't be constexpr.
 */
#ifndef __DYN_BUFFER
#define __DYN_BUFFER
#include "memory"
#include "limits"
#include "bits/stl_iterator.hpp"
#include "kernel/libk_decls.h"
#include "initializer_list"
namespace std::__impl
{
    /**
     * This contains the pointers to the first and last elements of a buffer as well as an "end" pointer that represents either the current end of the data
     * or the current position of read/write operations.
     */
    template<typename T>
    struct __buf_ptrs
    {
        typedef T* __ptr_type;
        typedef T const* __const_ptr_type;
        __ptr_type __begin{};
        __ptr_type __end{};
        __ptr_type __max{};
        constexpr __buf_ptrs() = default;
        constexpr __buf_ptrs(__ptr_type start, __ptr_type end, __ptr_type max) : __begin{ start }, __end{ end }, __max{ max } {}
        void __reset() { __begin = __end = __max = __ptr_type{}; }
        __buf_ptrs(__buf_ptrs const& that) : __begin{ that.__begin }, __end{ that.__end }, __max{ that.__max } {}
        __buf_ptrs(__buf_ptrs&& that) : __begin{ that.__begin }, __end{ that.__end }, __max{ that.__max } { that.__reset(); }
        __buf_ptrs(__ptr_type start, __ptr_type end) : __begin{ start }, __end{ end }, __max { end } {}
        __buf_ptrs(__ptr_type start, size_t cap) : __begin{ start }, __end{ start }, __max{ start + cap } {}
        void __copy_ptrs(__buf_ptrs const& that) { this->__begin = that.__begin; this->__end = that.__end; this->__max = that.__max; }
        void __move_ptrs(__buf_ptrs&& that) { this->__copy_ptrs(that); that.__reset(); }
        void __swap_ptrs(__buf_ptrs& that) { __buf_ptrs tmp; tmp.__copy_ptrs(*this); this->__copy_ptrs(that); that.__copy_ptrs(tmp); }
        void __set_ptrs(__ptr_type begin, __ptr_type end, __ptr_type max) { this->__begin = begin; this->__end = end; this->__max = max; }
        __ptr_type __getptr(size_t offs) { return __begin + offs; }
        void __setc(__ptr_type where) { __end = where; }
        void __setc(size_t offs) { __end = __begin + offs; }
        void __adv(size_t n) { __end += n; }
        void __bck(size_t n) { __end -= n; }
    };
    /**
     * This base-type implements the functionality shared by the dynamic-container types (mainly string and vector). 
     * Places where a string differs from a vector of chars are handled using virtual functions that can be overridden for the finer details.
     */
    template<typename T, allocator_object<T> A>
    struct __dynamic_buffer
    {
        A __allocator;
        typedef T* __ptr;
        typedef T const* __const_ptr;
        typedef T& __ref;
        typedef T const& __const_ref;
        __buf_ptrs<T> __my_data;
        template<std::matching_input_iterator<T> IT> void __transfer(T* where, IT start, IT end) { for(IT i = start; i != end; i++, where++) { *where = *i; } }
        virtual void __set(__ptr where, T const& val, size_t n) { arrayset<T>(where, val, n); }
        virtual void __zero(__ptr where, size_t n)  { if constexpr(is_integral_v<T>) arrayset<T>(where, 0, n); else for(size_t i = 0; i < n; i++, (void)++where) { where->~T(); } }
        virtual void __copy(__ptr where, __const_ptr src, size_t n) { arraycopy<T>(where, src, n); }
        /**
         * Called whenever the end and/or max pointers are changed after initial construction, other than through the advance and backtrack hooks.
         * Inheritors can override to add functionality that needs to be invoked whenever these pointers move.
         * The default implementation does nothing.
         */
        virtual void __on_modify() {}
        virtual void __grow_buffer(size_t added);
        template<std::matching_input_iterator<T> IT> void __append_elements(IT start_it, IT end_it);
        void __append_elements(__const_ptr start_ptr, __const_ptr end_ptr);
        __ptr __insert_elements(__const_ptr pos, __const_ptr start_ptr, __const_ptr end_ptr);
        template<matching_input_iterator<T> IT> __ptr __insert_elements(__const_ptr pos, IT start_ptr, IT end_ptr);
        template<typename ... Args> requires constructible_from<T, Args...> __ptr __emplace_element(__const_ptr pos, Args&& ... args);
        __ptr __insert_element(__const_ptr pos, T const& t) { return __insert_elements(pos, __builtin_addressof(t), __builtin_addressof(t) + 1); }
        constexpr __ptr __beg() noexcept { return __my_data.__begin; }
        constexpr __const_ptr __beg() const noexcept { return __my_data.__begin; }
        constexpr __ptr __cur() noexcept { return __my_data.__end; }
        constexpr __const_ptr __cur() const noexcept { return __my_data.__end; }
        constexpr __ptr __max() noexcept { return __my_data.__max; }
        constexpr __const_ptr __max() const noexcept { return __my_data.__max; }
        constexpr size_t __diff(__const_ptr pos) const noexcept { return size_t(pos - __beg()); }
        constexpr ptrdiff_t __needed(__const_ptr pos) const noexcept { return pos - __max(); }
        constexpr bool __valid_end_pos(__const_ptr pos) const noexcept { return pos >= __beg() && pos <= __max(); }
        constexpr bool __out_of_range(__const_ptr pos) const noexcept { return pos < __beg() || pos >= __max(); }
        constexpr bool __out_of_range(__const_ptr start, __const_ptr end) const noexcept { return __out_of_range(start) || __out_of_range(end) || end < start; }
        constexpr __ptr __getptr(size_t __i) noexcept { return __beg() + __i; }
        constexpr __const_ptr __getptr(size_t __i) const noexcept { return __beg() + __i; }
        constexpr __ref __get(size_t __i) { return *__getptr(__i); }
        constexpr __const_ref __get(size_t __i) const { return *__getptr(__i); }
        constexpr __ref __get_last() noexcept { return *__cur(); }
        constexpr __const_ref __get_last() const noexcept { return *__cur(); }
        constexpr void __setp(__ptr beg, __ptr cur, __ptr end) noexcept { __my_data.__set_ptrs(beg, cur, end); }
        constexpr void __setp(__ptr beg, __ptr end) noexcept { __setp(beg, beg, end); }
        constexpr void __setn(__ptr beg, size_t c, size_t n) noexcept { __setp(beg, beg + c, beg + n); }
        constexpr void __setn(__ptr beg, size_t n) noexcept { __setp(beg, beg + n); }
        constexpr void __setc(__ptr pos) noexcept { if(__valid_end_pos(pos)) __my_data.__setc(pos); } 
        constexpr void __setc(size_t pos) noexcept { __setc(__getptr(pos)); }
        constexpr size_t __max_capacity() const noexcept { return std::numeric_limits<size_t>::max(); }
        inline void __advance(size_t n) { if(__valid_end_pos(__cur() + n)) { __my_data.__adv(n); } else { __setc(__max()); } }
        inline void __backtrack(size_t n) { if(__valid_end_pos(__cur() - n)) { __my_data.__bck(n); } else {__setc(__beg()); } }
        inline size_t __size() const noexcept { return size_t(__cur() - __beg()); }
        inline size_t __capacity() const noexcept { return size_t(__max() - __beg()); }
        inline size_t __rem() const { return size_t(__max() - __cur()); }
        inline size_t __ediff(__const_ptr pos) { return size_t(__cur() - pos); }
        void __trim_buffer() { size_t num_elements = __size(); __setn(resize<T>(__beg(), num_elements), num_elements, num_elements); this->__on_modify(); }
        void __allocate_storage(size_t n) { __setn(__allocator.allocate(n), n); }
        void __construct_element(__ptr pos, T const& t) { if(!__out_of_range(pos)) { construct_at(pos, t); if(pos > __cur()) __setc(pos); }  }
        void __assign_elements(size_t count, T const& t) { if(count > __capacity()) __grow_buffer(count - __capacity()); __set(__beg(), t, count); if (count < __size()) { __zero(__getptr(count), __size() - count); } __setc(count); }
        void __assign_elements(__const_ptr start, __const_ptr end) { size_t count = end - start; if(count > __capacity()) __grow_buffer(count - __capacity()); __copy(__beg(), start, count); if (count < __size()) { __zero(__getptr(count), __size() - count); } __setc(count); }
        void __replace_elements(size_t pos, size_t count, __ptr from, size_t count2);
        void __replace_elements(__const_ptr start, __const_ptr end, __ptr from, size_t count) { if(!__out_of_range(start, end)) __replace_elements(start - __beg(), end - start, from, count); }
        void __assign_elements(std::initializer_list<T> ini) { __assign_elements(ini.begin(), ini.end()); }
        void __append_elements(size_t count, T const& t) { if(__max() < __cur() + count) this->__grow_buffer(size_t(count - __rem())); for(size_t i = 0; i < count; i++, __advance(1)) construct_at(__cur(), t); this->__on_modify(); }
        void __append_element(T const& t) { if(!(__max() > __cur())) { __grow_buffer(1); } construct_at(__cur(), t); __advance(1); this->__on_modify(); }
        void __clear() { size_t cap = __capacity(); __allocator.deallocate(__beg(), cap); __allocate_storage(cap); __on_modify(); }
        __ptr __erase_at_end(size_t how_many) { if(how_many >= __size()) __clear(); else { __backtrack(how_many); __zero(__cur(), how_many); } __on_modify(); return __cur(); }
        __ptr __erase_range(__const_ptr start, __const_ptr end);
        __ptr __erase(__const_ptr pos) { return __erase_range(pos, pos + 1); }
        void __destroy() { if(__beg()) { __allocator.deallocate(__beg(), __capacity()); __my_data.__reset(); } }
        void __swap(__dynamic_buffer& that) { __my_data.__swap_ptrs(that.__my_data); this->__on_modify(); }
        explicit __dynamic_buffer(A const& alloc) : __allocator{ alloc }, __my_data{} {}
        constexpr __dynamic_buffer() noexcept(noexcept(A())) : __allocator{ A() }, __my_data{} {}
        template<std::matching_input_iterator<T> IT> __dynamic_buffer(IT start, IT end, A const& alloc) : __allocator{ alloc }, __my_data{ __allocator.allocate(size_t(std::distance(start, end))), size_t(std::distance(start, end)) } { size_t n = std::distance(start, end); __transfer(__beg(), start, end); __advance(n); }
        __dynamic_buffer(size_t sz) : __allocator{}, __my_data{} { __allocate_storage(sz); __zero(__beg(), sz); }
        __dynamic_buffer(size_t sz, A const& alloc) : __allocator{ alloc }, __my_data{} { __allocate_storage(sz); __zero(__beg(), sz); }
        __dynamic_buffer(size_t sz, T const& val, A const& alloc) : __allocator{ alloc }, __my_data{ __allocator.allocate(sz), sz } { __set(__beg(), val, sz); __advance(sz); }
        __dynamic_buffer(initializer_list<T> const& __ils, A const& alloc) : __dynamic_buffer{ __ils.begin(), __ils.end(), alloc } {}
        __dynamic_buffer(__dynamic_buffer const& that) : __dynamic_buffer{ that.__beg(), that.__cur(), that.__allocator } {}
        __dynamic_buffer(__dynamic_buffer const& that, A const& alloc) : __dynamic_buffer{ that.__beg(), that.__cur(), alloc } {}
        __dynamic_buffer(__dynamic_buffer const& that, size_t start, A const& alloc) : __dynamic_buffer{ that.__getptr(start), that.__cur(), alloc } {}
        __dynamic_buffer(__dynamic_buffer const& that, size_t start, size_t count, A const& alloc) : __dynamic_buffer{ that.__getptr(start), that.__beg() + (count < that.__size() - start ? count : that.__size()), alloc } {}
        __dynamic_buffer(__dynamic_buffer&& that) : __allocator{ move(that.__allocator) }, __my_data{ move(that.__my_data) } {}
        __dynamic_buffer(__dynamic_buffer&& that, A const& alloc) : __allocator{ alloc }, __my_data{ move(that.__my_data) } {}
        ~__dynamic_buffer() { if(__beg()) { __allocator.deallocate(__beg(), __capacity()); } }
        __dynamic_buffer& operator=(__dynamic_buffer const& that) { __destroy(); __allocate_storage(that.__capacity()); __copy(this->__beg(), that.__beg(), that.__capacity()); __advance(that.__size()); __on_modify(); return *this; }
        __dynamic_buffer& operator=(__dynamic_buffer&& that) { __my_data.__move_ptrs(move(that.__my_data)); return *this; }
    };
    template<typename T, allocator_object<T> A>
    void __dynamic_buffer<T, A>::__replace_elements(size_t pos, size_t count, __ptr from, size_t count2)
    {
        if(count2 == count) __copy(__getptr(pos), from, count);
        else 
        {
            long diff = count2 - count;
            size_t target_cap = __capacity() + diff;
            __buf_ptrs nwdat { __allocator.allocate(target_cap), target_cap };
            size_t rem = __size() - (pos + count);
            __copy(nwdat.__begin, __beg(), pos);
            __copy(nwdat.__getptr(pos), from, count2);
            __copy(nwdat.__getptr(pos + count2), __getptr(pos + count), rem);
            nwdat.__setc(__size() + diff);
            __allocator.deallocate(__beg(), __capacity());
            __my_data.__copy_ptrs(nwdat);
            this->__on_modify();
        }
    }
    template<typename T, allocator_object<T> A>
    void __dynamic_buffer<T, A>::__grow_buffer(size_t added)
    {
        if(!added) return;
        size_t num_elements = __size();
        size_t target = __capacity() + added;
        __setn(resize<T>(__beg(), target), num_elements, target);
        __zero(__cur(), __rem());
    }
    template<typename T, allocator_object<T> A>
    template<matching_input_iterator<T> IT> 
    void __dynamic_buffer<T, A>::__append_elements(IT start_it, IT end_it)
    {
        size_t rem = __rem();
        size_t num = std::distance(start_it, end_it);
        if(num > rem) this->__grow_buffer(num - rem);
        for(IT i = start_it; i < end_it; i++, __advance(1)) construct_at(__cur(), *i);
        this->__on_modify();
    }
    template<typename T, allocator_object<T> A>
    void __dynamic_buffer<T, A>::__append_elements(__const_ptr start_ptr, __const_ptr end_ptr)
    {
        size_t rem = __rem();
        size_t num = end_ptr - start_ptr;
        if(num > rem) __grow_buffer(num - rem);
        for(__const_ptr p = start_ptr; p < end_ptr; p++, __advance(1)) construct_at(__cur(), *p);
        this->__on_modify();
    }
    template<typename T, allocator_object<T> A>
    typename __dynamic_buffer<T, A>::__ptr __dynamic_buffer<T, A>::__insert_elements(__const_ptr pos, __const_ptr start_ptr, __const_ptr end_ptr)
    {
        if(__out_of_range(pos)) return nullptr;
        size_t offs = __diff(pos);
        size_t range_size = end_ptr - start_ptr;
        if(pos + range_size < __max())
        {
            if(pos < __cur())
            {
                size_t n = __my_data.__end - pos;
                __ptr temp = __allocator.allocate(n);
                __copy(temp, pos, n);
                __copy(__getptr(offs), start_ptr, range_size);
                __copy(__getptr(offs + range_size), temp, n);
                __allocator.deallocate(temp, n);
                __advance(range_size);
            }
            else 
            {
                __copy(__getptr(offs), start_ptr, range_size);
                __setc(offs + range_size);
            }
        }
        else 
        {
            size_t target_cap = __capacity() + __needed(pos + range_size);
            __buf_ptrs nwdat{ __allocator.allocate(target_cap), target_cap };
            if(pos < __cur())
            {
                size_t rem = __rem();
                __copy(nwdat.__begin, __beg(), offs);
                __copy(nwdat.__getptr(offs + range_size), pos, rem);
                nwdat.__setc(__size() + range_size);
            }
            else
            {
                __copy(nwdat.__begin, __beg(), __size());
                nwdat.__setc(__size() + range_size);
            }
            __allocator.deallocate(__beg(), __size());
            __my_data.__copy_ptrs(nwdat);
            __copy(__getptr(offs), start_ptr, range_size);
        }
        __on_modify();
        return __getptr(offs);
    }
    template<typename T, allocator_object<T> A>
    template<matching_input_iterator<T> IT>
    typename __dynamic_buffer<T, A>::__ptr __dynamic_buffer<T, A>::__insert_elements(__const_ptr pos, IT start_ptr, IT end_ptr)
    {
        if(__out_of_range(pos)) return nullptr;
        size_t range_size = std::distance(start_ptr, end_ptr);
        size_t offs = __diff(pos);
        if(pos + range_size < __max())
        {
            if(pos < __cur())
            {
                size_t n = __rem();
                __ptr temp = __allocator.allocate(n);
                __copy(temp, pos, n);
                __transfer(__getptr(offs), start_ptr, end_ptr);
                __copy(__getptr(offs + range_size), temp, n);
                __allocator.deallocate(temp, n);
                __advance(range_size);
            }
            else 
            {
                __transfer(__getptr(offs), start_ptr, end_ptr);
                __setc(offs + range_size);
            }
        }
        else 
        {
            size_t target_cap = __capacity() + __needed(pos + range_size);
            __buf_ptrs nwdat{ __allocator.allocate(target_cap), target_cap };
            if(pos < __cur())
            {
                size_t rem = __rem();
                __copy(nwdat.__begin, __beg(), offs);
                __copy(nwdat.__getptr(offs + range_size), pos, rem);
                nwdat.__setc(__size() + range_size);
            }
            else
            {
                __copy(nwdat.__begin, __beg(), __size());
                nwdat.__setc(__size() + range_size);
            }
            __allocator.deallocate(__beg(), __size());
            __my_data.__copy_ptrs(nwdat);
            __transfer(__getptr(offs), start_ptr, end_ptr);
        }
        this->__on_modify();
        return __getptr(offs);
    }
    template<typename T, allocator_object<T> A>
    template<typename ... Args>
    requires constructible_from<T, Args...>
    typename __dynamic_buffer<T, A>::__ptr __dynamic_buffer<T, A>::__emplace_element(__const_ptr pos, Args&& ... args)
    {
        if(pos < __beg()) return nullptr;
        if(pos >= __max()) 
        { 
            __grow_buffer(1);
            pos = __max() - 1;
        }
        return construct_at(pos, forward<Args>(args)...);
    }
    template<typename T, allocator_object<T> A>
    typename __dynamic_buffer<T, A>::__ptr __dynamic_buffer<T, A>::__erase_range(__const_ptr start, __const_ptr end)
    {
        if(__out_of_range(start, end)) return nullptr;
        size_t how_many = end - start;
        size_t rem = __ediff(end);
        size_t start_pos = __diff(start);
        if(rem == 0) return __erase_at_end(how_many);
        else 
        {
            __ptr temp = __allocator.allocate(rem);
            __copy(temp, end, rem);
            __zero(__getptr(start_pos), __ediff(start));
            __copy(__getptr(start_pos), temp, rem);
            __allocator.deallocate(temp, rem);
            __setc(start_pos + rem);
        }
        __on_modify();
        return __getptr(start_pos);
    }
}
#endif