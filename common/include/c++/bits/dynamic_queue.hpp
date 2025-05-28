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
        typedef T* __ptr;
        typedef T const* __const_ptr;
        typedef decltype(sizeof(T)) __size_type;
        typedef decltype(declval<__ptr>() - declval<__ptr>()) __diff_type;
        __ptr __begin{};   // Points to the start of allocated storage. Extracted elements remain between here and __next until the buffer is trimmed for stale elements.
        __ptr __next{};    // Points to one past the last element EXTRACTED FROM the queue.
        __ptr __end{};     // Points to one past the last element INSERTED INTO the queue.
        __ptr __qmax{};    // Points to the end of allocated storage.
        void __reset() noexcept { __begin = __next = __end = __qmax = __ptr{}; }
        constexpr __queue_ptrs() noexcept = default;
        constexpr __queue_ptrs(__ptr beg, __ptr mx) noexcept : __begin{ beg }, __next{ beg }, __end{ beg }, __qmax{ mx } {}
        constexpr __queue_ptrs(__ptr beg, __ptr nx, __ptr ed, __ptr mx) noexcept : __begin{ beg }, __next{ nx }, __end{ ed }, __qmax{ mx } {}
        constexpr __queue_ptrs(__queue_ptrs const& that) noexcept : __begin{ that.__begin }, __next{ that.__next }, __end{ that.__end }, __qmax{ that.__qmax } {}
        constexpr __queue_ptrs(__queue_ptrs&& that) noexcept : __begin{ that.__begin }, __next{ that.__next }, __end{ that.__end }, __qmax{ that.__qmax } { that.__reset(); }
        constexpr __queue_ptrs(__ptr beg, __size_type sz) noexcept : __queue_ptrs{ beg, beg + sz } {}
        constexpr __ptr __q_get_ptr(__size_type offs) noexcept { return __begin + offs; }
        constexpr void __bumpc(__diff_type n = __diff_type(1L)) noexcept { __end += n; }
        constexpr void __bumpn(__diff_type n = __diff_type(1L)) noexcept { __next += n; }
        constexpr void __setc(__ptr where) noexcept { __end = where; }
        constexpr void __setc(__size_type offs) noexcept { __end = __begin + offs; }
        constexpr void __setn(__ptr where) noexcept { __next = where; }
        constexpr void __setn(__size_type offs) noexcept { __next = __begin + offs; }
        constexpr void __set_ptrs(__ptr beg, __ptr nx, __ptr ed, __ptr mx) noexcept { __begin = beg; __next = nx; __end = ed; __qmax = mx; }
        constexpr void __set_ptrs(__ptr beg, __size_type sz) noexcept { __set_ptrs(beg, beg, beg, beg + sz); }
        constexpr void __set_ptrs(__ptr beg, __size_type nx, __size_type ed, __size_type sz) noexcept { __set_ptrs(beg, beg + nx, beg + ed, beg + sz); }
        constexpr void __copy_ptrs(__queue_ptrs const& that) noexcept { __begin = that.__begin; __next = that.__next; __end = that.__end; __qmax = that.__qmax; }
        constexpr void __move(__queue_ptrs&& that) noexcept { __copy_ptrs(that); that.__reset(); }
        constexpr void __swap(__queue_ptrs& that) { __queue_ptrs tmp{}; tmp.__copy_ptrs(*this); __copy_ptrs(that); that.__copy_ptrs(tmp); }
    };
    template<typename T, allocator_object<T> A>
    struct __dynamic_queue
    {
        typedef T __value_type;
        typedef A __alloc_type;
        typedef __queue_ptrs<__value_type> __ptr_container;
        typedef typename __ptr_container::__ptr __ptr;
        typedef typename __ptr_container::__const_ptr __const_ptr;
        typedef typename __ptr_container::__size_type __size_type;
        typedef typename __ptr_container::__diff_type __diff_type;
        typedef deref_t<__ptr> __ref;
        typedef deref_t<__const_ptr> __const_ref;
        __alloc_type __qallocator;
        __ptr_container __my_queue_data;
        __diff_type __stale_size_thresh     { 16L };
        unsigned int __stale_op_thresh      { 3U };
        unsigned int __op_cnt               { 0U };
        template<matching_input_iterator<T> IT> constexpr void __qtransfer(__ptr where, IT start, IT end) { if constexpr(contiguous_iterator<IT>) array_copy(where, addressof(*start), static_cast<__size_type>(distance(start, end))); else for(IT i = start; i != end; i++, where++) { *where = *i; } }
        constexpr void __qset(__ptr where, __value_type const& val, __size_type n) { fill_n(where, n, val); }
        constexpr void __qassign(__ptr where, __value_type&& val) { *where = move(val); }
        constexpr void __qzero(__ptr where, __size_type n) { array_zero(where, n); }
        constexpr void __qcopy(__ptr where, __const_ptr src, __size_type n) { array_copy(where, src, n); }
        /**
         * Called whenever elements are pushed to or popped from the queue.
         * As with dynamic_buffer, the setn/sete/bumpn/bumpe functions do not call this function.
         * Inheritors can override to add functionality that needs to be invoked whenever the set's elements are modified, such as trimming stale elements.
         * The default implementation does nothing. 
         */
        extension virtual void on_modify_queue() {}
        constexpr void __qassign(__ptr where, __value_type&& val, __size_type n) { for(__size_type i = 0; i < n; i++, ++where) { __qassign(where, move(val)); } }
        constexpr __ptr __qbeg() noexcept { return __my_queue_data.__begin; }
        constexpr __const_ptr __qbeg() const noexcept { return __my_queue_data.__begin; }
        constexpr __ptr __qcur() noexcept { return __my_queue_data.__next; }
        constexpr __const_ptr __qcur() const noexcept { return __my_queue_data.__next; }
        constexpr __ptr __end() noexcept { return __my_queue_data.__end; }
        constexpr __const_ptr __end() const noexcept { return __my_queue_data.__end; }
        constexpr __ptr __qmax() noexcept { return __my_queue_data.__qmax; }
        constexpr __const_ptr __qmax() const noexcept { return __my_queue_data.__qmax; }
        constexpr __ptr __q_get_ptr(__size_type __i) noexcept { return __qbeg() + __i; }
        constexpr __const_ptr __q_get_ptr(__size_type __i) const noexcept { return __qbeg() + __i; }
        constexpr bool __q_out_of_range(__const_ptr pos) const noexcept { return pos < __qbeg() || pos >= __qmax(); }
        constexpr bool __q_out_of_range(__const_ptr start, __const_ptr end) const noexcept { return __q_out_of_range(start) || __q_out_of_range(end) || end < start; }
        constexpr void __bumpn(__diff_type n = 1Z) noexcept { __my_queue_data.__bumpn(n); }
        constexpr void __bumpe(__diff_type n = 1Z) noexcept { __my_queue_data.__bumpc(n); }
        constexpr void __qsetn(__ptr where) noexcept { if(!__q_out_of_range(where) && where <= __end()) __my_queue_data.__setn(where); }
        constexpr void __qsetn(__size_type n) noexcept { __qsetn(__q_get_ptr(n)); }
        constexpr void __qsete(__ptr where) noexcept { if(!__q_out_of_range(where)) __my_queue_data.__setc(where); }
        constexpr void __qsete(__size_type n) noexcept { __qsete(__q_get_ptr(n)); }
        constexpr void __qrst() noexcept { __qsetn(__qbeg()); /* Not an EKG though */ }
        constexpr __ref __qget(__size_type __i) { return *__q_get_ptr(__i); }
        constexpr __const_ref __qget(__size_type __i) const { return *__q_get_ptr(__i); }
        constexpr __ptr __peek_next() noexcept { return __qcur(); }
        constexpr __const_ptr __peek_next() const noexcept { return __qcur(); }
        constexpr bool __have_next() const noexcept { return __qcur() < __end(); }
        constexpr __ptr __pop_next() noexcept { if(__qcur() < __end()) { __ptr result = __qcur(); __bumpn(); return result; } return nullptr; }
        constexpr __ptr __unpop() noexcept { if(__qcur() > __qbeg()) { __bumpn(-1Z); __op_cnt = 0U; return __qcur(); } return nullptr; }
        constexpr void __qsetp(__ptr_container const& ptrs) noexcept { __my_queue_data.__copy_ptrs(ptrs); }
        constexpr __size_type __q_max_capacity() const noexcept { return numeric_limits<__size_type>::max(); }
        constexpr __size_type __qsize() const noexcept { return static_cast<__size_type>(__end() - __qbeg()); }
        constexpr __size_type __qrem() const noexcept { return static_cast<__size_type>(__end() - __qcur()); }
        constexpr __size_type __stell() const noexcept { return static_cast<__size_type>(__qcur() - __qbeg()); }
        constexpr __diff_type __tell() const noexcept { return __qcur() - __qbeg(); }
        constexpr __size_type __qcapacity() const noexcept { return static_cast<__size_type>(__qmax() - __qbeg()); }
        constexpr __size_type __cap_rem() const noexcept { return static_cast<__size_type>(__qmax() - __end()); }
        constexpr void __set_stale_op_threshold(unsigned int value) noexcept { __stale_op_thresh = value; }
        constexpr void __set_stale_size_threshold(__diff_type value) noexcept { __stale_size_thresh = value; }
        constexpr void __q_state_assign(__dynamic_queue const& that) noexcept { __stale_op_thresh = that.__stale_op_thresh; __stale_size_thresh = that.__stale_size_thresh; __op_cnt = that.__op_cnt; }
        constexpr void __q_move_assign(__dynamic_queue&& that) noexcept { __my_queue_data.__move(move(that.__my_queue_data)); __q_state_assign(that); }
        constexpr void __q_copy_assign(__dynamic_queue const& that) noexcept { __my_queue_data.__set_ptrs(__qallocator.allocate(that.__qcapacity()), that.__qcapacity()); __qcopy(__qbeg(), that.__qbeg(), that.__qcapacity()); __q_state_assign(that); }
        constexpr __size_type __erase_before_next();
        constexpr bool __is_stale() const noexcept { return __op_cnt > __stale_op_thresh && (__my_queue_data.__next - __my_queue_data.__begin) > __stale_size_thresh; }
        /**
         * If there have been a number of push operations exceeding the configured operation threshold since the most recent pop or trim,
         * and the number of elements between the base and current pointers exceeds the configured size threshold (defaults are 3 and 16 respectively),
         * reallocates the buffer with the same capacity but with the element at the current pointer moved to be at the start, 
         * sets the position of that pointer to the new start, and returns the number of elements removed. 
         * Otherwise, does nothing and returns zero.
         */
        constexpr __size_type __trim_stale() { return __is_stale() ? __erase_before_next() : __size_type(0UL); }
        constexpr bool __q_grow_buffer(__size_type added);
        constexpr __ptr __push_elements(T const&, __size_type = 1UZ);
        constexpr __ptr __push_elements(T&&, __size_type = 1UZ);
        template<matching_input_iterator<T> IT> constexpr __ptr __push_elements(IT start, IT end) requires(!is_same_v<IT, __const_ptr>);
        template<matching_input_iterator<T> IT> constexpr __ptr __push_elements(IT what) requires(!is_same_v<IT, __const_ptr>) { IT end = what; end++; return __push_elements(what, end); }
        constexpr __ptr __push_elements(__const_ptr start, __const_ptr end);
        template<output_iterator<T> IT> constexpr __size_type __pop_elements(IT out_start, IT out_end) requires(!is_same_v<IT, __ptr>);
        constexpr __size_type __pop_elements(__ptr out_start, __ptr out_end);
        constexpr __size_type __erase_elements(__const_ptr start, __size_type n = 1UZ);
        constexpr __size_type __force_trim() { return __erase_before_next(); }
        constexpr __ptr __insert(__const_ptr where, __const_ref what, __size_type how_many = 1UZ);
        template<typename ... Args> requires constructible_from<T, Args...> constexpr __ptr __emplace_element(Args&& ... args) { if(__qmax() <= __end() && !__q_grow_buffer(1UL)) return nullptr; else { __ptr result = construct_at(__end(), forward<Args>(args)...); __bumpe(1L); return result; } }
        constexpr void __qdestroy() { if(__qbeg()) { __qallocator.deallocate(__qbeg(), __qcapacity()); __my_queue_data.__reset(); } __op_cnt = 0; }
        constexpr void __qclear() { __size_type cap = __qcapacity(); __qdestroy(); __my_queue_data.__set_ptrs(__qallocator.allocate(cap), cap); on_modify_queue(); }
        constexpr void __qswap(__dynamic_queue& that) noexcept { __my_queue_data.__swap(that.__my_queue_data); }
        constexpr __dynamic_queue() noexcept : __qallocator{}, __my_queue_data{} {}
        constexpr __dynamic_queue(__size_type sz, __alloc_type alloc = __alloc_type{}) : __qallocator{ alloc }, __my_queue_data{ __qallocator.allocate(sz), sz } {}
        constexpr __dynamic_queue(__dynamic_queue const& that) : __qallocator{ that.__qallocator }, __my_queue_data{ that.__my_queue_data } {}
        constexpr __dynamic_queue(__dynamic_queue&& that) : __qallocator{ that.__qallocator }, __my_queue_data{ move(that.__my_queue_data) } {}
        constexpr ~__dynamic_queue() { __qdestroy(); }
        constexpr __dynamic_queue& operator=(__dynamic_queue&& that) { __qdestroy(); __q_move_assign(move(that)); return *this; }
        constexpr __dynamic_queue& operator=(__dynamic_queue const& that) { __qdestroy(); __q_copy_assign(that); return *this; }
    };
    template<typename T, allocator_object<T> A>
    constexpr typename __dynamic_queue<T, A>::__size_type __dynamic_queue<T, A>::__erase_before_next()
    {
        size_t result = __tell();
        __ptr_container tmp{ __qallocator.allocate(__qcapacity()), __qcapacity() };
        array_move(tmp.__begin, __qcur(), __qrem());
        tmp.__bumpc(__qrem());
        __qdestroy();
        __my_queue_data.__copy_ptrs(tmp);
        return result;
    }
    template<typename T, allocator_object<T> A>
    constexpr bool __dynamic_queue<T, A>::__q_grow_buffer(typename __dynamic_queue<T, A>::__size_type added)
    {
        if(__unlikely(!added)) return true; // Vacuously true success value
        __size_type num_elements = __qsize();
        __size_type nx_offs = __stell();
        __size_type target_cap = __qcapacity() + added;
        try { __my_queue_data.__set_ptrs(resize(__qbeg(), __qcapacity(), target_cap, __qallocator), nx_offs, num_elements, target_cap); }
        catch(...) { return false; }
        return true;
    }
    template<typename T, allocator_object<T> A>
    constexpr typename __dynamic_queue<T, A>::__ptr __dynamic_queue<T, A>::__push_elements(T const& t, typename __dynamic_queue<T, A>::__size_type count)
    {
        if(__qmax() <= __end() + count && __unlikely(!__q_grow_buffer(static_cast<__size_type>(count - __cap_rem())))) return nullptr;
        __qset(__end(), t, count);
        __bumpe(count);
        __op_cnt++;
        on_modify_queue();
        return __end();
    }
    template<typename T, allocator_object<T> A>
    constexpr typename __dynamic_queue<T, A>::__ptr __dynamic_queue<T, A>::__push_elements(T&& t, typename __dynamic_queue<T, A>::__size_type count)
    {
        if(__qmax() <= __end() + count && __unlikely(!__q_grow_buffer(static_cast<__size_type>(count - __cap_rem())))) return nullptr;
        __qassign(__end(), move(t), count);
        __bumpe(count);
        __op_cnt++;
        on_modify_queue();
        return __end();
    }
    template<typename T, allocator_object<T> A>
    constexpr typename __dynamic_queue<T, A>::__ptr __dynamic_queue<T, A>::__push_elements(__const_ptr start, __const_ptr end)
    {
        if(__unlikely(end < start)) return nullptr;
        if(end == start) return __end();
        __size_type n = end - start;
        if(__qmax() <= __end() + n && __unlikely(!__q_grow_buffer(static_cast<__size_type>(n - __cap_rem())))) return nullptr;
        __qcopy(__end(), start, n);
        __bumpe(n);
        __op_cnt++;
        on_modify_queue();
        return __end();
    }
    template<typename T, allocator_object<T> A>
    constexpr typename __dynamic_queue<T, A>::__size_type __dynamic_queue<T, A>::__pop_elements(__ptr out_start, __ptr out_end)
    {
        if(__unlikely(out_end <= out_start)) return 0UL;
        __size_type count = min(__qrem(), static_cast<__size_type>(out_end - out_start));
        __qcopy(out_start, __qcur(), count);
        __bumpn(count);
        __op_cnt = 0U;
        on_modify_queue();
        return count;
    }
    template<typename T, allocator_object<T> A>
    template<matching_input_iterator<T> IT>
    constexpr typename __dynamic_queue<T, A>::__ptr __dynamic_queue<T, A>::__push_elements(IT start, IT end)
    requires(!is_same_v<IT, typename __dynamic_queue<T, A>::__const_ptr>)
    {
        if(__unlikely(end < start)) return nullptr;
        if(end == start) return __end();
        __size_type n = end - start;
        if(__qmax() <= __end() + n && !__q_grow_buffer(static_cast<__size_type>(n - __cap_rem()))) return nullptr;
        __qtransfer(__end(), start, end);
        __bumpe(n);
        __op_cnt++;
        on_modify_queue();
        return __end();
    }
    template<typename T, allocator_object<T> A>
    template<output_iterator<T> IT>
    constexpr typename __dynamic_queue<T, A>::__size_type __dynamic_queue<T, A>::__pop_elements(IT out_start, IT out_end)
    requires(!is_same_v<IT, typename __dynamic_queue<T, A>::__ptr>)
    {
        if(__unlikely(out_end <= out_start)) return 0UL;
        __size_type n = 0UZ;
        for(IT i = out_start; i != out_end && __qcur() != __end(); ++i, ++n, __bumpn()) *i = *__qcur();
        __op_cnt = 0;
        on_modify_queue();
        return n;
    }
    template<typename T, allocator_object<T> A>
    constexpr typename __dynamic_queue<T, A>::__size_type __dynamic_queue<T, A>::__erase_elements(__const_ptr start, __size_type n)
    {
        if(__unlikely(__q_out_of_range(start))) return 0UL;
        if(start + n > __qmax()) { n = static_cast<__size_type>(__qmax() - start); }
        try 
        {
            if(start == __qbeg() && start + n >= __end()) 
            {
                size_t r = __qsize();
                __qdestroy();
                __my_queue_data.__reset();
                return r;
            }
            __ptr_container tmp{ __qallocator.allocate(__qcapacity() - n), __qcapacity() - n }; 
            if(__end() > start + n) { tmp.__bumpn(__qsize() - n); }
            else if(__end() > start) { tmp.__bumpn(start - __qbeg()); }
            else { tmp.__bumpn(__qsize()); }
            if(__qcur() > start + n) { tmp.__bumpc(__tell() - n); }
            else if(__qcur() > start) tmp.__bumpc(start - __qbeg());
            else { tmp.__bumpc(__tell()); }
            if(tmp.__next > tmp.__end) tmp.__next = tmp.__end;
            __size_type pstart = static_cast<__size_type>(start - __qbeg());
            __size_type srem = static_cast<__size_type>(__qmax() - (start + n));
            array_move(tmp.__begin, __qbeg(), pstart);
            if(srem) array_move(tmp.__q_get_ptr(pstart), __q_get_ptr(pstart) + n, srem);
            __qdestroy();
            __my_queue_data.__copy_ptrs(tmp);
            return n;
        }
        catch(...) { return 0UZ; }
    }
    template <typename T, allocator_object<T> A>
    constexpr typename __dynamic_queue<T, A>::__ptr __dynamic_queue<T, A>::__insert(__const_ptr where, __const_ref what, __size_type how_many)
    {
        if(__unlikely(__q_out_of_range(where))) return nullptr;
        if(__unlikely(!how_many)) return nullptr;
        try
        {
            __ptr_container tmp{ __qallocator.allocate(__qcapacity() + how_many), __qcapacity() + how_many };
            if(where < __qcur()) tmp.__bumpc(__tell() + how_many);
            else tmp.__bumpc(__tell());
            if(where < __end()) tmp.__bumpn(__qsize() + how_many);
            else tmp.__bumpn(where - __qbeg());
            __size_type preface_elems = where - __qbeg();
            if(preface_elems) array_move(tmp.__begin, __qbeg(), preface_elems);
            if(how_many == 1) *(tmp.__q_get_ptr(preface_elems)) = what;
            else __qset(tmp.__q_get_ptr(preface_elems), what, how_many);
            array_move(tmp.__q_get_ptr(preface_elems + how_many), __q_get_ptr(preface_elems), static_cast<__size_type>(__end() - where));
            __qdestroy();
            __my_queue_data.__copy_ptrs(tmp);
            return __q_get_ptr(preface_elems + how_many - 1);
        }
        catch(...) { return nullptr; }
    }
}
#endif