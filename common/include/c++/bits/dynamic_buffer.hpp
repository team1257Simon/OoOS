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
		typedef T* __pointer;
		typedef T const* __const_pointer;
		typedef decltype(sizeof(T)) __size_type;
		typedef decltype(declval<__pointer>() - declval<__pointer>()) __difference_type;
		__pointer __begin{};
		__pointer __end{};
		__size_type __cap{};
		constexpr __buf_ptrs() = default;
		constexpr __buf_ptrs(__pointer start, __pointer end, __size_type cap) noexcept : __begin(start), __end(end), __cap(cap) {}
		constexpr __buf_ptrs(__pointer start, __pointer end, __pointer max) noexcept : __begin(start), __end(end), __cap(static_cast<__size_type>(end - start)) {}
		constexpr void __reset() noexcept { __begin = __end = __pointer(); __cap = 0UZ; }
		constexpr __buf_ptrs(__buf_ptrs const& that) noexcept : __begin(that.__begin), __end(that.__end), __cap(that.__cap) {}
		constexpr __buf_ptrs(__buf_ptrs&& that) noexcept : __begin(that.__begin), __end(that.__end), __cap(that.__cap) { that.__reset(); }
		constexpr __buf_ptrs(__pointer start, __pointer end) : __begin(start), __end(end), __cap(static_cast<__size_type>(end - start)) {}
		constexpr __buf_ptrs(__pointer start, __size_type cap) : __begin(start), __end(start), __cap(cap) {}
		template<allocator_object<T> A> constexpr __buf_ptrs(A& alloc, __size_type cap) : __begin(alloc.allocate(cap)), __end(__begin), __cap(cap) {}
		constexpr void __copy_ptrs(__buf_ptrs const& that) { __begin = that.__begin; __end = that.__end; __cap = that.__cap; }
		constexpr void __set_ptrs(__pointer begin, __pointer end, __size_type ncap) { __begin = begin; __end = end; __cap = ncap; }
		constexpr void __set_ptrs(__pointer begin, __pointer end, __pointer max) { __begin = begin; __end = end; __cap = static_cast<__size_type>(max - begin); }
		constexpr void __set_ptrs(__pointer begin, __size_type ncap) { __set_ptrs(begin, begin + min(ncap, static_cast<__size_type>(__end - __begin)), ncap); }
		constexpr __pointer __get_ptr(__size_type offs) { return __begin + offs; }
		constexpr void __setc(__pointer where) { __end = where; }
		constexpr void __setc(__size_type offs) { __end = __begin + offs; }
		constexpr void __adv(__size_type n) { __end += n; }
		constexpr void __bck(__size_type n) { __end -= n; }
		constexpr __size_type __capacity() const { return __cap; }
		constexpr __pointer __beg() const { return __begin; }
		constexpr __pointer __cur() const { return __end; }
		constexpr __pointer __max() const { return __begin + __cap; }
		constexpr void __bounds(__pointer e, __size_type c) noexcept { __end = e; __cap = c; }
		constexpr void __swap_ptrs(__buf_ptrs& that)
		{
			__buf_ptrs tmp(this->__begin, this->__end, this->__cap);
			this->__copy_ptrs(that);
			that.__copy_ptrs(tmp);
		}
		template<allocator_object<T> A>
		constexpr void __destroy(A& alloc)
		{
			if(__unlikely(!__begin)) return;
			if constexpr(!std::is_trivially_destructible_v<T>)
				for(__pointer p = __begin; p < __end; p++)
					p->~T();
			alloc.deallocate(__begin, __cap);
			__reset();
		}
		template<allocator_object<T> A>
		constexpr void __create(A& alloc, __size_type cap)
		{
			if(__unlikely(__begin != nullptr)) this->__destroy(alloc);
			__begin	= alloc.allocate(cap);
			__end	= __begin;
			__cap	= cap;
		}
		template<allocator_object<T> A>
		constexpr void __resize(A& alloc, __size_type ncur, __size_type ncap)
		{
			if constexpr(__has_resize<A, T>)
				__begin		= alloc.resize(__begin, ncur, ncap);
			else __begin	= resize(__begin, __cap, ncap, alloc);
			__cap			= ncap;
			if(__unlikely(ncur > __cap))
				ncur		= __cap;
			__end			= __begin + ncur;
		}
	};
	template<typename C, typename A>
	struct __buffer_container_impl : A, C
	{
		using typename C::__size_type;
		constexpr __buffer_container_impl() noexcept(noexcept(A())) = default;
		constexpr __buffer_container_impl(A const& that) noexcept(std::is_nothrow_copy_constructible_v<A>) : A(that), C() {}
		constexpr __buffer_container_impl(A const& that, __size_type s) : A(that), C(*static_cast<A*>(this), s) {}
		constexpr __buffer_container_impl(__buffer_container_impl&&) noexcept(std::is_nothrow_move_constructible_v<A>) = default;
		constexpr __buffer_container_impl(__buffer_container_impl const&) noexcept(std::is_nothrow_copy_constructible_v<A>) = default;
		constexpr void __create(__size_type cap) { C::__create(*this, cap); }
		constexpr void __destroy() { C::__destroy(*this); }
		constexpr void __resize(__size_type ncur, __size_type ncap) { C::__resize(*this, ncur, ncap); }
		constexpr __buffer_container_impl& operator=(__buffer_container_impl const& that)
		{
			this->__destroy();
			this->__copy_ptrs(that);
			if constexpr(__has_move_propagate<A>)
				*static_cast<A*>(this)	= that;
			return *this;
		}
		constexpr __buffer_container_impl& operator=(__buffer_container_impl&& that)
		{
			this->__destroy();
			this->__swap_ptrs(that);
			if constexpr(__has_move_propagate<A>)
				*static_cast<A*>(this)	= std::move(that);
			return *this;
		}
	};
	/**
	 * This base-type implements the functionality shared by the dynamic-container types (mainly string and vector). 
	 * If the buffer is for a null-terminated string, use NTS = true. This parameter exists in place of using a virtual member to implement this functionality.
	 * Apparently, using a virtual member in something like std::string can cause problems...who knew.
	 */
	template<typename T, allocator_object<T> A, bool NTS>
	struct __dynamic_buffer
	{
		typedef __buffer_container_impl<__buf_ptrs<T>, A> __container;
		typedef typename __buf_ptrs<T>::__pointer __pointer;
		typedef typename __buf_ptrs<T>::__const_pointer __const_pointer;
		typedef typename __buf_ptrs<T>::__size_type __size_type;
		typedef typename __buf_ptrs<T>::__difference_type __difference_type;
		typedef deref_t<__pointer> __reference;
		typedef deref_t<__const_pointer> __const_reference;
		typedef A __allocator_type;
		constexpr static bool __end_zero 	= NTS;
		constexpr static bool __trivial 	= trivial_copy<T>;
		__container __my_data;
		constexpr static __const_pointer __pmin(__const_pointer a, __const_pointer b) { return a < b ? a : b; }
		/**
		 * Copies data using iterators that might not be contiguous; i.e. not linear (e.g. tree iterators) or not forward-facing (e.g. reverse iterators).
		 * If the iterator happens to be contiguous (e.g. a pointer), this will simplify into the normal copy operation.
		 */
		template<matching_input_iterator<T> IT> constexpr void __transfer(T* where, IT start, IT end);
		constexpr void __set(__pointer where, T const& val, __size_type n) { fill_n(where, n, val); }
		/**
		 * Selects the proper function to clear allocated storage of garbage data.
		 * If the type T is a nontrivial type which cannot be default-constructed, this function does nothing.
		 * If the type is nontrivial and can be default-constructed, it default-constructs each element of the destination array.
		 * Otherwise, it will behave similarly to using memset to zero the memory.
		 */
		constexpr void __zero(__pointer where, __size_type n);
		constexpr void __copy(__pointer where, __const_pointer src, __size_type n) { array_copy(where, src, n); }
		constexpr void __move(__pointer where, __pointer src, __size_type n) { array_move(where, src, n); }
		constexpr bool __grow_buffer(__size_type added);
		constexpr void __post_modify_check_nt();
		template<matching_input_iterator<T> IT> constexpr __pointer __append_elements(IT start_it, IT end_it);
		template<matching_input_iterator<T> IT> constexpr __pointer __insert_elements(__const_pointer pos, IT start_ptr, IT end_ptr);
		template<typename ... Args> requires constructible_from<T, Args...> constexpr __pointer __emplace_element(__const_pointer pos, Args&& ... args);
		template<typename ... Args> requires constructible_from<T, Args...> constexpr __pointer __emplace_at_end(Args&& ... args);
		constexpr __pointer __replace_elements(__size_type pos, __size_type count, __const_pointer from, __size_type count2);
		constexpr __pointer __erase_range(__const_pointer start, __const_pointer end);
		constexpr __pointer __insert_element(__const_pointer pos, T const& t) { return __insert_elements(pos, addressof(t), addressof(t) + 1); }
		constexpr __pointer __beg() noexcept { return __my_data.__beg(); }
		constexpr __const_pointer __beg() const noexcept { return __my_data.__beg(); }
		constexpr __pointer __cur() noexcept { return __my_data.__cur(); }
		constexpr __const_pointer __cur() const noexcept { return __my_data.__cur(); }
		constexpr __pointer __max() noexcept { return __my_data.__max(); }
		constexpr __const_pointer __max() const noexcept { return __my_data.__max(); }
		constexpr __size_type __diff(__const_pointer pos) const noexcept { return __size_type(pos - __beg()); }
		constexpr __difference_type __needed(__const_pointer pos) const noexcept { return pos - __max(); }
		constexpr bool __valid_end_pos(__const_pointer pos) const noexcept { return pos >= __beg() && pos <= __max(); }
		constexpr bool __out_of_range(__const_pointer pos) const noexcept { return pos < __beg() || pos >= __max(); }
		constexpr bool __out_of_range(__const_pointer start, __const_pointer end) const noexcept { return __out_of_range(start) || __out_of_range(end) || end < start; }
		constexpr __pointer __get_ptr(__size_type __i) noexcept { return __beg() + __i; }
		constexpr __const_pointer __get_ptr(__size_type __i) const noexcept { return __beg() + __i; }
		constexpr __reference __get(__size_type __i) { return *__get_ptr(__i); }
		constexpr __const_reference __get(__size_type __i) const { return *__get_ptr(__i); }
		constexpr __reference __get_last() noexcept { return *__get_ptr(__size() - 1); }
		constexpr __const_reference __get_last() const noexcept { return *__get_ptr(__size() - 1); }
		constexpr void __setp(__buf_ptrs<T> const& ptrs) noexcept { __my_data.__copy_ptrs(ptrs); }
		constexpr void __setp(__pointer beg, __pointer cur, __pointer end) noexcept { __my_data.__set_ptrs(beg, cur, end); }
		constexpr void __setp(__pointer beg, __pointer end) noexcept { __setp(beg, beg, end); }
		constexpr void __setn(__pointer beg, __size_type c, __size_type n) noexcept { __setp(beg, beg + c, beg + n); }
		constexpr void __setn(__pointer beg, __size_type n) noexcept { __setp(beg, beg + n); }
		constexpr void __setc(__pointer pos) noexcept { if(__valid_end_pos(pos)) __my_data.__setc(pos); } 
		constexpr void __setc(__size_type pos) noexcept { __setc(__get_ptr(pos)); }
		constexpr void __bumpc(int64_t off) noexcept { __setc(__cur() + off); }
		constexpr void __rst() noexcept { __setc(__beg()); }
		constexpr __size_type __max_capacity() const noexcept { return numeric_limits<__size_type>::max(); }
		constexpr void __advance(__size_type n) { if(__valid_end_pos(__cur() + n)) { __my_data.__adv(n); } else { __setc(__max()); } }
		constexpr void __backtrack(__size_type n) { if(__valid_end_pos(__cur() - n)) { __my_data.__bck(n); } else { __setc(__beg()); } }
		constexpr __size_type __size() const noexcept { return static_cast<__size_type>(__cur() - __beg()); }
		constexpr __size_type __capacity() const noexcept { return static_cast<__size_type>(__max() - __beg()); }
		constexpr __size_type __rem() const noexcept { return static_cast<__size_type>(__max() - __cur()); }
		constexpr __size_type __ediff(__const_pointer pos) const noexcept { return static_cast<__size_type>(__cur() - pos); }
		constexpr __allocator_type const& __get_alloc() const noexcept { return __my_data; }
		constexpr void __allocate_storage(__size_type n) { __my_data.__create(n); }
		constexpr __pointer __create_temp(__size_type n) { return __my_data.allocate(n); }
		constexpr void __destroy_temp(__pointer tmp, __size_type n) { __my_data.deallocate(tmp, n); }
		constexpr void __data_resize(__size_type ncur, __size_type ncap) { __my_data.__resize(ncur, ncap); }
		constexpr __container __temp_container(__size_type cap) { return __container(__my_data, cap); }
		constexpr explicit __dynamic_buffer(A const& alloc) noexcept(noexcept(A(alloc))) : __my_data(alloc) {}
		constexpr __dynamic_buffer() noexcept(noexcept(A())) : __my_data() {}
		constexpr __dynamic_buffer(__size_type sz) : __my_data(A(), sz) {}
		constexpr __dynamic_buffer(__size_type sz, A const& alloc) : __my_data(alloc, sz) {}
		constexpr __dynamic_buffer(__size_type sz, T const& val, A const& alloc) : __my_data(alloc, sz) { if(sz) { __set(__beg(), val, sz); __advance(sz); } }
		constexpr __dynamic_buffer(initializer_list<T> const& __ils, A const& alloc) : __dynamic_buffer(__ils.begin(), __ils.end(), alloc) {}
		constexpr __dynamic_buffer(__dynamic_buffer const& that) : __dynamic_buffer(that.__beg(), that.__cur(), that.__get_alloc()) {}
		constexpr __dynamic_buffer(__dynamic_buffer const& that, A const& alloc) : __dynamic_buffer(that.__beg(), that.__cur(), alloc) {}
		constexpr __dynamic_buffer(__dynamic_buffer const& that, __size_type start, A const& alloc) : __dynamic_buffer(that.__get_ptr(start), that.__cur(), alloc) {}
		constexpr __dynamic_buffer(__dynamic_buffer const& that, __size_type start, __size_type count, A const& alloc) : __dynamic_buffer(that.__get_ptr(start), __pmin(that.__get_ptr(start + count), that.__cur()), alloc) {}
		constexpr __dynamic_buffer(__dynamic_buffer&& that) noexcept(noexcept(__container(move(that.__my_data)))) : __my_data(move(that.__my_data)) {}
		constexpr  __dynamic_buffer(__dynamic_buffer&& that, A const& alloc) noexcept(noexcept(__container(alloc))) : __my_data(move(that.__my_data)) {}
		constexpr ~__dynamic_buffer() { this->__destroy(); }
		constexpr __dynamic_buffer& operator=(__dynamic_buffer const& that) { this->__copy_assign(that); return *this; }
		constexpr __dynamic_buffer& operator=(__dynamic_buffer&& that) { this->__move_assign(move(that)); return *this; }
		constexpr __pointer __assign_elements(initializer_list<T> ini) { return this->__assign_elements(ini.begin(), ini.end()); }
		constexpr __pointer __erase(__const_pointer pos) { return this->__erase_range(pos, pos + 1); }
		constexpr void __assign_ptrs(__container const& c) noexcept { __my_data.__copy_ptrs(c); }
		constexpr void __move_assign(__dynamic_buffer&& that) { this->__my_data	= std::move(that.__my_data); }
		template<matching_input_iterator<T> IT>
		constexpr __dynamic_buffer(IT start, IT end, A const& alloc) :
			__my_data(alloc, static_cast<__size_type>(std::distance(start, end < start ? start : end)))
			{
				__size_type n	= __my_data.__capacity();
				__transfer(__beg(), start, end < start ? start : end);
				__advance(n);
			}
		constexpr void __trim_buffer()
		{
			__size_type num_elements	= __size();
			__data_resize(num_elements, num_elements);
			__post_modify_check_nt();
		}
		constexpr void __size_buffer(__size_type n)
		{
			__size_type num_elements	= __size();
			__data_resize(std::min(n, num_elements), n);
			__post_modify_check_nt();
		}
		constexpr void __construct_element(__pointer pos, T const& t)
		{
			if(__unlikely(__out_of_range(pos))) return;
			construct_at(pos, t);
			if(pos > __cur()) __setc(pos);
		}
		constexpr __pointer __assign_elements(__size_type count, T const& t)
		{
			if(count > __capacity() && __unlikely(!__grow_buffer(count - __capacity()))) return nullptr;
			__set(__beg(), t, count);
			if(count < __size()) __zero(__get_ptr(count), __size() - count);
			__setc(count);
			__post_modify_check_nt();
			return __cur();
		}
		constexpr __pointer __assign_elements(__const_pointer start, __const_pointer end)
		{
			__size_type count	= end - start;
			if(count > __capacity() && __unlikely(!__grow_buffer(count - __capacity()))) return nullptr;
			__copy(__beg(), start, count);
			if(count < __size()) __zero(__get_ptr(count), __size() - count);
			__setc(count);
			__post_modify_check_nt();
			return __cur();
		}
		constexpr __pointer __replace_elements(__const_pointer start, __const_pointer end, __const_pointer from, __size_type count)
		{
			if(__unlikely(__out_of_range(start, end)))
				return nullptr;
			return __replace_elements(start - __beg(), end - start, from, count);
		}
		constexpr __pointer __append_elements(__size_type count, T const& t)
		{
			if(!(__max() > __cur() + count) && __unlikely(!__grow_buffer(static_cast<__size_type>(count - __rem())))) return nullptr;
			for(__size_type i = 0; i < count; i++, __advance(1UZ)) construct_at(__cur(), t);
			__post_modify_check_nt();
			return __cur();
		}
		constexpr __pointer __append_element(T const& t)
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
		constexpr __pointer __erase_at_end(__size_type how_many)
		{
			if(__unlikely(how_many >= __size())) __clear();
			else { __backtrack(how_many); __zero(__cur(), how_many); }
			__post_modify_check_nt();
			return __cur();
		}
		constexpr void __destroy()
		{
			if(__unlikely(!this->__beg()))
				return;
			__my_data.__destroy();
		}
		constexpr void __copy_assign(__dynamic_buffer const& that)
		{
			__destroy();
			if(__unlikely(!that.__beg())) return;
			if constexpr(__has_move_propagate<__allocator_type>) this->__my_data = that.__get_alloc();
			__allocate_storage(that.__capacity());
			__copy(__beg(), that.__beg(), that.__capacity());
			__advance(that.__size());
			__post_modify_check_nt();
		}
		constexpr void __swap(__dynamic_buffer& that)
		{
			this->__my_data.__swap_ptrs(that.__my_data);
			if constexpr(__has_move_propagate<__allocator_type>)
				std::swap<__allocator_type>(this->__my_data, that.__my_data);
			this->__post_modify_check_nt();
			that.__post_modify_check_nt();
		}
	};
	template<typename T, allocator_object<T> A, bool NTS>
	template<matching_input_iterator<T> IT>
	constexpr void __dynamic_buffer<T, A, NTS>::__transfer(T* where, IT start, IT end)
	{
		if constexpr(contiguous_iterator<IT>)
			array_copy(where, addressof(*start), static_cast<size_t>(distance(start, end))); 
		else for(IT i = start; i != end; i++, where++)
			*where = *i;
	}
	template<typename T, allocator_object<T> A, bool NTS>
	constexpr void __dynamic_buffer<T, A, NTS>::__zero(__pointer where, __size_type n)
	{
		if constexpr(!__trivial)
			for(__size_type i = 0; i < n; i++) 
				where[i].~T();
		else array_zero(where, n);
	}
	template<typename T, allocator_object<T> A, bool NTS>
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer __dynamic_buffer<T, A, NTS>::__replace_elements(__size_type pos, __size_type count, __const_pointer from, __size_type count2)
	{
		if(count2 == count) __copy(__get_ptr(pos), from, count);
		else try
		{
			__difference_type diff 	= count2 - count;
			__size_type target_cap 	= __capacity() + diff;
			__size_type rem 		= __size() - (pos + count);
			__container nwdat		= __temp_container(target_cap);
			__move(nwdat.__begin, __beg(), pos);
			__copy(nwdat.__get_ptr(pos), from, count2);
			__move(nwdat.__get_ptr(pos + count2), __get_ptr(pos + count), rem);
			nwdat.__setc(__size() + diff);
			__destroy();
			__assign_ptrs(nwdat);
			__post_modify_check_nt();
		}
		catch(...) { return nullptr; }
		return __get_ptr(pos + count);
	}
	template<typename T, allocator_object<T> A, bool NTS>
	constexpr bool __dynamic_buffer<T, A, NTS>::__grow_buffer(__size_type added)
	{
		if(!added) return true; // Zero elements -> vacuously true completion
		__size_type num_elements 	= __size();
		__size_type cur_capacity	= __capacity();
		__size_type target 			= min(max(cur_capacity << 1, cur_capacity + added), __max_capacity());
		try { __data_resize(num_elements, target); }
		catch(...) { return false; }
		if(__unlikely(!__cur())) return false;
		if constexpr(__end_zero && __trivial) __zero(__cur(), added);
		return true;
	}
	template<typename T, allocator_object<T> A, bool NTS>
	template<matching_input_iterator<T> IT> 
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer __dynamic_buffer<T, A, NTS>::__append_elements(IT start_it, IT end_it)
	{
		__size_type rem	= __rem();
		__size_type num	= distance(start_it, end_it);
		if((!__beg() || num > rem) && __unlikely(!__grow_buffer(num - rem))) return nullptr;
		for(IT i = start_it; i < end_it; i++, __advance(1UZ)) construct_at(__cur(), *i);
		__post_modify_check_nt();
		return __cur();
	}
	template<typename T, allocator_object<T> A, bool NTS>
	template<matching_input_iterator<T> IT>
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer __dynamic_buffer<T, A, NTS>::__insert_elements(__const_pointer pos, IT start_ptr, IT end_ptr)
	{
		if(__unlikely(__out_of_range(pos))) return nullptr;
		__pointer ncpos				= __get_ptr(__diff(pos));
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
					__pointer temp 	= __create_temp(n);
					__move(temp, ncpos, n);
					__transfer(__get_ptr(offs), start_ptr, end_ptr);
					__move(__get_ptr(offs + range_size), temp, n);
					__destroy_temp(temp, n);
					__advance(range_size);
				}
				else
				{
					__pointer target	= __get_ptr(offs);
					__transfer(target, start_ptr, end_ptr);
					__setc(offs + range_size);
				}
			}
			else 
			{
				__size_type target_cap	= __capacity() + __needed(pos + range_size);
				__container nwdat		= __temp_container(target_cap);
				__size_type osz			= __size();
				if(prepending)
				{
					__size_type rem		= __ediff(pos);
					__move(nwdat.__begin, __beg(), offs);
					__move(nwdat.__get_ptr(offs + range_size), ncpos, rem);
				}
				else __move(nwdat.__begin, __beg(), osz);
				__destroy();
				__assign_ptrs(nwdat);
				__setc(osz + range_size);
				__transfer(__get_ptr(offs), start_ptr, end_ptr);
			}
			__post_modify_check_nt();
			return __get_ptr(offs);
		}
		catch(...) { return nullptr; }
	}
	template<typename T, allocator_object<T> A, bool NTS>
	template<typename ... Args>
	requires(constructible_from<T, Args...>)
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer __dynamic_buffer<T, A, NTS>::__emplace_element(__const_pointer pos, Args&& ... args)
	{
		if(__unlikely(pos < this->__beg())) return nullptr;
		if(pos >= __max()) return __emplace_at_end(forward<Args>(args)...);
		__size_type start_pos 	= __diff(pos);
		__size_type rem 		= __ediff(pos);
		__size_type target_cap 	= (__size() < __capacity()) ? __capacity() : __capacity() + 1;
		__container nwdat		= __temp_container(target_cap);
		__pointer result 		= construct_at(nwdat.__get_ptr(start_pos), forward<Args>(args)...);
		__move(nwdat.__begin, __beg(), start_pos);
		if(rem) __move(result + 1, __get_ptr(start_pos), rem);
		__destroy();
		__assign_ptrs(nwdat);
		__post_modify_check_nt();
		return result;
	}
	template <typename T, allocator_object<T> A, bool NTS>
	template <typename ... Args>
	requires(constructible_from<T, Args...>)
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer __dynamic_buffer<T, A, NTS>::__emplace_at_end(Args && ...args)
	{
		if(__size() == __capacity() && __unlikely(!__grow_buffer(1UZ))) return nullptr;
		__pointer p = construct_at(__cur(), forward<Args>(args)...);
		__bumpc(1L);
		__post_modify_check_nt();
		return p;
	}
	template<typename T, allocator_object<T> A, bool NTS>
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer __dynamic_buffer<T, A, NTS>::__erase_range(__const_pointer start, __const_pointer end)
	{
		if(__unlikely(__out_of_range(start, end))) return nullptr;
		__size_type how_many 	= end - start;
		__size_type rem 		= __ediff(end);
		__size_type start_pos 	= __diff(start);
		if(!rem) return __erase_at_end(how_many);
		else try
		{
			__pointer temp 	= __create_temp(rem);
			__pointer ncend = __get_ptr(__diff(end));
			__move(temp, ncend, rem);
			__zero(__get_ptr(start_pos), __ediff(start));
			__move(__get_ptr(start_pos), temp, rem);
			__destroy_temp(temp, rem);
			__setc(start_pos + rem);
		}
		catch(...) { return nullptr; }
		__post_modify_check_nt();
		return __get_ptr(start_pos);
	}
	template<typename T, allocator_object<T> A, bool NTS> 
	constexpr void __dynamic_buffer<T, A, NTS>::__post_modify_check_nt() 
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