#ifndef __STL_VECTOR
#define __STL_VECTOR
#include "memory"
#include "bits/stl_iterator.hpp"
#include "kernel/libk_decls.h"
#include "initializer_list"
namespace std::__impl
{
    /**
     * Base for the contiguous STL containers such as vector, streambuf, and string, implementing a lot of the shared functionality of those types.
     * Like much of the namespace std the libk will see, there are things here that are not constexpr that normally would be. 
     * Calls to the heap allocator, for instance, rely on the kernel frame pointer which resides somewhere that can't be constexpr.
     */
    template<typename T, allocator_object<T> A>
    struct __dynamic_buffer
    {    
    protected:
        
        A __allocator;
        typedef T* __ptr;
        typedef T const* __const_ptr;
        typedef T& __ref;
        typedef T const& __const_ref;
        struct __data_impl
        {
            __ptr __begin{};
            __ptr __end{};
            __ptr __max{};
            constexpr __data_impl() = default;
            constexpr __data_impl(__ptr start, __ptr end, __ptr max) : __begin{ start }, __end{ end }, __max{ max } {}
            void __reset() { __begin = __end = __max = __ptr{}; }
            __data_impl(__data_impl const& that) : __begin{ that.__begin }, __end{ that.__end }, __max{ that.__max } {}
            __data_impl(__data_impl&& that) : __begin{ that.__begin }, __end{ that.__end }, __max{ that.__max } { that.__reset(); }
            __data_impl(__ptr start, __ptr end) : __begin{ start }, __end{ end }, __max { end } {}
            __data_impl(__ptr start, size_t cap) : __begin{ start }, __end{ start }, __max{ start + cap } {}
            void __copy_ptrs(__data_impl const& that) { this->__begin = that.__begin; this->__end = that.__end; this->__max = that.__max; }
            void __move(__data_impl&& that) { this->__copy_ptrs(that); that.__reset(); }
            void __swap(__data_impl& that) { __data_impl tmp; tmp.__copy_ptrs(*this); this->__copy_ptrs(that); that.__copy_ptrs(tmp); }
        } __my_data;
        template<std::matching_input_iterator<T> IT> 
        void __transfer(T* where, IT start, IT end) { for(IT i = start; i < end; i++, where++) { construct_at(where, *i); } }
        virtual void __set(__ptr where, T const& val, size_t n) { arrayset<T>(where, val, n); }
        virtual void __zero(__ptr where, size_t n) { arrayset<T>(where, 0, n); }
        virtual void __copy(__ptr where, __const_ptr src, size_t n) { arraycopy<T>(where, src, n); }
        /**
         * Called whenever the end and/or max pointers are changed after initial construction.
         * Inheritors can override to add functionality that needs to be invoked whenever these pointers move.
         * The default implementation does nothing.
         */
        virtual void __on_modify() {}
        // Shortcut to move the end pointer. Note that this does NOT call the on-modify callback; it's just a space saver.
        inline void __advance(size_t n) { __my_data.__end += n; }
        void __allocate_storage(size_t n)
        {
            __my_data.__begin = __allocator.allocate(n);
            __my_data.__end  = __my_data.__begin;
            __my_data.__max  = __my_data.__begin + n;
        }
        inline size_t __size() const noexcept { return static_cast<size_t>(__my_data.__end - __my_data.__begin); }
        inline size_t __capacity() const noexcept { return static_cast<size_t>(__my_data.__max - __my_data.__begin); }
        inline void __construct_element(__ptr pos, T const& t) { if(pos < __my_data.__max && pos >= __my_data.__begin) { construct_at(pos, t); if(pos > __my_data.__end) __my_data.__end = pos; }  }
        void __assign_elements(size_t count, T const& t) 
        { 
            if(count > __capacity()) __grow_buffer(count - __capacity());
            __set(__my_data.__begin, t, count);
            if (count < __size()) { __zero(__my_data.__begin + count, __size() - count); }
            __my_data.__end = __my_data.__begin + count;
        }
        void __assign_elements(__const_ptr start, __const_ptr end)
        {
            size_t count = end - start;
            if(count > __capacity()) __grow_buffer(count - __capacity());
            __copy(__my_data.__begin, start, count);
            if (count < __size()) { __zero(__my_data.__begin + count, __size() - count); }
            __my_data.__end = __my_data.__begin + count;
        }
        void __replace_elements(size_t pos, size_t count, __ptr from, size_t count2) 
        {
            if(count2 == count) __copy(__my_data.__begin + pos, from, count);
            else 
            {
                long diff = count2 - count;
                size_t target_cap = __capacity() + diff;
                __data_impl nwdat { __allocator.allocate(target_cap), target_cap };
                size_t rem = __size() - (pos + count);
                __copy(nwdat.__begin, __my_data.__begin, pos);
                __copy(nwdat.__begin + pos, from, count2);
                __copy(nwdat.__begin + pos + count2, __my_data.__begin + pos + count, rem);
                nwdat.__end = nwdat.__begin + __size() + diff;
                __allocator.deallocate(__my_data.__begin, __capacity());
                __my_data.__copy_ptrs(nwdat);
                this->__on_modify();
            }
        }
        void __replace_elements(__const_ptr start, __const_ptr end, __ptr from, size_t count)
        { 
            if(start < __my_data.__begin || start >= __my_data.__max || end < __my_data.__begin || end >= __my_data.__max || end <= start) return;
            __replace_elements(start - __my_data.__begin, end - start, from, count);
        }
        void __assign_elements(std::initializer_list<T> ini) { __assign_elements(ini.begin(), ini.end()); }
        void __grow_buffer(size_t added)
        {
            if(!added) return;
            size_t num_elements = __size();
            size_t target = __capacity() + added;
            __my_data.__begin = resize<T>(__my_data.__begin, target);
            __my_data.__end = __my_data.__begin + num_elements;
            __my_data.__max = __my_data.__begin + target;
            __zero(__my_data.__end, __my_data.__max - __my_data.__end);
        }
        inline size_t __rem() const { return __my_data.__max - __my_data.__end; }
        void __trim_buffer() 
        {
            size_t num_elements = __size();
            __my_data.__begin = resize<T>(__my_data.__begin, num_elements, alignof(T));
            __my_data.__end = __my_data.__begin + num_elements;
            __my_data.__max = __my_data.__begin + num_elements;
            this->__on_modify();
        }
        inline void __append_elements(size_t count, T const& t)
        {
            if(__my_data.__max < __my_data.__end + count) this->__grow_buffer(size_t(count - __rem()));
            for(size_t i = 0; i < count; i++, __my_data.__end++) construct_at(__my_data.__end, t);
            this->__on_modify();
        }
        template<std::matching_input_iterator<T> IT> 
        void __append_elements(IT start_it, IT end_it)
        {
            size_t rem = __my_data.__max - __my_data.__end;
            size_t num = end_it - start_it;
            if(num > rem) this->__grow_buffer(num - rem);
            for(IT i = start_it; i < end_it; i++)
            {
                construct_at(__my_data.__end, *i);
                __my_data.__end++;
            }
            this->__on_modify();
        }
        void __append_elements(__const_ptr start_ptr, __const_ptr end_ptr)
        {
            size_t rem = __my_data.__max - __my_data.__end;
            size_t num = end_ptr - start_ptr;
            if(num > rem) __grow_buffer(num - rem);
            for(__const_ptr p = start_ptr; p < end_ptr; p++)
            {
                construct_at(__my_data.__end, *p);
                __my_data.__end++;
            }
            this->__on_modify();
        }
        void __append_element(T const& t) 
        {
            if(!(__my_data.__max > __my_data.__end)) __grow_buffer(1);
            construct_at(__my_data.__end, t);
            __my_data.__end++;
            this->__on_modify();
        }
        __ptr __insert_element(__const_ptr pos, T const& t)
        {
            if(__my_data.__begin > pos || __my_data.__max <= pos) return nullptr;
            size_t offs = pos - __my_data.__begin;
            if(__my_data.__max > __my_data.__end)
            {
                if(pos < __my_data.__end)
                {
                    size_t n = __my_data.__end - pos;
                    __ptr temp = __allocator.allocate(n);
                    __copy(temp, pos, n);
                    __construct_element(__my_data.__begin + offs, t);
                    __copy(__my_data.__begin + offs + 1, temp, n);
                    __allocator.deallocate(temp, n);
                }
                else __construct_element(__my_data.__begin + offs, t);
                __my_data.__end++;
            }
            else
            {
                size_t target_cap = __capacity() + 1;
                __data_impl nwdat{ __allocator.allocate(target_cap), target_cap };
                if(pos < __my_data.__end)
                {
                    size_t rem = __my_data.__end - pos;
                    __copy(nwdat.__begin, __my_data.__begin, offs);
                    __copy(nwdat.__begin + offs + 1, pos, rem);
                    nwdat.__end = nwdat.__begin + __size() + 1;
                }
                else
                {
                    __copy(nwdat.__begin, __my_data.__begin, __size());
                    nwdat.__end = nwdat.__begin + offs + 1;
                }
                __allocator.deallocate(__my_data.__begin, __size());
                __my_data.__copy_ptrs(nwdat);
                __construct_element(__my_data.__begin + offs, t);
            }
            this->__on_modify();
            return __my_data.__begin + offs;
        }
        __ptr __insert_elements(__const_ptr pos, __const_ptr start_ptr, __const_ptr end_ptr)
        {
            if(__my_data.__begin > pos || __my_data.__max <= pos) return nullptr;
            size_t range_size = end_ptr - start_ptr;
            size_t offs = pos - __my_data.__begin;
            if(pos + range_size < __my_data.__max)
            {
                if(pos < __my_data.__end)
                {
                    size_t n = __my_data.__end - pos;
                    __ptr temp = __allocator.allocate(n);
                    __copy(temp, pos, n);
                    __copy(__my_data.__begin + offs, start_ptr, range_size);
                    __copy(__my_data.__begin + offs + range_size, temp, n);
                    __allocator.deallocate(temp, n);
                    __my_data.__end += range_size;
                }
                else 
                {
                    __copy(__my_data.__begin + offs, start_ptr, range_size);
                    __my_data.__end = __my_data.__begin + offs + range_size;
                }
            }
            else 
            {
                size_t target_cap = __capacity() + size_t((pos + range_size) - __my_data.__max);
                __data_impl nwdat{ __allocator.allocate(target_cap), target_cap };
                if(pos < __my_data.__end)
                {
                    size_t rem = __my_data.__end - pos;
                    __copy(nwdat.__begin, __my_data.__begin, offs);
                    __copy(nwdat.__begin + offs + range_size, pos, rem);
                    nwdat.__end = nwdat.__begin + __size() + range_size;
                }
                else
                {
                    __copy(nwdat.__begin, __my_data.__begin, __size());
                    nwdat.__end = nwdat.__begin + offs + range_size;
                }
                __allocator.deallocate(__my_data.__begin, __size());
                __my_data.__copy_ptrs(nwdat);
                __copy(__my_data.__begin + offs, start_ptr, range_size);
            }
            __on_modify();
            return __my_data.__begin + offs;
        }
        template<std::matching_input_iterator<T> IT> 
        __ptr __insert_elements(__const_ptr pos, IT start_ptr, IT end_ptr)
        {
            if(__my_data.__begin > pos || __my_data.__max <= pos) return nullptr;
            size_t range_size = end_ptr - start_ptr;
            size_t offs = pos - __my_data.__begin;
            if(pos + range_size < __my_data.__max)
            {
                if(pos < __my_data.__end)
                {
                    size_t n = __my_data.__end - pos;
                    __ptr temp = __allocator.allocate(n);
                    __copy(temp, pos, n);
                    __transfer(__my_data.__begin + offs, start_ptr, end_ptr);
                    __copy(__my_data.__begin + offs + range_size, temp, n);
                    __allocator.deallocate(temp, n);
                    __my_data.__end += range_size;
                }
                else 
                {
                    __transfer(__my_data.__begin + offs, start_ptr, end_ptr);
                    __my_data.__end = __my_data.__begin + offs + range_size;
                }
            }
            else 
            {
                size_t target_cap = __capacity() + size_t((pos + range_size) - __my_data.__max);
                __data_impl nwdat{ __allocator.allocate(target_cap), target_cap };
                if(pos < __my_data.__end)
                {
                    size_t rem = __my_data.__end - pos;
                    __copy(nwdat.__begin, __my_data.__begin, offs);
                    __copy(nwdat.__begin + offs + range_size, pos, rem);
                    nwdat.__end = nwdat.__begin + __size() + range_size;
                }
                else
                {
                    __copy(nwdat.__begin, __my_data.__begin, __size());
                    nwdat.__end = nwdat.__begin + offs + range_size;
                }
                __allocator.deallocate(__my_data.__begin, __size());
                __my_data.__copy_ptrs(nwdat);
                __transfer(__my_data.__begin + offs, start_ptr, end_ptr);
            }
            this->__on_modify();
            return __my_data.__begin + offs;
        }
        template<typename ... Args> 
        requires constructible_from<T, Args...>
        __ptr __emplace_element(__const_ptr pos, Args&& ... args)
        {
            if(pos < __my_data.__begin) return nullptr;
            if(pos >= __my_data.__max) 
            { 
                __grow_buffer(1);
                pos = __my_data.__max - 1;
            }
            size_t offs = pos - __my_data.__begin;
            return construct_at(pos, forward<Args>(args)...);
        }
        void __clear()
        {
            size_t cap = __capacity();
            __allocator.deallocate(__my_data.__begin, cap);
            __allocate_storage(cap);
            __on_modify();
        }
        __ptr __erase_at_end(size_t how_many)
        {
            if(how_many >= __size()) __clear();
            else
            {
                __my_data.__end -= how_many;
                __zero(__my_data.__end, how_many);
            }
            __on_modify();
            return __my_data.__end;
        }
        __ptr __erase_range(__const_ptr start, __const_ptr end)
        {
            if(__my_data.__begin > start || __my_data.__begin > end || __my_data.__max <= start || __my_data.__max < end || end < start) return nullptr;
            size_t how_many = end - start;
            size_t rem = __my_data.__end - end;
            size_t start_pos = start - __my_data.__begin;
            if(rem == 0) return __erase_at_end(how_many);
            else 
            {
                __ptr temp = __allocator.allocate(rem);
                __copy(temp, end, rem);
                __zero(__my_data.__begin + start_pos, __my_data.__end - start);
                __copy(__my_data.__begin + start_pos, temp, rem);
                __allocator.deallocate(temp, rem);
                __my_data.__end = __my_data.__begin + start_pos + rem;
            }
            __on_modify();
            return __my_data.__begin + start_pos;
        }
        inline __ptr __erase(__const_ptr pos) { return __erase_range(pos, pos + 1); }
        void __swap(__dynamic_buffer& that) { __my_data.__swap(that.__my_data); this->__on_modify(); }
        explicit __dynamic_buffer(A const& alloc) : __allocator{ alloc }, __my_data{} {}
        constexpr __dynamic_buffer() noexcept(noexcept(A())) : __allocator{ A() }, __my_data{} {}
        template<std::matching_input_iterator<T> IT> 
        __dynamic_buffer(IT start, IT end, A const& alloc) : __allocator{ alloc }, __my_data{ __allocator.allocate(size_t(end - start)), size_t(end - start) } { size_t n = end - start; __transfer(__my_data.__begin, start, end); __advance(n); }
        __dynamic_buffer(size_t sz) : __allocator{}, __my_data{} { __allocate_storage(sz); __zero(__my_data.__begin, sz); }
        __dynamic_buffer(size_t sz, A const& alloc) : __allocator{ alloc }, __my_data{} { __allocate_storage(sz); __zero(__my_data.__begin, sz); }
        __dynamic_buffer(size_t sz, T const& val, A const& alloc) : __allocator{ alloc }, __my_data{ __allocator.allocate(sz), sz } { __set(__my_data.__begin, val, sz); __advance(sz); }
        __dynamic_buffer(initializer_list<T> const& __ils, A const& alloc) : __dynamic_buffer{ __ils.begin(), __ils.end(), alloc } {}
        __dynamic_buffer(__dynamic_buffer const& that) : __dynamic_buffer{ that.__access(), that.__access_end(), that.__allocator } {}
        __dynamic_buffer(__dynamic_buffer const& that, A const& alloc) : __dynamic_buffer{ that.__access(), that.__access_end(), alloc } {}
        __dynamic_buffer(__dynamic_buffer const& that, size_t start, A const& alloc) : __dynamic_buffer{ that.__access() + start, that.__access_end(), alloc } {}
        __dynamic_buffer(__dynamic_buffer const& that, size_t start, size_t count, A const& alloc) : __dynamic_buffer{ that.__access() + start, that.__access() + (count < that.__size() - start ? count : that.__size()), alloc } {}
        __dynamic_buffer(__dynamic_buffer&& that) : __allocator{ move(that.__allocator) }, __my_data{ move(that.__my_data) } {}
        __dynamic_buffer(__dynamic_buffer&& that, A const& alloc) : __allocator{ alloc }, __my_data{ move(that.__my_data) } {}
        ~__dynamic_buffer() { if(__my_data.__begin) __allocator.deallocate(__my_data.__begin, __capacity()); }
        __dynamic_buffer& operator=(__dynamic_buffer const& that) { __my_data.__copy_ptrs(that.__my_data); return *this; }
        __dynamic_buffer& operator=(__dynamic_buffer&& that) { __my_data.__move(move(that.__my_data)); return *this; }
        constexpr __ptr __access() noexcept { return __my_data.__begin; }
        constexpr __const_ptr __access() const noexcept { return __my_data.__begin; }
        constexpr __ptr __access_end() noexcept { return __my_data.__end; }
        constexpr __const_ptr __access_end() const noexcept { return __my_data.__end; }
        constexpr __ref __get(size_t __i) { return __my_data.__begin[__i]; }
        constexpr __const_ref __get(size_t __i) const { return __my_data.__begin[__i]; }
        constexpr __ref __get_last() noexcept { return *__my_data.__end; }
        constexpr __const_ref __get_last() const noexcept { return *__my_data.__end; }
    };
}
#endif