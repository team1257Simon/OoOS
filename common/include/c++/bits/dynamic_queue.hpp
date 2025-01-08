#ifndef __DYN_QUEUE
#define __DYN_QUEUE
#include "memory"
#include "limits"
#include "bits/stl_iterator.hpp"
#include "kernel/libk_decls.h"
#include "initializer_list"
#include "bits/stdexcept.h"
#include "bits/stl_algobase.hpp"
namespace std::__impl
{
    template<typename T>
    struct __queue_ptrs
    {
        typedef T* __ptr_type;
        typedef T const* __const_ptr_type;
        typedef decltype(sizeof(T)) __size_type;
        typedef decltype(declval<T*>() - declval<T*>()) __diff_type;
        __ptr_type __begin{};   // Never changes. Extracted elements remain between here and __next; an "undo" therefore just means decreasing __next
        __ptr_type __next{};    // Points to one past the last element EXTRACTED FROM the queue.
        __ptr_type __end{};     // Points to one past the last element INSERTED INTO the queue.
        __ptr_type __qmax{};     // Points to the end of allocated storage.
        void __reset() noexcept { __begin = __next = __end = __qmax = __ptr_type{}; }
        constexpr __queue_ptrs() = default;
        constexpr __queue_ptrs(__ptr_type beg, __ptr_type mx) noexcept : __begin{ beg }, __next{ beg }, __end{ beg }, __qmax{ mx } {}
        constexpr __queue_ptrs(__ptr_type beg, __ptr_type nx, __ptr_type ed, __ptr_type mx) noexcept : __begin{ beg }, __next{ nx }, __end{ ed }, __qmax{ mx } {}
        constexpr __queue_ptrs(__queue_ptrs const& that) noexcept : __begin{ that.__begin }, __next{ that.__next }, __end{ that.__end }, __qmax{ that.__qmax } {}
        constexpr __queue_ptrs(__queue_ptrs&& that) noexcept : __begin{ that.__begin }, __next{ that.__next }, __end{ that.__end }, __qmax{ that.__qmax } { that.__reset(); }
        constexpr __queue_ptrs(__ptr_type beg, __size_type sz) noexcept : __queue_ptrs{ beg, beg + sz } {}
        constexpr __ptr_type __qgetptr(__size_type offs) noexcept { return __begin + offs;  }
        constexpr void __bumpc(__diff_type n = __diff_type(1L)) noexcept { __end += n; }
        constexpr void __bumpn(__diff_type n = __diff_type(1L)) noexcept { __next += n; }
        constexpr void __setc(__ptr_type where) noexcept { __end = where; }
        constexpr void __setc(__size_type offs) noexcept { __end = __begin + offs; }
        constexpr void __setn(__ptr_type where) noexcept { __next = where; }
        constexpr void __setn(__size_type offs) noexcept { __next = __begin + offs; }
        constexpr void __set_ptrs(__ptr_type beg, __ptr_type nx, __ptr_type ed, __ptr_type mx) noexcept { this->__begin = beg; this->__next = nx; this->__end = ed; this->__qmax = mx; }
        constexpr void __set_ptrs(__ptr_type beg, __size_type sz) noexcept { this->__set_ptrs(beg, beg, beg, beg + sz); }
        constexpr void __set_ptrs(__ptr_type beg, __size_type nx, __size_type ed, __size_type sz) noexcept { this->__set_ptrs(beg, beg + nx, beg + ed, beg + sz); }
        constexpr void __copy_ptrs(__queue_ptrs const& that) noexcept { this->__begin = that.__begin; this->__next = that.__next; this->__end = that.__end; this->__qmax = that.__qmax; }
        constexpr void __move(__queue_ptrs&& that) noexcept { this->__copy_ptrs(that); that.__reset(); }
        constexpr void __swap(__queue_ptrs& that) { __queue_ptrs tmp{}; tmp.__copy_ptrs(*this); this->__copy_ptrs(that); that.__copy_ptrs(tmp); }
    };
    template<typename T, allocator_object<T> A>
    struct __dynamic_queue
    {
        typedef T __value_type;
        typedef A __alloc_type;
        typedef __queue_ptrs<__value_type> __ptr_container;
        typedef typename __ptr_container::__ptr_type __ptr;
        typedef typename __ptr_container::__const_ptr_type __const_ptr;
        typedef typename __ptr_container::__size_type __size_type;
        typedef typename __ptr_container::__diff_type __diff_type;
        typedef decltype(*(declval<__ptr>())) __ref;
        typedef decltype(*(declval<__const_ptr>())) __const_ref;
        __alloc_type __qallocator;
        __ptr_container __my_queue_data;
        template<std::matching_input_iterator<T> IT> void __qtransfer(T* where, IT start, IT end) { for(IT i = start; i != end; i++, where++) { *where = *i; } }
        virtual void __qset(__ptr where, __value_type const& val, __size_type n) { arrayset<T>(where, val, n); }
        virtual void __qzero(__ptr where, __size_type n)  { if constexpr(is_integral_v<T>) arrayset<T>(where, 0, n); else for(__size_type i = 0; i < n; i++, (void)++where) { where->~T(); } }
        virtual void __qcopy(__ptr where, __const_ptr src, __size_type n) { arraycopy<T>(where, src, n); }
        /**
         * Called whenever elements are pushed to or popped from the queue.
         * As with dynamic_buffer, the setn/sete/bumpn/bumpe functions do not call this function.
         * Inheritors can override to add functionality that needs to be invoked whenever the set's elements are modified.
         * The default implementation does nothing.
         */
        virtual void __q_on_modify() {}
        constexpr __ptr __qbeg() noexcept { return __my_queue_data.__begin; }
        constexpr __const_ptr __qbeg() const noexcept { return __my_queue_data.__begin; }
        constexpr __ptr __qcur() noexcept { return __my_queue_data.__next; }
        constexpr __const_ptr __qcur() const noexcept { return __my_queue_data.__next; }
        constexpr __ptr __end() noexcept { return __my_queue_data.__end; }
        constexpr __const_ptr __end() const noexcept { return __my_queue_data.__end; }
        constexpr __ptr __qmax() noexcept { return __my_queue_data.__qmax; }
        constexpr __const_ptr __qmax() const noexcept { return __my_queue_data.__qmax; }
        constexpr __ptr __qgetptr(__size_type __i) noexcept { return __qbeg() + __i; }
        constexpr __const_ptr __qgetptr(__size_type __i) const noexcept { return __qbeg() + __i; }
        constexpr bool __q_out_of_range(__const_ptr pos) const noexcept { return pos < __qbeg() || pos >= __qmax(); }
        constexpr bool __q_out_of_range(__const_ptr start, __const_ptr end) const noexcept { return __q_out_of_range(start) || __q_out_of_range(end) || end < start; }
        constexpr void __bumpn(__diff_type n = __diff_type(1L)) noexcept { this->__my_queue_data.__bumpn(n); }
        constexpr void __bumpe(__diff_type n = __diff_type(1L)) noexcept { this->__my_queue_data.__bumpc(n); }
        constexpr void __qsetn(__ptr where) noexcept { if(!__q_out_of_range(where) && where <= __end()) this->__my_queue_data.__setn(where); }
        constexpr void __qsetn(__size_type n) noexcept { __qsetn(this->__qgetptr(n)); }
        constexpr void __qsete(__ptr where) noexcept { if(!__q_out_of_range(where)) this->__my_queue_data.__setc(where); }
        constexpr void __qsete(__size_type n) noexcept { __qsete(this->__qgetptr(n)); }
        constexpr __ref __qget(__size_type __i) { return *__qgetptr(__i); }
        constexpr __const_ref __qget(__size_type __i) const { return *__qgetptr(__i); }
        constexpr __ptr __peek_next() noexcept { return __qcur(); }
        constexpr __const_ptr __peek_next() const noexcept { return __qcur(); }
        constexpr bool __have_next() const noexcept { return __qcur() < __end(); }
        constexpr __ptr __pop_next() noexcept { if(__qcur() < __end()) { __ptr result = __qcur(); this->__bumpn(); return result; } return nullptr; }
        constexpr __ptr __unpop() noexcept { if(__qcur() > __qbeg()) this->__bumpn(-1); return __qcur(); }
        constexpr void __qsetp(__ptr_container const& ptrs) noexcept { this->__my_queue_data.__copy_ptrs(ptrs); }
        constexpr __size_type __q_max_capacity() const noexcept { return std::numeric_limits<__size_type>::max(); }
        constexpr __size_type __qsize() const noexcept { return __size_type(__end() - __qbeg()); }
        constexpr __size_type __qrem() const noexcept { return __size_type(__end() - __qcur()); }
        constexpr __size_type __stell() const noexcept { return __size_type(__qcur() - __qbeg()); }
        constexpr __diff_type __tell() const noexcept{ return __qcur() - __qbeg(); }
        constexpr __size_type __qcapacity() const noexcept { return __size_type(__qmax() - __qbeg()); }
        constexpr __size_type __cap_rem() const noexcept { return __size_type(__qmax() - __end()); }
        constexpr void __qmove(__dynamic_queue&& that) noexcept { __my_queue_data.__move(std::move(that.__my_queue_data)); }
        constexpr void __qpcopy(__dynamic_queue const& that) noexcept { __my_queue_data.__copy_ptrs(that.__my_queue_data); }
        void __q_trim_buffer() throw() { __size_type sz = __qsize(); this->__my_queue_data.__set_ptrs(resize<T>(this->__qbeg(), sz), sz); this->__q_on_modify(); }
        template<typename ... Args> requires std::constructible_from<T, Args...> __ptr __construct_element(__size_type where, Args&& ... args) { if(__q_out_of_range(__qbeg() + where)) return nullptr; else return construct_at(__qbeg() + where, forward<Args>(args)...); }
        virtual bool __q_grow_buffer(__size_type added);
        __ptr __push_elements(T const&, __size_type = 1);
        template<std::matching_input_iterator<T> IT> __ptr __push_elements(IT start, IT end) requires(!std::is_same_v<IT, __const_ptr>);
        __ptr __push_elements(__const_ptr start, __const_ptr end);
        template<std::output_iterator<T> IT> __size_type __pop_elements(IT out_start, IT out_end) requires(!std::is_same_v<IT, __ptr>);
        __size_type __pop_elements(__ptr out_start, __ptr out_end);
        void __qdestroy() { if(__qbeg()) { __qallocator.deallocate(__qbeg(), __qcapacity()); __my_queue_data.__reset(); } }
        void __qclear() { __size_type cap = __qcapacity(); __qdestroy(); __my_queue_data.__set_ptrs(__qallocator.allocate(cap), cap); this->__q_on_modify(); }
        constexpr __dynamic_queue() = default;
        __dynamic_queue(__size_type sz, __alloc_type alloc = __alloc_type{}) : __qallocator{ alloc }, __my_queue_data{ __qallocator.allocate(sz), sz } {}
        __dynamic_queue(__dynamic_queue const& that) : __qallocator{ that.__qallocator }, __my_queue_data{ that.__my_queue_data } {}
        __dynamic_queue(__dynamic_queue&& that) : __qallocator{ that.__qallocator }, __my_queue_data{ move(that.__my_queue_data) } {}
        ~__dynamic_queue() { __qdestroy(); }
        __dynamic_queue& operator=(__dynamic_queue&& that) { __qdestroy(); this->__qmove(std::forward<__dynamic_queue>(that)); return *this; }
        __dynamic_queue& operator=(__dynamic_queue const& that) { __qdestroy(); this->__qpcopy(that); return *this;  }
    };
    template <typename T, allocator_object<T> A>
    bool __dynamic_queue<T, A>::__q_grow_buffer(typename __dynamic_queue<T, A>::__size_type added)
    {
        if(!added) return true; // Vacuously true success value
        __size_type num_elements = __qsize();
        __size_type nx_offs = __stell();
        __size_type target_cap = __qcapacity() + added;
        try { __my_queue_data.__set_ptrs(resize<T>(__qbeg(), target_cap), nx_offs, num_elements, target_cap); }
        catch(std::exception&) { return false; }
        return true;
    }
    template <typename T, allocator_object<T> A>
    typename __dynamic_queue<T, A>::__ptr __dynamic_queue<T, A>::__push_elements(T const &t, typename __dynamic_queue<T, A>::__size_type count)
    {
        if(__qmax() <= __end() + count && !this->__q_grow_buffer(__size_type(count - this->__cap_rem()))) return nullptr;
        this->__qset(this->__end(), t, count);
        this->__bumpe(count);
        this->__q_on_modify();
        return this->__end();
    }
    template <typename T, allocator_object<T> A>
    typename __dynamic_queue<T, A>::__ptr __dynamic_queue<T, A>::__push_elements(__const_ptr start, __const_ptr end)
    {
        if(end < start) return nullptr;
        if(end == start) return __end();
        __size_type n = end - start;
        if(this->__qmax() <= this->__end() + n && !this->__q_grow_buffer(__size_type(n - this->__cap_rem()))) return nullptr;
        this->__qcopy(this->__end(), start, n);
        this->__bumpe(n);
        this->__q_on_modify();
        return this->__end();
    }
    template <typename T, allocator_object<T> A>
    inline typename __dynamic_queue<T, A>::__size_type __dynamic_queue<T, A>::__pop_elements(__ptr out_start, __ptr out_end)
    {
        if(out_end <= out_start) return 0uL;
        __size_type count = std::min(this->__qrem(), __size_type(out_end - out_start));
        this->__qcopy(out_start, this->__qcur(), count);
        this->__bumpn(count);
        this->__q_on_modify();
        return count;
    }
    template <typename T, allocator_object<T> A>
    template <std::matching_input_iterator<T> IT>
    typename __dynamic_queue<T, A>::__ptr __dynamic_queue<T, A>::__push_elements(IT start, IT end)
    requires(!std::is_same_v<IT, typename __dynamic_queue<T, A>::__const_ptr>)
    {
        if(end < start) return nullptr;
        if(end == start) return __end();
        __size_type n = end - start;
        if(this->__qmax() <= this->__end() + n && !this->__q_grow_buffer(size_t(n - this->__cap_rem()))) return nullptr;
        this->__qtransfer(this->__end(), start, end);
        this->__bumpe(n);
        this->__q_on_modify();
        return this->__end();
    }
    template <typename T, allocator_object<T> A>
    template <std::output_iterator<T> IT>
    inline typename __dynamic_queue<T, A>::__size_type __dynamic_queue<T, A>::__pop_elements(IT out_start, IT out_end)
    requires(!std::is_same_v<IT, typename __dynamic_queue<T, A>::__ptr>)
    {
        if(out_end <= out_start) return 0uL;
        __size_type n = 0;
        for(IT i = out_start; i != out_end && this->__qcur() != this->__end(); ++i, ++n, this->__bumpn()) *i = *this->__qcur();
        this->__q_on_modify();
        return n;
    }
}
#endif