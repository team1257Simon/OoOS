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
        typedef decltype(sizeof(T)) __size_type;
        typedef decltype(declval<T*>() - declval<T*>()) __diff_type;
        __ptr_type __begin{};
        __ptr_type __end{};
        __ptr_type __max{};
        constexpr __buf_ptrs() = default;
        constexpr __buf_ptrs(__ptr_type start, __ptr_type end, __ptr_type max) : __begin{ start }, __end{ end }, __max{ max } {}
        void __reset() { __begin = __end = __max = __ptr_type{}; }
        __buf_ptrs(__buf_ptrs const& that) : __begin{ that.__begin }, __end{ that.__end }, __max{ that.__max } {}
        __buf_ptrs(__buf_ptrs&& that) : __begin{ that.__begin }, __end{ that.__end }, __max{ that.__max } { that.__reset(); }
        __buf_ptrs(__ptr_type start, __ptr_type end) : __begin{ start }, __end{ end }, __max { end } {}
        __buf_ptrs(__ptr_type start, __size_type cap) : __begin{ start }, __end{ start }, __max{ start + cap } {}
        void __copy_ptrs(__buf_ptrs const& that) { this->__begin = that.__begin; this->__end = that.__end; this->__max = that.__max; }
        void __move_ptrs(__buf_ptrs&& that) { this->__copy_ptrs(that); that.__reset(); }
        void __swap_ptrs(__buf_ptrs& that) { __buf_ptrs tmp; tmp.__copy_ptrs(*this); this->__copy_ptrs(that); that.__copy_ptrs(tmp); }
        void __set_ptrs(__ptr_type begin, __ptr_type end, __ptr_type max) { this->__begin = begin; this->__end = end; this->__max = max; }
        __ptr_type __get_ptr(__size_type offs) { return __begin + offs; }
        void __setc(__ptr_type where) { __end = where; }
        void __setc(__size_type offs) { __end = __begin + offs; }
        void __adv(__size_type n) { __end += n; }
        void __bck(__size_type n) { __end -= n; }
    };
    /**
     * This base-type implements the functionality shared by the dynamic-container types (mainly string and vector). 
     * Places where a string differs from a vector of chars are handled using virtual functions that can be overridden for the finer details.
     */
    template<typename T, allocator_object<T> A>
    struct __dynamic_buffer
    {
        typedef __buf_ptrs<T> __container;
        typedef typename __container::__ptr_type __ptr;
        typedef typename __container::__const_ptr_type __const_ptr;
        typedef typename __container::__size_type __size_type;
        typedef typename __container::__diff_type __diff_type;
        typedef decltype(*(declval<__ptr>())) __ref;
        typedef decltype(*(declval<__const_ptr>())) __const_ref;
        typedef A __alloc_type;
        __alloc_type __allocator;
        __container __my_data;
        /**
         * Copies data using iterators that might not be contiguous (e.g. tree iterators) or forward-facing (e.g. reverse iterators).
         */
        template<std::matching_input_iterator<T> IT> constexpr void __transfer(T* where, IT start, IT end) { for(IT i = start; i != end; i++, where++) { *where = *i; } }
        
        constexpr void __set(__ptr where, T const& val, __size_type n) { arrayset<T>(where, val, n); }
        /**
         * Selects the proper function to clear allocated storage of garbage data.
         * If the data consists of nontrivial objects (classes with virtual members, etc) this function does nothing.
         * Otherwise, it will behave similarly to using memset to zero the memory.
         */
        constexpr void __zero(__ptr where, __size_type n)  { array_zero<T>(where, n); }
        constexpr void __copy(__ptr where, __const_ptr src, __size_type n) { arraycopy<T>(where, src, n); }
        /**
         * Called whenever the end and/or max pointers are changed after initial construction, other than through the advance and backtrack hooks.
         * Inheritors can override to add functionality that needs to be invoked whenever these pointers move. The default implementation does nothing.
         */
        virtual void __on_modify() {}
        constexpr bool __grow_buffer(__size_type added);
        template<std::matching_input_iterator<T> IT> constexpr __ptr __append_elements(IT start_it, IT end_it);
        constexpr __ptr __insert_elements(__const_ptr pos, __const_ptr start_ptr, __const_ptr end_ptr);
        template<matching_input_iterator<T> IT> constexpr __ptr __insert_elements(__const_ptr pos, IT start_ptr, IT end_ptr);
        template<typename ... Args> requires constructible_from<T, Args...> constexpr __ptr __emplace_element(__const_ptr pos, Args&& ... args);
        template<typename ... Args> requires constructible_from<T, Args...> constexpr __ptr __emplace_at_end(Args&& ... args);
        constexpr __ptr __replace_elements(__size_type pos, __size_type count, __ptr from, __size_type count2);
        constexpr __ptr __erase_range(__const_ptr start, __const_ptr end);
        constexpr __ptr __insert_element(__const_ptr pos, T const& t) { return __insert_elements(pos, __builtin_addressof(t), __builtin_addressof(t) + 1); }
        constexpr __ptr __beg() noexcept { return __my_data.__begin; }
        constexpr __const_ptr __beg() const noexcept { return __my_data.__begin; }
        constexpr __ptr __cur() noexcept { return __my_data.__end; }
        constexpr __const_ptr __cur() const noexcept { return __my_data.__end; }
        constexpr __ptr __max() noexcept { return __my_data.__max; }
        constexpr __const_ptr __max() const noexcept { return __my_data.__max; }
        constexpr __size_type __diff(__const_ptr pos) const noexcept { return __size_type(pos - __beg()); }
        constexpr __diff_type __needed(__const_ptr pos) const noexcept { return pos - __max(); }
        constexpr bool __valid_end_pos(__const_ptr pos) const noexcept { return pos >= __beg() && pos <= __max(); }
        constexpr bool __out_of_range(__const_ptr pos) const noexcept { return pos < __beg() || pos >= __max(); }
        constexpr bool __out_of_range(__const_ptr start, __const_ptr end) const noexcept { return __out_of_range(start) || __out_of_range(end) || end < start; }
        constexpr __ptr __get_ptr(__size_type __i) noexcept { return __beg() + __i; }
        constexpr __const_ptr __get_ptr(__size_type __i) const noexcept { return __beg() + __i; }
        constexpr __ref __get(__size_type __i) { return *__get_ptr(__i); }
        constexpr __const_ref __get(__size_type __i) const { return *__get_ptr(__i); }
        constexpr __ref __get_last() noexcept { return *(__cur() - 1); }
        constexpr __const_ref __get_last() const noexcept { return *(__cur() - 1); }
        constexpr void __setp(__buf_ptrs<T> const& ptrs) noexcept { __my_data.__copy_ptrs(ptrs); }
        constexpr void __setp(__ptr beg, __ptr cur, __ptr end) noexcept { __my_data.__set_ptrs(beg, cur, end); }
        constexpr void __setp(__ptr beg, __ptr end) noexcept { __setp(beg, beg, end); }
        constexpr void __setn(__ptr beg, __size_type c, __size_type n) noexcept { __setp(beg, beg + c, beg + n); }
        constexpr void __setn(__ptr beg, __size_type n) noexcept { __setp(beg, beg + n); }
        constexpr void __setc(__ptr pos) noexcept { if(__valid_end_pos(pos)) __my_data.__setc(pos); } 
        constexpr void __setc(__size_type pos) noexcept { __setc(__get_ptr(pos)); }
        constexpr void __bumpc(int64_t off) noexcept { __setc(__cur() + off); }
        constexpr __size_type __max_capacity() const noexcept { return std::numeric_limits<__size_type>::max(); }
        constexpr void __advance(__size_type n) { if(__valid_end_pos(__cur() + n)) { __my_data.__adv(n); } else { __setc(__max()); } }
        constexpr void __backtrack(__size_type n) { if(__valid_end_pos(__cur() - n)) { __my_data.__bck(n); } else { __setc(__beg()); } }
        constexpr __size_type __size() const noexcept { return __size_type(__cur() - __beg()); }
        constexpr __size_type __capacity() const noexcept { return __size_type(__max() - __beg()); }
        constexpr __size_type __rem() const noexcept { return __size_type(__max() - __cur()); }
        constexpr __size_type __ediff(__const_ptr pos) const noexcept { return __size_type(__cur() - pos); }
        constexpr void __trim_buffer() throw() { __size_type num_elements = __size(); __setn(resize<T>(__beg(), num_elements), num_elements, num_elements); this->__on_modify(); }
        constexpr void __allocate_storage(__size_type n) throw() { __setn(__allocator.allocate(n), n); }
        constexpr void __construct_element(__ptr pos, T const& t) { if(!__out_of_range(pos)) { construct_at(pos, t); if(pos > __cur()) __setc(pos); }  }
        constexpr __ptr __assign_elements(__size_type count, T const& t) { if(count > __capacity()) { if(!__grow_buffer(count - __capacity())) return nullptr; } __set(__beg(), t, count); if (count < __size()) { __zero(__get_ptr(count), __size() - count); } __setc(count); this->__on_modify(); return __cur(); }
        constexpr __ptr __assign_elements(__const_ptr start, __const_ptr end) { __size_type count = end - start; if(count > __capacity()) { if(!__grow_buffer(count - __capacity())) return nullptr; } __copy(__beg(), start, count); if (count < __size()) { __zero(__get_ptr(count), __size() - count); } __setc(count); this->__on_modify(); return __cur(); }
        constexpr __ptr __replace_elements(__const_ptr start, __const_ptr end, __ptr from, __size_type count) { if(!__out_of_range(start, end)) return __replace_elements(start - __beg(), end - start, from, count); else return nullptr; }
        constexpr __ptr __assign_elements(std::initializer_list<T> ini) { return __assign_elements(ini.begin(), ini.end()); }
        constexpr __ptr __append_elements(__size_type count, T const& t) { if(__max() <= __cur() + count) { if(!this->__grow_buffer(__size_type(count - __rem()))) return nullptr; } for(__size_type i = 0; i < count; i++, __advance(1)) construct_at(__cur(), t); this->__on_modify(); return __cur(); }
        constexpr __ptr __append_element(T const& t) { if(!(__max() > __cur())) { if(!this->__grow_buffer(1)) return nullptr; } construct_at(__cur(), t); __advance(1); this->__on_modify(); return __cur(); }
        constexpr void __clear() { __size_type cap = __capacity(); __destroy(); __allocate_storage(cap); this->__on_modify(); }
        constexpr __ptr __erase_at_end(__size_type how_many) { if(how_many >= __size()) __clear(); else { __backtrack(how_many); this->__zero(__cur(), how_many); } __on_modify(); return __cur(); }
        constexpr __ptr __erase(__const_ptr pos) { return __erase_range(pos, pos + 1); }
        constexpr void __destroy() { if(__beg()) { __allocator.deallocate(__beg(), __capacity()); __my_data.__reset(); } }
        constexpr void __swap(__dynamic_buffer& that) { __my_data.__swap_ptrs(that.__my_data); this->__on_modify(); that.__on_modify(); }
        constexpr void __move(__dynamic_buffer&& that) { __my_data.__move_ptrs(move(that.__my_data)); }
        constexpr void __realloc_move(__dynamic_buffer&& that) { __destroy(); if(!that.__beg()) return; __allocate_storage(that.__capacity()); arraymove<T>(this->__beg(), that.__beg(), that.__size()); __advance(that.__size()); that.__destroy(); this->__on_modify(); }
        constexpr explicit __dynamic_buffer(A const& alloc) : __allocator{ alloc }, __my_data{} {}
        constexpr __dynamic_buffer() noexcept(noexcept(A())) : __allocator{ A() }, __my_data{} {}
        template<std::matching_input_iterator<T> IT> constexpr __dynamic_buffer(IT start, IT end, A const& alloc) : __allocator{ alloc }, __my_data{ __allocator.allocate(__size_type(std::distance(start, end))), __size_type(std::distance(start, end)) } { __size_type n = std::distance(start, end); __transfer(__beg(), start, end); __advance(n); }
        constexpr __dynamic_buffer(__size_type sz) : __allocator{}, __my_data{} { __allocate_storage(sz); this->__zero(__beg(), sz); }
        constexpr __dynamic_buffer(__size_type sz, A const& alloc) : __allocator{ alloc }, __my_data{} { __allocate_storage(sz); __zero(__beg(), sz); }
        constexpr __dynamic_buffer(__size_type sz, T const& val, A const& alloc) : __allocator{ alloc }, __my_data{ __allocator.allocate(sz), sz } { __set(__beg(), val, sz); __advance(sz); }
        constexpr __dynamic_buffer(initializer_list<T> const& __ils, A const& alloc) : __dynamic_buffer{ __ils.begin(), __ils.end(), alloc } {}
        constexpr __dynamic_buffer(__dynamic_buffer const& that) : __dynamic_buffer{ that.__beg(), that.__cur(), that.__allocator } {}
        constexpr __dynamic_buffer(__dynamic_buffer const& that, A const& alloc) : __dynamic_buffer{ that.__beg(), that.__cur(), alloc } {}
        constexpr __dynamic_buffer(__dynamic_buffer const& that, __size_type start, A const& alloc) : __dynamic_buffer{ that.__get_ptr(start), that.__cur(), alloc } {}
        constexpr __dynamic_buffer(__dynamic_buffer const& that, __size_type start, __size_type count, A const& alloc) : __dynamic_buffer{ that.__get_ptr(start), that.__beg() + (count < that.__size() - start ? count : that.__size()), alloc } {}
        constexpr __dynamic_buffer(__dynamic_buffer&& that) : __allocator{ move(that.__allocator) }, __my_data{ move(that.__my_data) } {}
        constexpr  __dynamic_buffer(__dynamic_buffer&& that, A const& alloc) : __allocator{ alloc }, __my_data{ move(that.__my_data) } {}
        constexpr ~__dynamic_buffer() { if(__beg()) { __allocator.deallocate(__beg(), __capacity()); } }
        constexpr __dynamic_buffer& operator=(__dynamic_buffer const& that) { __destroy(); __allocate_storage(that.__capacity()); __copy(this->__beg(), that.__beg(), that.__capacity()); __advance(that.__size()); __on_modify(); return *this; }
        constexpr __dynamic_buffer& operator=(__dynamic_buffer&& that) { __destroy(); this->__move(move(that)); return *this; }
    };
    template<typename T, allocator_object<T> A>
    constexpr typename __dynamic_buffer<T, A>::__ptr __dynamic_buffer<T, A>::__replace_elements(__size_type pos, __size_type count, __ptr from, __size_type count2)
    {
        if(count2 == count){ __copy(__get_ptr(pos), from, count); return __get_ptr(pos + count); }
        else try
        {
            long diff = count2 - count;
            __size_type target_cap = __capacity() + diff;
            __buf_ptrs nwdat { __allocator.allocate(target_cap), target_cap };
            __size_type rem = __size() - (pos + count);
            __copy(nwdat.__begin, __beg(), pos);
            __copy(nwdat.__get_ptr(pos), from, count2);
            __copy(nwdat.__get_ptr(pos + count2), __get_ptr(pos + count), rem);
            nwdat.__setc(__size() + diff);
            __allocator.deallocate(__beg(), __capacity());
            __my_data.__copy_ptrs(nwdat);
            this->__on_modify();
            return __get_ptr(pos + count);
        }
        catch(...) { return nullptr; }
    }
    template<typename T, allocator_object<T> A>
    constexpr bool __dynamic_buffer<T, A>::__grow_buffer(__size_type added)
    {
        if(!added) return true; // Zero elements -> vacuously true completion
        __size_type num_elements = __size();
        __size_type target = __capacity() + added;
        try { __setn(resize<T>(__beg(), target), num_elements, target); this->__zero(__cur(), added); } 
        catch(...) { return false; }
        return true;
    }
    template<typename T, allocator_object<T> A>
    template<matching_input_iterator<T> IT> 
    constexpr typename __dynamic_buffer<T, A>::__ptr __dynamic_buffer<T, A>::__append_elements(IT start_it, IT end_it)
    {
        __size_type rem = __rem();
        __size_type num = std::distance(start_it, end_it);
        if(!__beg() || num > rem){ if(!this->__grow_buffer(num - rem)) return nullptr; }
        for(IT i = start_it; i < end_it; i++, __advance(1)) construct_at(__cur(), *i);
        this->__on_modify();
        return __cur();
    }
    template<typename T, allocator_object<T> A>
    constexpr typename __dynamic_buffer<T, A>::__ptr __dynamic_buffer<T, A>::__insert_elements(__const_ptr pos, __const_ptr start_ptr, __const_ptr end_ptr)
    {
        if(__out_of_range(pos)) return nullptr;
        try
        {
            __size_type offs = __diff(pos);
            __size_type range_size = end_ptr - start_ptr;
            if(pos + range_size < __max())
            {
                if(pos < __cur())
                {
                    __size_type n = __my_data.__end - pos;
                    __ptr temp = __allocator.allocate(n);
                    __copy(temp, pos, n);
                    __copy(__get_ptr(offs), start_ptr, range_size);
                    __copy(__get_ptr(offs + range_size), temp, n);
                    __allocator.deallocate(temp, n);
                    __advance(range_size);
                }
                else 
                {
                    __copy(__get_ptr(offs), start_ptr, range_size);
                    __setc(offs + range_size);
                }
            }
            else 
            {
                __size_type target_cap = __capacity() + __needed(pos + range_size);
                __buf_ptrs nwdat{ __allocator.allocate(target_cap), target_cap };
                if(pos < __cur())
                {
                    __size_type rem = __rem();
                    __copy(nwdat.__begin, __beg(), offs);
                    __copy(nwdat.__get_ptr(offs + range_size), pos, rem);
                    nwdat.__setc(__size() + range_size);
                }
                else
                {
                    __copy(nwdat.__begin, __beg(), __size());
                    nwdat.__setc(__size() + range_size);
                }
                __allocator.deallocate(__beg(), __size());
                __my_data.__copy_ptrs(nwdat);
                __copy(__get_ptr(offs), start_ptr, range_size);
            }
            __on_modify();
            return __get_ptr(offs);
        }
        catch(...) { return nullptr; }
    }
    template<typename T, allocator_object<T> A>
    template<matching_input_iterator<T> IT>
    constexpr typename __dynamic_buffer<T, A>::__ptr __dynamic_buffer<T, A>::__insert_elements(__const_ptr pos, IT start_ptr, IT end_ptr)
    {
        if(__out_of_range(pos)) return nullptr;
        try
        {
            __size_type range_size = std::distance(start_ptr, end_ptr);
            __size_type offs = __diff(pos);
            if(pos + range_size < __max())
            {
                if(pos < __cur())
                {
                    __size_type n = __rem();
                    __ptr temp = __allocator.allocate(n);
                    __copy(temp, pos, n);
                    __transfer(__get_ptr(offs), start_ptr, end_ptr);
                    __copy(__get_ptr(offs + range_size), temp, n);
                    __allocator.deallocate(temp, n);
                    __advance(range_size);
                }
                else 
                {
                    __transfer(__get_ptr(offs), start_ptr, end_ptr);
                    __setc(offs + range_size);
                }
            }
            else 
            {
                __size_type target_cap = __capacity() + __needed(pos + range_size);
                __buf_ptrs nwdat{ __allocator.allocate(target_cap), target_cap };
                if(pos < __cur())
                {
                    __size_type rem = __rem();
                    __copy(nwdat.__begin, __beg(), offs);
                    __copy(nwdat.__get_ptr(offs + range_size), pos, rem);
                    nwdat.__setc(__size() + range_size);
                }
                else
                {
                    __copy(nwdat.__begin, __beg(), __size());
                    nwdat.__setc(__size() + range_size);
                }
                __allocator.deallocate(__beg(), __size());
                __my_data.__copy_ptrs(nwdat);
                __transfer(__get_ptr(offs), start_ptr, end_ptr);
            }
            this->__on_modify();
            return __get_ptr(offs);
        }
        catch(...) { return nullptr; }
    }
    template<typename T, allocator_object<T> A>
    template<typename ... Args>
    requires constructible_from<T, Args...>
    constexpr typename __dynamic_buffer<T, A>::__ptr __dynamic_buffer<T, A>::__emplace_element(__const_ptr pos, Args&& ... args)
    {
        if(pos < __beg()) return nullptr;
        if(pos >= __max()) { if(!__grow_buffer(1)) return nullptr; pos = __max() - 1; }
        return construct_at(const_cast<__ptr>(pos), forward<Args>(args)...);
    }
    template <typename T, allocator_object<T> A>
    template <typename ... Args>
    requires constructible_from<T, Args...>
    constexpr typename __dynamic_buffer<T, A>::__ptr std::__impl::__dynamic_buffer<T, A>::__emplace_at_end(Args && ...args)
    {
        if(__size() == __capacity() && !__grow_buffer(1)) return nullptr;
        __ptr p = construct_at(__cur(), forward<Args>(args)...);
        __bumpc(1L);
        this->__on_modify();
        return p;
    }
    template<typename T, allocator_object<T> A>
    constexpr typename __dynamic_buffer<T, A>::__ptr __dynamic_buffer<T, A>::__erase_range(__const_ptr start, __const_ptr end)
    {
        if(__out_of_range(start, end)) return nullptr;
        __size_type how_many = end - start;
        __size_type rem = __ediff(end);
        __size_type start_pos = __diff(start);
        if(rem == 0) return __erase_at_end(how_many);
        else try
        {
            __ptr temp = __allocator.allocate(rem);
            __copy(temp, end, rem);
            __zero(__get_ptr(start_pos), __ediff(start));
            __copy(__get_ptr(start_pos), temp, rem);
            __allocator.deallocate(temp, rem);
            __setc(start_pos + rem);
        }
        catch (...) { return nullptr; }
        __on_modify();
        return __get_ptr(start_pos);
    }
}
#endif