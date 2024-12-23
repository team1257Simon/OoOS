#ifndef __STL_VECTOR
#define __STL_VECTOR
#include "memory"
#include "kernel/libk_decls.h"
#include "initializer_list"
namespace std::__impl
{
    template<typename T, allocator_object<T> A>
    struct __dynamic_buffer
    {
        A __allocator{};
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
            void __reset();
            __data_impl(__data_impl const& that) : __begin{ that.__begin }, __end{ that.__end }, __max{ that.__max } {}
            __data_impl(__data_impl&& that) : __begin{ that.__begin }, __end{ that.__end }, __max{ that.__max } { that.__reset(); }
            __data_impl(__ptr start, __ptr end) : __begin{ start }, __end{ end }, __max { end } {}
            __data_impl(__ptr start, size_t cap) : __begin{ start }, __end{ start }, __max{ start + cap } {}
            void __copy(__data_impl const& that) { this->__begin = that.__begin; this->__end = that.__end; this->__max = that.__max; }
            void __move(__data_impl&& that) { this->__copy(that); that.__reset(); }
            void __swap(__data_impl& that) { __data_impl tmp; tmp.__copy(*this); this->__copy(that); that.__copy(tmp); }
            void __reset() { __begin = __end = __max = __ptr{}; }
        } __my_data;
        void __allocate_storage(size_t n)
        {
            __my_data.__begin = __allocator.allocate(n);
            __my_data.__end  = __my_data.__begin;
            __my_data.__max   = __my_data.__begin + n;
        }
        inline size_t __actual_size() const noexcept { return static_cast<size_t>(__my_data.__end - __my_data.__begin); }
        inline size_t __capacity() const noexcept { return static_cast<size_t>(__my_data.__max = __my_data.__begin); }
        inline void __construct_element(__ptr pos, T const& t) { if(pos < __my_data.__max && pos >= __my_data.__begin) { construct_at(pos, t); if(pos > __my_data.__end) __my_data.__end = pos; }  }
        void __assign_elements(size_t count, T const& t) 
        { 
            if(count > __capacity()) __grow_buffer(count - __capacity());
            arrayset<T>(__my_data.__begin, t, count);
            if (count < __actual_size()) { arrayset<T>(__my_data.__begin + count, 0, __actual_size() - count); }
            __my_data.__end = __my_data.__begin + count;
        }
        void __assign_elements(__const_ptr start, __const_ptr end)
        {
            size_t count = end - start;
            if(count > __capacity()) __grow_buffer(count - __capacity());
            arraycopy<T>(__my_data.__begin, start, count);
            if (count < __actual_size()) { arrayset<T>(__my_data.__begin + count, 0, __actual_size() - count); }
            __my_data.__end = __my_data.__begin + count;
        }
        void __assign_elements(std::initializer_list<T> ini) { __assign_elements(ini.begin(), ini.end()); }
        void __grow_buffer(size_t added)
        {
            size_t num_elements = __actual_size();
            size_t target = __capacity() + added;
            __my_data.__begin = __detail::__aligned_reallocate(__my_data.__begin, target, alignof(T));
            __my_data.__end = __my_data.__begin + num_elements;
            __my_data.__max = __my_data.__begin + target;
        }
        void __trim_buffer() 
        {
            size_t num_elements = __actual_size();
            __my_data.__begin = __detail::__aligned_reallocate(__my_data.__begin, num_elements, alignof(T));
            __my_data.__end = __my_data.__begin + num_elements;
            __my_data.__max = __my_data.__begin + num_elements;
        }
        void __append_elements(__const_ptr range_start, __const_ptr range_end)
        {
            size_t rem = __my_data.__max - __my_data.__end;
            size_t num = range_end - range_start;
            if(num > rem) __grow_buffer(num - rem);
            for(__const_ptr p = range_start; p < range_end; p++)
            {
                construct_at(__my_data.__end, *p);
                __my_data.__end++;
            }
        }
        void __append_element(T const& t) 
        {
            if(!(__my_data.__max > __my_data.__end)) __grow_buffer(1);
            construct_at(__my_data.__end, t);
            __my_data.__end++;
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
                    arraycopy<T>(temp, pos, n);
                    __construct_element(__my_data.__begin + offs, t);
                    arraycopy<T>(__my_data.__begin + offs + 1, temp, n);
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
                    arraycopy<T>(nwdat.__begin, __my_data.__begin, offs);
                    arraycopy<T>(nwdat.__begin + offs + 1, pos, rem);
                    nwdat.__end = nwdat.__begin + __actual_size() + 1;
                }
                else
                {
                    arraycopy<T>(nwdat.__begin, __my_data.__begin, __actual_size());
                    nwdat.__end = nwdat.__begin + offs + 1;
                }
                __allocator.deallocate(__my_data.__begin, __actual_size());
                __my_data.__copy(nwdat);
                __construct_element(__my_data.__begin + offs, t);
            }
            return __my_data.__begin + offs;
        }
        __ptr __insert_range(__const_ptr pos, __const_ptr range_start, __const_ptr range_end)
        {
            if(__my_data.__begin > pos || __my_data.__max <= pos) return nullptr;
            size_t range_size = range_end - range_start;
            size_t offs = pos - __my_data.__begin;
            if(pos + range_size < __my_data.__max)
            {
                if(pos < __my_data.__end)
                {
                    size_t n = __my_data.__end - pos;
                    __ptr temp = __allocator.allocate(n);
                    arraycopy<T>(temp, pos, n);
                    arraycopy<T>(__my_data.__begin + offs, range_start, range_size);
                    arraycopy<T>(__my_data.__begin + offs + range_size, temp, n);
                    __allocator.deallocate(temp, n);
                    __my_data.__end += range_size;
                }
                else 
                {
                    arraycopy<T>(__my_data.__begin + offs, range_start, range_size);
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
                    arraycopy<T>(nwdat.__begin, __my_data.__begin, offs);
                    arraycopy<T>(nwdat.__begin + offs + range_size, pos, rem);
                    nwdat.__end = nwdat.__begin + __actual_size() + range_size;
                }
                else
                {
                    arraycopy<T>(nwdat.__begin, __my_data.__begin, __actual_size());
                    nwdat.__end = nwdat.__begin + offs + range_size;
                }
                __allocator.deallocate(__my_data.__begin, __actual_size());
                __my_data.__copy(nwdat);
                arraycopy<T>(__my_data.__begin + offs, range_start, range_size);
            }
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
        }
        __ptr __erase_at_end(size_t how_many)
        {
            if(how_many >= __actual_size()) __clear();
            else
            {
                __my_data.__end -= how_many;
                arrayset<T>(__my_data.__end, 0, how_many);
            }
            return __my_data.__end;
        }
        __ptr __erase_range(__const_ptr start, __const_ptr end)
        {
            if(__my_data.__begin > start || __my_data.__begin > end || __my_data.__max <= start || __my_data.__max < end || end < start) return nullptr;
            size_t how_many = end - start;
            size_t rem = __my_data.__end - end;
            size_t start_pos = start - __my_data.__begin;
            size_t end_pos = end - __my_data.__begin;
            if(rem == 0) return __erase_at_end(how_many);
            else 
            {
                __ptr temp = __allocator.allocate(rem);
                arraycopy<T>(temp, end, rem);
                arrayset<T>(__my_data.__begin + start_pos, 0, __my_data.__end - start);
                arraycopy<T>(__my_data.__begin + start_pos, temp, rem);
                __allocator.deallocate(temp, rem);
                __my_data.__end = __my_data.__begin + start_pos + rem;
            }
            return __my_data.__begin + start_pos;
        }
        inline __ptr __erase(__const_ptr pos)   { return __erase_range(pos, pos + 1); }
        __dynamic_buffer() = default;
        __dynamic_buffer(A const& alloc) : __allocator{alloc}, __my_data{} {}
        __dynamic_buffer(size_t sz) : __allocator{}, __my_data{} { __allocate_storage(sz); }
        __dynamic_buffer(size_t sz, A const& alloc) : __allocator{ alloc }, __my_data{} { __allocate_storage(sz); }
        ~__dynamic_buffer() { if(__my_data.__begin) __allocator.deallocate(__my_data.__begin, __capacity()); }
        __dynamic_buffer(initializer_list<T> const& __ils) : __allocator{}, __my_data{} 
        {
            __allocate_storage(__ils.size());
            for(T const& t : __ils) __append_element(t);
        }
        __dynamic_buffer(__dynamic_buffer const& that) : __allocator{ that.__allocator }, __my_data{ that.__allocator.allocate(that.__capacity()), that.__capacity() } 
        {
            arraycopy<T>(__my_data.__begin, that.__my_data.__begin, that.__actual_size());
            __my_data.__end = __my_data.__begin + that.__actual_size();
        }
        __dynamic_buffer(__dynamic_buffer&& that) : __allocator{ move(that.__allocator) }, __my_data{ move(that.__my_data) } {}
        __dynamic_buffer& operator=(__dynamic_buffer const& that) { __my_data.__copy(that.__my_data); return *this; }
        __dynamic_buffer& operator=(__dynamic_buffer&& that) { __my_data.__move(move(that.__my_data)); return *this; }
        constexpr __ptr __access() noexcept { return __my_data.__begin; }
        constexpr __const_ptr __access() const noexcept { return __my_data.__begin; }
        constexpr __ptr __access_end() noexcept { return __my_data.__end; }
        constexpr __const_ptr __access_end() const noexcept { return __my_data.__end; }
        constexpr __ref __get(size_t __i) { return __my_data.__begin[i]; }
        constexpr __const_ref __get(size_t __i) const { return __my_data.__begin[i]; }
        constexpr __ref __get_last() noexcept { return *__my_data.__end; }
        constexpr __const_ref __get_last() const noexcept { return *__my_data.__end; }
    };
}
#endif