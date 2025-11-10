/**
 * Base structs for the contiguous STL containers such as vector, streambuf, and string, implementing a lot of the shared functionality of those types.
 * Like much of the namespace std the libk will see, there are things here that are not constexpr that normally would be. 
 * Calls to the heap allocator, for instance, rely on the kernel frame pointer which resides somewhere that can't be constexpr.
 */
#ifndef __DYN_BUFFER
#define __DYN_BUFFER
#include "memory"
#include "limits"
#include "bits/stl_algobase.hpp"
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
		typedef T* __ptr;
		typedef T const* __const_ptr;
		typedef decltype(sizeof(T)) __size_type;
		typedef decltype(declval<__ptr>() - declval<__ptr>()) __diff_type;
		__ptr __begin{};
		__ptr __end{};
		__ptr __max{};
		constexpr __buf_ptrs() = default;
		constexpr __buf_ptrs(__ptr start, __ptr end, __ptr max) : __begin(start), __end(end), __max(max) {}
		void __reset() { __begin = __end = __max = __ptr(); }
		__buf_ptrs(__buf_ptrs const& that) : __begin(that.__begin), __end(that.__end), __max(that.__max) {}
		__buf_ptrs(__buf_ptrs&& that) : __begin(that.__begin), __end(that.__end), __max(that.__max) { that.__reset(); }
		__buf_ptrs(__ptr start, __ptr end) : __begin(start), __end(end), __max(end) {}
		__buf_ptrs(__ptr start, __size_type cap) : __begin(start), __end(start), __max(start + cap) {}
		void __copy_ptrs(__buf_ptrs const& that) { __begin = that.__begin; __end = that.__end; __max = that.__max; }
		void __swap_ptrs(__buf_ptrs& that) { __buf_ptrs tmp; tmp.__copy_ptrs(*this); __copy_ptrs(that); that.__copy_ptrs(tmp); }
		void __set_ptrs(__ptr begin, __ptr end, __ptr max) { __begin = begin; __end = end; __max = max; }
		void __set_ptrs(__ptr begin, __size_type ncap) { __set_ptrs(begin, begin + min(ncap, static_cast<__size_type>(__end - __begin)), begin + ncap); }
		__ptr __get_ptr(__size_type offs) { return __begin + offs; }
		void __setc(__ptr where) { __end = where; }
		void __setc(__size_type offs) { __end = __begin + offs; }
		void __adv(__size_type n) { __end += n; }
		void __bck(__size_type n) { __end -= n; }
		__size_type __capacity() const { return __begin ? static_cast<__size_type>(__max - __begin) : 0UZ; }
	};
	/**
	 * This base-type implements the functionality shared by the dynamic-container types (mainly string and vector). 
	 * If the buffer must be null-terminated, use NT = true. This parameter exists in place of using a virtual member to implement this functionality.
	 * Apparently, using a virtual member in something like std::string can cause problems...who knew.
	 */
	template<typename T, allocator_object<T> A, bool NT = false>
	struct __dynamic_buffer
	{
		typedef __buf_ptrs<T> __container;
		typedef typename __container::__ptr __ptr;
		typedef typename __container::__const_ptr __const_ptr;
		typedef typename __container::__size_type __size_type;
		typedef typename __container::__diff_type __diff_type;
		typedef deref_t<__ptr> __ref;
		typedef deref_t<__const_ptr> __const_ref;
		typedef A __alloc_type;
		constexpr static bool __end_zero 	= NT;
		constexpr static bool __trivial 	= trivial_copy<T>;
		__alloc_type __allocator;
		__container __my_data;
		constexpr static __const_ptr __pmin(__const_ptr a, __const_ptr b) { return a < b ? a : b; }
		/**
		 * Copies data using iterators that might not be contiguous; i.e. not linear (e.g. tree iterators) or not forward-facing (e.g. reverse iterators).
		 * If the iterator happens to be contiguous (e.g. a pointer), this will simplify into the normal copy operation.
		 */
		template<matching_input_iterator<T> IT> constexpr void __transfer(T* where, IT start, IT end);
		constexpr void __set(__ptr where, T const& val, __size_type n) { fill_n(where, n, val); }
		/**
		 * Selects the proper function to clear allocated storage of garbage data.
		 * If the type T is a nontrivial type which cannot be default-constructed, this function does nothing.
		 * If the type is nontrivial and can be default-constructed, it default-constructs each element of the destination array.
		 * Otherwise, it will behave similarly to using memset to zero the memory.
		 */
		constexpr void __zero(__ptr where, __size_type n);
		constexpr void __copy(__ptr where, __const_ptr src, __size_type n) { array_copy(where, src, n); }
		constexpr void __move(__ptr where, __ptr src, __size_type n) { array_move(where, src, n); }
		constexpr bool __grow_buffer(__size_type added);
		extension constexpr void __post_modify_check_nt();
		template<matching_input_iterator<T> IT> constexpr __ptr __append_elements(IT start_it, IT end_it);
		template<matching_input_iterator<T> IT> constexpr __ptr __insert_elements(__const_ptr pos, IT start_ptr, IT end_ptr);
		template<typename ... Args> requires constructible_from<T, Args...> constexpr __ptr __emplace_element(__const_ptr pos, Args&& ... args);
		template<typename ... Args> requires constructible_from<T, Args...> constexpr __ptr __emplace_at_end(Args&& ... args);
		constexpr __ptr __replace_elements(__size_type pos, __size_type count, __const_ptr from, __size_type count2);
		constexpr __ptr __erase_range(__const_ptr start, __const_ptr end);
		constexpr __ptr __insert_element(__const_ptr pos, T const& t) { return __insert_elements(pos, addressof(t), addressof(t) + 1); }
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
		constexpr __ref __get_last() noexcept { return *__get_ptr(__size() - 1); }
		constexpr __const_ref __get_last() const noexcept { return *__get_ptr(__size() - 1); }
		constexpr void __setp(__buf_ptrs<T> const& ptrs) noexcept { __my_data.__copy_ptrs(ptrs); }
		constexpr void __setp(__ptr beg, __ptr cur, __ptr end) noexcept { __my_data.__set_ptrs(beg, cur, end); }
		constexpr void __setp(__ptr beg, __ptr end) noexcept { __setp(beg, beg, end); }
		constexpr void __setn(__ptr beg, __size_type c, __size_type n) noexcept { __setp(beg, beg + c, beg + n); }
		constexpr void __setn(__ptr beg, __size_type n) noexcept { __setp(beg, beg + n); }
		constexpr void __setc(__ptr pos) noexcept { if(__valid_end_pos(pos)) __my_data.__setc(pos); } 
		constexpr void __setc(__size_type pos) noexcept { __setc(__get_ptr(pos)); }
		constexpr void __bumpc(int64_t off) noexcept { __setc(__cur() + off); }
		constexpr void __rst() noexcept { __setc(__beg()); }
		constexpr __size_type __max_capacity() const noexcept { return numeric_limits<__size_type>::max(); }
		constexpr void __advance(__size_type n) { if(__valid_end_pos(__cur() + n)) { __my_data.__adv(n); } else { __setc(__max()); } }
		constexpr void __backtrack(__size_type n) { if(__valid_end_pos(__cur() - n)) { __my_data.__bck(n); } else { __setc(__beg()); } }
		constexpr __size_type __size() const noexcept { return static_cast<__size_type>(__cur() - __beg()); }
		constexpr __size_type __capacity() const noexcept { return static_cast<__size_type>(__max() - __beg()); }
		constexpr __size_type __rem() const noexcept { return static_cast<__size_type>(__max() - __cur()); }
		constexpr __size_type __ediff(__const_ptr pos) const noexcept { return static_cast<__size_type>(__cur() - pos); }
		constexpr void __allocate_storage(__size_type n) { if(n) __setn(__allocator.allocate(n), n); }
		constexpr explicit __dynamic_buffer(A const& alloc) : __allocator(alloc), __my_data() {}
		constexpr __dynamic_buffer() noexcept(noexcept(A())) : __allocator(), __my_data() {}
		constexpr __dynamic_buffer(__size_type sz) : __allocator(), __my_data() { __allocate_storage(sz); }
		constexpr __dynamic_buffer(__size_type sz, A const& alloc) : __allocator(alloc), __my_data() { __allocate_storage(sz); }
		constexpr __dynamic_buffer(__size_type sz, T const& val, A const& alloc) : __allocator(alloc), __my_data(__allocator.allocate(sz), sz) { if(sz) { __set(__beg(), val, sz); __advance(sz); } }
		constexpr __dynamic_buffer(initializer_list<T> const& __ils, A const& alloc) : __dynamic_buffer(__ils.begin(), __ils.end(), alloc) {}
		constexpr __dynamic_buffer(__dynamic_buffer const& that) : __dynamic_buffer(that.__beg(), that.__cur(), that.__allocator) {}
		constexpr __dynamic_buffer(__dynamic_buffer const& that, A const& alloc) : __dynamic_buffer(that.__beg(), that.__cur(), alloc) {}
		constexpr __dynamic_buffer(__dynamic_buffer const& that, __size_type start, A const& alloc) : __dynamic_buffer(that.__get_ptr(start), that.__cur(), alloc) {}
		constexpr __dynamic_buffer(__dynamic_buffer const& that, __size_type start, __size_type count, A const& alloc) : __dynamic_buffer(that.__get_ptr(start), __pmin(that.__get_ptr(start + count), that.__cur()), alloc) {}
		constexpr __dynamic_buffer(__dynamic_buffer&& that) : __allocator(move(that.__allocator)), __my_data(move(that.__my_data)) {}
		constexpr  __dynamic_buffer(__dynamic_buffer&& that, A const& alloc) : __allocator(alloc), __my_data(move(that.__my_data)) {}
		constexpr ~__dynamic_buffer() { if(__beg()) { __allocator.deallocate(__beg(), __capacity()); } }
		constexpr __dynamic_buffer& operator=(__dynamic_buffer const& that) { this->__copy_assign(that); return *this; }
		constexpr __dynamic_buffer& operator=(__dynamic_buffer&& that) { this->__move_assign(move(that)); return *this; }
		constexpr __ptr __assign_elements(initializer_list<T> ini) { return this->__assign_elements(ini.begin(), ini.end()); }
		constexpr __ptr __erase(__const_ptr pos) { return this->__erase_range(pos, pos + 1); }
		constexpr void __trim_buffer()
		{
			__size_type num_elements = __size();
			__setn(resize(__beg(), __capacity(), num_elements), num_elements, num_elements);
			__post_modify_check_nt();
		}
		constexpr void __size_buffer(__size_type n)
		{
			__size_type num_elements = __size();
			__setn(resize(__beg(), __capacity(), n), std::min(n, num_elements), n);
			__post_modify_check_nt();
		}
		constexpr void __construct_element(__ptr pos, T const& t)
		{
			if(__unlikely(__out_of_range(pos))) return;
			construct_at(pos, t);
			if(pos > __cur()) __setc(pos);
		}
		constexpr __ptr __assign_elements(__size_type count, T const& t)
		{
			if(count > __capacity() && __unlikely(!__grow_buffer(count - __capacity()))) return nullptr;
			__set(__beg(), t, count);
			if(count < __size()) __zero(__get_ptr(count), __size() - count);
			__setc(count);
			__post_modify_check_nt();
			return __cur();
		}
		constexpr __ptr __assign_elements(__const_ptr start, __const_ptr end)
		{
			__size_type count = end - start;
			if(count > __capacity() && __unlikely(!__grow_buffer(count - __capacity()))) return nullptr;
			__copy(__beg(), start, count);
			if(count < __size()) __zero(__get_ptr(count), __size() - count);
			__setc(count);
			__post_modify_check_nt();
			return __cur();
		}
		constexpr __ptr __replace_elements(__const_ptr start, __const_ptr end, __const_ptr from, __size_type count)
		{
			if(__unlikely(__out_of_range(start, end)))
				return nullptr;
			return __replace_elements(start - __beg(), end - start, from, count);
		}
		constexpr __ptr __append_elements(__size_type count, T const& t)
		{
			if(!(__max() > __cur() + count) && __unlikely(!__grow_buffer(static_cast<__size_type>(count - __rem())))) return nullptr;
			for(__size_type i = 0; i < count; i++, __advance(1UZ)) construct_at(__cur(), t);
			__post_modify_check_nt();
			return __cur();
		}
		constexpr __ptr __append_element(T const& t)
		{
			if(!(__max() > __cur()) && __unlikely(!__grow_buffer(1UZ))) return nullptr;
			construct_at(__cur(), t);
			__advance(1UZ);
			__post_modify_check_nt();
			return __cur();
		}
		constexpr void __clear()
		{
			__size_type cap = __capacity();
			__destroy();
			__allocate_storage(cap);
			__post_modify_check_nt();
			if constexpr(__trivial)	__zero(__beg(), __capacity());
		}
		constexpr __ptr __erase_at_end(__size_type how_many)
		{
			if(__unlikely(how_many >= __size())) __clear();
			else { __backtrack(how_many); __zero(__cur(), how_many); }
			__post_modify_check_nt();
			return __cur();
		}
		constexpr void __copy_assign(__dynamic_buffer const& that)
		{
			__destroy(); if(__unlikely(!that.__beg())) return;
			__allocate_storage(that.__capacity());
			__copy(__beg(), that.__beg(), that.__capacity());
			__advance(that.__size());
		}
		constexpr void __destroy()
		{
			if(__unlikely(!__beg())) return;
			__allocator.deallocate(__beg(), __capacity());
			__my_data.__reset();
		}
		constexpr void __swap(__dynamic_buffer& that)
		{
			this->__my_data.__swap_ptrs(that.__my_data);
			this->__post_modify_check_nt();
			that.__post_modify_check_nt();
		}
		template<matching_input_iterator<T> IT>
		constexpr __dynamic_buffer(IT start, IT end, A const& alloc) :
			__allocator(alloc),
			__my_data(__allocator.allocate(static_cast<__size_type>(std::distance(start, end))), static_cast<__size_type>(std::distance(start, end)))
			{
				__size_type n = std::distance(start, end);
				__transfer(__beg(), start, end);
				__advance(n);
			}
		constexpr void __move_assign(__dynamic_buffer&& that)
		{
			this->__destroy();
			this->__my_data.__reset();
			this->__my_data.__swap_ptrs(that.__my_data);
		}
	};
	template<typename T, allocator_object<T> A, bool NT>
	template<matching_input_iterator<T> IT>
	constexpr void __dynamic_buffer<T, A, NT>::__transfer(T *where, IT start, IT end)
	{
		if constexpr(contiguous_iterator<IT>)
			array_copy(where, addressof(*start), static_cast<size_t>(distance(start, end))); 
		else for(IT i = start; i != end; i++, where++)
			*where = *i;
	}
	template<typename T, allocator_object<T> A, bool NT>
	constexpr void __dynamic_buffer<T, A, NT>::__zero(__ptr where, __size_type n)
	{
		if constexpr(!__trivial)
			for(__size_type i = 0; i < n; i++) 
				where[i].~T();
		else array_zero(where, n);
	}
	template<typename T, allocator_object<T> A, bool NT>
	constexpr typename __dynamic_buffer<T, A, NT>::__ptr __dynamic_buffer<T, A, NT>::__replace_elements(__size_type pos, __size_type count, __const_ptr from, __size_type count2)
	{
		if(count2 == count) __copy(__get_ptr(pos), from, count);
		else try
		{
			__diff_type diff 		= count2 - count;
			__size_type target_cap 	= __capacity() + diff;
			__size_type rem 		= __size() - (pos + count);
			__buf_ptrs nwdat(__allocator.allocate(target_cap), target_cap);
			__move(nwdat.__begin, __beg(), pos);
			__copy(nwdat.__get_ptr(pos), from, count2);
			__move(nwdat.__get_ptr(pos + count2), __get_ptr(pos + count), rem);
			nwdat.__setc(__size() + diff);
			__allocator.deallocate(__beg(), __capacity());
			__my_data.__copy_ptrs(nwdat);
			__post_modify_check_nt();
		}
		catch(...) { return nullptr; }
		return __get_ptr(pos + count);
	}
	template<typename T, allocator_object<T> A, bool NT>
	constexpr bool __dynamic_buffer<T, A, NT>::__grow_buffer(__size_type added)
	{
		if(!added) return true; // Zero elements -> vacuously true completion
		__size_type num_elements 	= __size();
		__size_type cur_capacity	= __capacity();
		__size_type target 			= min(max(cur_capacity << 1, cur_capacity + added), __max_capacity());
		try { __setn(resize(__beg(), cur_capacity, target, __allocator), num_elements, target); }
		catch(...) { return false; }
		if constexpr(__end_zero && __trivial) __zero(__cur(), added);
		return true;
	}
	template<typename T, allocator_object<T> A, bool NT>
	template<matching_input_iterator<T> IT> 
	constexpr typename __dynamic_buffer<T, A, NT>::__ptr __dynamic_buffer<T, A, NT>::__append_elements(IT start_it, IT end_it)
	{
		__size_type rem = __rem();
		__size_type num = distance(start_it, end_it);
		if((!__beg() || num > rem) && __unlikely(!__grow_buffer(num - rem))) return nullptr;
		for(IT i = start_it; i < end_it; i++, __advance(1UZ)) construct_at(__cur(), *i);
		__post_modify_check_nt();
		return __cur();
	}
	template<typename T, allocator_object<T> A, bool NT>
	template<matching_input_iterator<T> IT>
	constexpr typename __dynamic_buffer<T, A, NT>::__ptr __dynamic_buffer<T, A, NT>::__insert_elements(__const_ptr pos, IT start_ptr, IT end_ptr)
	{
		if(__unlikely(__out_of_range(pos))) return nullptr;
		__ptr ncpos = __get_ptr(__diff(pos));
		try
		{
			__size_type range_size 	= distance(start_ptr, end_ptr);
			__size_type offs 		= __diff(pos);
			bool prepending			= (pos < __cur());
			if(pos + range_size < __max())
			{
				if(prepending)
				{
					__size_type n 	= __ediff(pos);
					__ptr temp 		= __allocator.allocate(n);
					__move(temp, ncpos, n);
					__transfer(__get_ptr(offs), start_ptr, end_ptr);
					__move(__get_ptr(offs + range_size), temp, n);
					__allocator.deallocate(temp, n);
					__advance(range_size);
				}
				else {
					__transfer(__get_ptr(offs), start_ptr, end_ptr);
					__setc(offs + range_size);
				}
			}
			else 
			{
				__size_type target_cap = __capacity() + __needed(pos + range_size);
				__buf_ptrs nwdat(__allocator.allocate(target_cap), target_cap);
				if(prepending)
				{
					__size_type rem = __ediff(pos);
					__move(nwdat.__begin, __beg(), offs);
					__move(nwdat.__get_ptr(offs + range_size), ncpos, rem);
					nwdat.__setc(__size() + range_size);
				}
				else {
					__move(nwdat.__begin, __beg(), __size());
					nwdat.__setc(__size() + range_size);
				}
				__allocator.deallocate(__beg(), __size());
				__my_data.__copy_ptrs(nwdat);
				__transfer(__get_ptr(offs), start_ptr, end_ptr);
			}
			__post_modify_check_nt();
			return __get_ptr(offs);
		}
		catch(...) { return nullptr; }
	}
	template<typename T, allocator_object<T> A, bool NT>
	template<typename ... Args>
	requires(constructible_from<T, Args...>)
	constexpr typename __dynamic_buffer<T, A, NT>::__ptr __dynamic_buffer<T, A, NT>::__emplace_element(__const_ptr pos, Args&& ... args)
	{
		if(__unlikely(pos < this->__beg())) return nullptr;
		if(pos >= __max()) return __emplace_at_end(forward<Args>(args)...);
		__size_type start_pos 	= __diff(pos);
		__size_type rem 		= __ediff(pos);
		__size_type target_cap 	= (__size() < __capacity()) ? __capacity() : __capacity() + 1;
		__container nwdat(__allocator.allocate(target_cap), target_cap);
		__ptr result 			= construct_at(nwdat.__get_ptr(start_pos), forward<Args>(args)...);
		__move(nwdat.__begin, __beg(), start_pos);
		if(rem) __move(result + 1, __get_ptr(start_pos), rem);
		__destroy();
		__my_data.__swap_ptrs(nwdat);
		__post_modify_check_nt();
		return result;
	}
	template <typename T, allocator_object<T> A, bool NT>
	template <typename ... Args>
	requires(constructible_from<T, Args...>)
	constexpr typename __dynamic_buffer<T, A, NT>::__ptr __dynamic_buffer<T, A, NT>::__emplace_at_end(Args && ...args)
	{
		if(__size() == __capacity() && __unlikely(!__grow_buffer(1UZ))) return nullptr;
		__ptr p = construct_at(__cur(), forward<Args>(args)...);
		__bumpc(1L);
		__post_modify_check_nt();
		return p;
	}
	template<typename T, allocator_object<T> A, bool NT>
	constexpr typename __dynamic_buffer<T, A, NT>::__ptr __dynamic_buffer<T, A, NT>::__erase_range(__const_ptr start, __const_ptr end)
	{
		if(__unlikely(__out_of_range(start, end))) return nullptr;
		__size_type how_many 	= end - start;
		__size_type rem 		= __ediff(end);
		__size_type start_pos 	= __diff(start);
		if(!rem) return __erase_at_end(how_many);
		else try
		{
			__ptr temp 	= __allocator.allocate(rem);
			__ptr ncend = __get_ptr(__diff(end));
			__move(temp, ncend, rem);
			__zero(__get_ptr(start_pos), __ediff(start));
			__move(__get_ptr(start_pos), temp, rem);
			__allocator.deallocate(temp, rem);
			__setc(start_pos + rem);
		}
		catch(...) { return nullptr; }
		__post_modify_check_nt();
		return __get_ptr(start_pos);
	}
	extension template<typename T, allocator_object<T> A, bool NT> constexpr void __dynamic_buffer<T, A, NT>::__post_modify_check_nt() 
	{ 
		if constexpr(__end_zero) 
		{ 
			if(!(__max() > __cur()))
				__grow_buffer(1UZ); 
			construct_at(__cur()); 
		} 
	}
}
#endif