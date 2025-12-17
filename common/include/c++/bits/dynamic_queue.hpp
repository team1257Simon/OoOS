#ifndef __DYN_QUEUE
#define __DYN_QUEUE
#include <memory>
#include <limits>
#include <bits/stl_iterator.hpp>
#include <libk_decls.h>
#include <initializer_list>
#include <bits/stdexcept.h>
#include <bits/stl_algobase.hpp>
namespace std::__impl
{
	template<typename T>
	struct __queue_ptrs
	{
		typedef T* __pointer;
		typedef T const* __const_pointer;
		typedef decltype(sizeof(T)) __size_type;
		typedef decltype(declval<__pointer>() - declval<__pointer>()) __difference_type;
		__pointer __begin{};	// Points to the start of allocated storage. Extracted elements remain between here and __next until the buffer is trimmed for stale elements.
		__pointer __next{};		// Points to one past the last element EXTRACTED FROM the queue.
		__pointer __end{};		// Points to one past the last element INSERTED INTO the queue.
		__pointer __qmax{};		// Points to the end of allocated storage.
		void __reset() noexcept { __begin = __next = __end = __qmax = __pointer(); }
		constexpr __queue_ptrs() noexcept = default;
		constexpr __queue_ptrs(__pointer beg, __pointer mx) noexcept : __begin(beg), __next(beg), __end(beg), __qmax(mx) {}
		constexpr __queue_ptrs(__pointer beg, __pointer nx, __pointer ed, __pointer mx) noexcept : __begin(beg), __next(nx), __end(ed), __qmax(mx) {}
		constexpr __queue_ptrs(__queue_ptrs const& that) noexcept : __begin(that.__begin), __next(that.__next), __end(that.__end), __qmax(that.__qmax) {}
		constexpr __queue_ptrs(__queue_ptrs&& that) noexcept : __begin(that.__begin), __next(that.__next), __end(that.__end), __qmax(that.__qmax) { that.__reset(); }
		constexpr __queue_ptrs(__pointer beg, __size_type sz) noexcept : __queue_ptrs(beg, beg + sz) {}
		constexpr __pointer __q_get_ptr(__size_type offs) noexcept { return __begin + offs; }
		constexpr void __bumpc(__difference_type n = __difference_type(1L)) noexcept { __end += n; }
		constexpr void __bumpn(__difference_type n = __difference_type(1L)) noexcept { __next += n; }
		constexpr void __setc(__pointer where) noexcept { __end = where; }
		constexpr void __setc(__size_type offs) noexcept { __end = __begin + offs; }
		constexpr void __setn(__pointer where) noexcept { __next = where; }
		constexpr void __setn(__size_type offs) noexcept { __next = __begin + offs; }
		constexpr void __set_ptrs(__pointer beg, __pointer nx, __pointer ed, __pointer mx) noexcept { __begin = beg; __next = nx; __end = ed; __qmax = mx; }
		constexpr void __set_ptrs(__pointer beg, __size_type sz) noexcept { __set_ptrs(beg, beg, beg, beg + sz); }
		constexpr void __set_ptrs(__pointer beg, __size_type nx, __size_type ed, __size_type sz) noexcept { __set_ptrs(beg, beg + nx, beg + ed, beg + sz); }
		constexpr void __copy_ptrs(__queue_ptrs const& that) noexcept { __begin = that.__begin; __next = that.__next; __end = that.__end; __qmax = that.__qmax; }
		constexpr void __move(__queue_ptrs&& that) noexcept { __copy_ptrs(that); that.__reset(); }
		constexpr void __swap(__queue_ptrs& that) { __queue_ptrs tmp{}; tmp.__copy_ptrs(*this); __copy_ptrs(that); that.__copy_ptrs(tmp); }
	};
	template<typename T, allocator_object<T> A>
	struct __dynamic_queue
	{
		typedef T __value_type;
		typedef A __allocator_type;
		typedef __queue_ptrs<__value_type> __ptr_container;
		typedef typename __ptr_container::__pointer __pointer;
		typedef typename __ptr_container::__const_pointer __const_pointer;
		typedef typename __ptr_container::__size_type __size_type;
		typedef typename __ptr_container::__difference_type __difference_type;
		typedef deref_t<__pointer> __reference;
		typedef deref_t<__const_pointer> __const_reference;
		struct __q_alloc_and_state : __allocator_type
		{
			__difference_type __stale_size_thresh;
			unsigned int __stale_op_thresh;
			unsigned int __op_cnt;
			typedef std::bool_constant<__has_move_propagate<__allocator_type>> propagate_on_container_move_assignment;
			typedef std::bool_constant<__has_copy_propagate<__allocator_type>> propagate_on_container_copy_assignment;
			typedef std::bool_constant<__has_swap_propagate<__allocator_type>> propagate_on_container_swap;
			constexpr static bool __nt_copy_assign	= !propagate_on_container_copy_assignment::value || std::is_nothrow_copy_assignable_v<__allocator_type>;
			constexpr static bool __nt_move_assign	= !propagate_on_container_move_assignment::value || std::is_nothrow_move_assignable_v<__allocator_type>;
			constexpr __q_alloc_and_state() noexcept(noexcept(__allocator_type())) : __allocator_type(), __stale_size_thresh(16L), __stale_op_thresh(3U), __op_cnt(0U) {}
			constexpr __q_alloc_and_state(__allocator_type const& that) noexcept(std::is_nothrow_copy_constructible_v<__allocator_type>) : __allocator_type(that), __stale_size_thresh(16L), __stale_op_thresh(3U), __op_cnt(0U) {}
			constexpr __q_alloc_and_state(__allocator_type&& that) noexcept(std::is_nothrow_move_constructible_v<__allocator_type>) : __allocator_type(move(that)), __stale_size_thresh(16L), __stale_op_thresh(3U), __op_cnt(0U) {}
			constexpr __q_alloc_and_state& operator=(__q_alloc_and_state const& that) noexcept(__nt_copy_assign)
			{
				this->__stale_size_thresh	= that.__stale_size_thresh;
				this->__stale_op_thresh		= that.__stale_op_thresh;
				this->__op_cnt				= that.__op_cnt;
				if constexpr(propagate_on_container_copy_assignment::value)
					*static_cast<__allocator_type*>(this)	= that;
				return *this;
			}
			constexpr __q_alloc_and_state& operator=(__q_alloc_and_state&& that) noexcept(__nt_move_assign)
			{
				this->__stale_size_thresh	= that.__stale_size_thresh;
				this->__stale_op_thresh		= that.__stale_op_thresh;
				this->__op_cnt				= that.__op_cnt;
				that.__op_cnt				= 0U;
				if constexpr(propagate_on_container_move_assignment::value)
					*static_cast<__allocator_type*>(this)	= std::move(that);
				return *this;
			}
		} __qallocator;
		__ptr_container __my_queue_data;
		template<matching_input_iterator<T> IT> constexpr void __qtransfer(__pointer where, IT start, IT end) { if constexpr(contiguous_iterator<IT>) array_copy(where, addressof(*start), static_cast<__size_type>(distance(start, end))); else for(IT i = start; i != end; i++, where++) { *where = *i; } }
		constexpr void __qset(__pointer where, __value_type const& val, __size_type n) { fill_n(where, n, val); }
		constexpr void __qassign(__pointer where, __value_type&& val) { *where = move(val); }
		constexpr void __qcopy(__pointer where, __const_pointer src, __size_type n) { array_copy(where, src, n); }
		constexpr void __qzero(__pointer where, __size_type n)
		{
			if constexpr(!std::is_trivially_destructible_v<T>)
				for(__size_type i{}; i < n; i++)
					where[i].~T();
			array_zero(where, n);
		}
		/**
		 * Called whenever elements are pushed to or popped from the queue.
		 * As with dynamic_buffer, the setn/sete/bumpn/bumpe functions do not call this function.
		 * Inheritors can override to add functionality that needs to be invoked whenever the set's elements are modified, such as trimming stale elements.
		 * The default implementation does nothing.
		 */
		extension virtual void on_modify_queue() {}
		constexpr void __qassign(__pointer where, __value_type&& val, __size_type n) { for(__size_type i = 0; i < n; i++, ++where) { __qassign(where, move(val)); } }
		constexpr __pointer __qbeg() noexcept { return __my_queue_data.__begin; }
		constexpr __const_pointer __qbeg() const noexcept { return __my_queue_data.__begin; }
		constexpr __pointer __qcur() noexcept { return __my_queue_data.__next; }
		constexpr __const_pointer __qcur() const noexcept { return __my_queue_data.__next; }
		constexpr __pointer __end() noexcept { return __my_queue_data.__end; }
		constexpr __const_pointer __end() const noexcept { return __my_queue_data.__end; }
		constexpr __pointer __qmax() noexcept { return __my_queue_data.__qmax; }
		constexpr __const_pointer __qmax() const noexcept { return __my_queue_data.__qmax; }
		constexpr __pointer __q_get_ptr(__size_type __i) noexcept { return __qbeg() + __i; }
		constexpr __const_pointer __q_get_ptr(__size_type __i) const noexcept { return __qbeg() + __i; }
		constexpr bool __q_out_of_range(__const_pointer pos) const noexcept { return pos < __qbeg() || pos >= __qmax(); }
		constexpr bool __q_out_of_range(__const_pointer start, __const_pointer end) const noexcept { return __q_out_of_range(start) || __q_out_of_range(end) || end < start; }
		constexpr void __bumpn(__difference_type n = 1Z) noexcept { __my_queue_data.__bumpn(n); }
		constexpr void __bumpe(__difference_type n = 1Z) noexcept { __my_queue_data.__bumpc(n); }
		constexpr void __qsetn(__pointer where) noexcept { if(!__q_out_of_range(where) && where <= __end()) __my_queue_data.__setn(where); }
		constexpr void __qsetn(__size_type n) noexcept { __qsetn(__q_get_ptr(n)); }
		constexpr void __qsete(__pointer where) noexcept { if(!__q_out_of_range(where)) __my_queue_data.__setc(where); }
		constexpr void __qsete(__size_type n) noexcept { __qsete(__q_get_ptr(n)); }
		constexpr void __qrst() noexcept { __qsetn(__qbeg()); /* Not an EKG though */ }
		constexpr __reference __qget(__size_type __i) { return *__q_get_ptr(__i); }
		constexpr __const_reference __qget(__size_type __i) const { return *__q_get_ptr(__i); }
		constexpr __pointer __peek_next() noexcept { return __qcur(); }
		constexpr __const_pointer __peek_next() const noexcept { return __qcur(); }
		constexpr bool __have_next() const noexcept { return __qcur() < __end(); }
		constexpr __pointer __pop_next() noexcept { if(__qcur() < __end()) { __pointer result = __qcur(); __bumpn(); return result; } return nullptr; }
		constexpr __pointer __unpop() noexcept { if(__qcur() > __qbeg()) { __bumpn(-1Z); __qallocator.__op_cnt = 0U; return __qcur(); } return nullptr; }
		constexpr void __qsetp(__ptr_container const& ptrs) noexcept { __my_queue_data.__copy_ptrs(ptrs); }
		constexpr __size_type __q_max_capacity() const noexcept { return numeric_limits<__size_type>::max(); }
		constexpr __size_type __qsize() const noexcept { return static_cast<__size_type>(__end() - __qbeg()); }
		constexpr __size_type __qrem() const noexcept { return static_cast<__size_type>(__end() - __qcur()); }
		constexpr __size_type __stell() const noexcept { return static_cast<__size_type>(__qcur() - __qbeg()); }
		constexpr __difference_type __tell() const noexcept { return __qcur() - __qbeg(); }
		constexpr __size_type __qcapacity() const noexcept { return static_cast<__size_type>(__qmax() - __qbeg()); }
		constexpr __size_type __cap_rem() const noexcept { return static_cast<__size_type>(__qmax() - __end()); }
		constexpr void __q_move_assign(__dynamic_queue&& that) noexcept { __my_queue_data.__move(move(that.__my_queue_data)); __q_state_assign(move(that)); }
		constexpr void __q_copy_assign(__dynamic_queue const& that) noexcept { __my_queue_data.__set_ptrs(__qallocator.allocate(that.__qcapacity()), that.__qcapacity()); __qcopy(__qbeg(), that.__qbeg(), that.__qcapacity()); __q_state_assign(that); }
		constexpr __size_type __erase_before_next();
		constexpr void __set_stale_op_threshold(unsigned int value) noexcept { __qallocator.__stale_op_thresh			= value; }
		constexpr void __set_stale_size_threshold(__difference_type value) noexcept { __qallocator.__stale_size_thresh	= value; }
		constexpr void __q_state_assign(__dynamic_queue const& that) noexcept { this->__qallocator						= that.__qallocator; }
		constexpr void __q_state_assign(__dynamic_queue&& that) noexcept { this->__qallocator							= move(that.__qallocator); }
		constexpr bool __is_stale() const noexcept { return __qallocator.__op_cnt > __qallocator.__stale_op_thresh && (__my_queue_data.__next - __my_queue_data.__begin) > __qallocator.__stale_size_thresh; }
		constexpr void __qdestroy()
		{
			if(__unlikely(!__qbeg())) return;
			if constexpr(!std::is_trivially_destructible_v<T>)
				for(__pointer p = __qbeg(); p < __end(); p++)
					p->~T();
			__qallocator.deallocate(__qbeg(), __qcapacity());
			__my_queue_data.__reset();
			__qallocator.__op_cnt = 0;
		}
		/**
		 * If there have been a number of push operations exceeding the configured operation threshold since the most recent pop or trim,
		 * and the number of elements between the base and current pointers exceeds the configured size threshold (defaults are 3 and 16 respectively),
		 * reallocates the buffer with the same capacity but with the element at the current pointer moved to be at the start,
		 * sets the position of that pointer to the new start, and returns the number of elements removed.
		 * Otherwise, does nothing and returns zero.
		 */
		constexpr __size_type __trim_stale() { return __is_stale() ? __erase_before_next() : __size_type(0UL); }
		constexpr bool __q_grow_buffer(__size_type added);
		constexpr __pointer __push_elements(T const&, __size_type = 1UZ);
		constexpr __pointer __push_elements(T&&, __size_type = 1UZ);
		template<matching_input_iterator<T> IT> constexpr __pointer __push_elements(IT start, IT end) requires(!is_same_v<IT, __const_pointer>);
		template<matching_input_iterator<T> IT> constexpr __pointer __push_elements(IT what) requires(!is_same_v<IT, __const_pointer>) { IT end = what; end++; return __push_elements(what, end); }
		constexpr __pointer __push_elements(__const_pointer start, __const_pointer end);
		template<output_iterator<T> IT> constexpr __size_type __pop_elements(IT out_start, IT out_end) requires(!is_same_v<IT, __pointer>);
		constexpr __size_type __pop_elements(__pointer out_start, __pointer out_end);
		constexpr __size_type __erase_elements(__const_pointer start, __size_type n = 1UZ);
		constexpr __size_type __force_trim() { return __erase_before_next(); }
		constexpr __pointer __insert(__const_pointer where, __const_reference what, __size_type how_many = 1UZ);
		template<typename ... Args> requires constructible_from<T, Args...> constexpr __pointer __emplace_element(Args&& ... args) { if(__qmax() <= __end() && !__q_grow_buffer(1UL)) return nullptr; else { __pointer result = construct_at(__end(), forward<Args>(args)...); __bumpe(1L); return result; } }
		constexpr void __qclear() { __size_type cap = __qcapacity(); __qdestroy(); __my_queue_data.__set_ptrs(__qallocator.allocate(cap), cap); on_modify_queue(); }
		constexpr void __qswap(__dynamic_queue& that) noexcept { __my_queue_data.__swap(that.__my_queue_data); }
		constexpr __dynamic_queue() noexcept(noexcept(__allocator_type())) = default;
		constexpr __dynamic_queue(__size_type sz, __allocator_type alloc = __allocator_type()) : __qallocator(alloc), __my_queue_data(__qallocator.allocate(sz), sz) {}
		constexpr __dynamic_queue(__dynamic_queue const& that) : __qallocator(that.__qallocator), __my_queue_data(that.__my_queue_data) {}
		constexpr __dynamic_queue(__dynamic_queue&& that) : __qallocator(that.__qallocator), __my_queue_data(move(that.__my_queue_data)) {}
		constexpr ~__dynamic_queue() { __qdestroy(); }
		constexpr __dynamic_queue& operator=(__dynamic_queue&& that) { __qdestroy(); __q_move_assign(move(that)); return *this; }
		constexpr __dynamic_queue& operator=(__dynamic_queue const& that) { __qdestroy(); __q_copy_assign(that); return *this; }
	};
	template<typename T, allocator_object<T> A>
	constexpr typename __dynamic_queue<T, A>::__size_type __dynamic_queue<T, A>::__erase_before_next()
	{
		size_t result	= __tell();
		__ptr_container tmp(__qallocator.allocate(__qcapacity()), __qcapacity());
		array_move(tmp.__begin, __qcur(), __qrem());
		tmp.__bumpc(__qrem());
		__qdestroy();
		__my_queue_data.__copy_ptrs(tmp);
		return result;
	}
	template<typename T, allocator_object<T> A>
	constexpr bool __dynamic_queue<T, A>::__q_grow_buffer(typename __dynamic_queue<T, A>::__size_type added)
	{
		if(__unlikely(!added)) return true;		// Vacuously true success value
		__size_type num_elements	= __qsize();
		__size_type nx_offs			= __stell();
		__size_type target_cap		= __qcapacity() + added;
		__my_queue_data.__set_ptrs(resize(__qbeg(), __qcapacity(), target_cap, __qallocator), nx_offs, num_elements, target_cap);
		if(__unlikely(!__qbeg())) return false;	// No-throw allocators will return null if they fail
		return true;
	}
	template<typename T, allocator_object<T> A>
	constexpr typename __dynamic_queue<T, A>::__pointer __dynamic_queue<T, A>::__push_elements(T const& t, typename __dynamic_queue<T, A>::__size_type count)
	{
		if(__qmax() <= __end() + count && __unlikely(!__q_grow_buffer(static_cast<__size_type>(count - __cap_rem())))) return nullptr;
		__qset(__end(), t, count);
		__bumpe(count);
		__qallocator.__op_cnt++;
		on_modify_queue();
		return __end();
	}
	template<typename T, allocator_object<T> A>
	constexpr typename __dynamic_queue<T, A>::__pointer __dynamic_queue<T, A>::__push_elements(T&& t, typename __dynamic_queue<T, A>::__size_type count)
	{
		if(__qmax() <= __end() + count && __unlikely(!__q_grow_buffer(static_cast<__size_type>(count - __cap_rem())))) return nullptr;
		__qassign(__end(), move(t), count);
		__bumpe(count);
		__qallocator.__op_cnt++;
		on_modify_queue();
		return __end();
	}
	template<typename T, allocator_object<T> A>
	constexpr typename __dynamic_queue<T, A>::__pointer __dynamic_queue<T, A>::__push_elements(__const_pointer start, __const_pointer end)
	{
		if(__unlikely(end < start)) return nullptr;
		if(end == start) return __end();
		__size_type n	= end - start;
		if(__qmax() <= __end() + n && __unlikely(!__q_grow_buffer(static_cast<__size_type>(n - __cap_rem())))) return nullptr;
		__qcopy(__end(), start, n);
		__bumpe(n);
		__qallocator.__op_cnt++;
		on_modify_queue();
		return __end();
	}
	template<typename T, allocator_object<T> A>
	constexpr typename __dynamic_queue<T, A>::__size_type __dynamic_queue<T, A>::__pop_elements(__pointer out_start, __pointer out_end)
	{
		if(__unlikely(out_end <= out_start)) return 0UL;
		__size_type count		= min(__qrem(), static_cast<__size_type>(out_end - out_start));
		__qcopy(out_start, __qcur(), count);
		__bumpn(count);
		__qallocator.__op_cnt	= 0U;
		on_modify_queue();
		return count;
	}
	template<typename T, allocator_object<T> A>
	template<matching_input_iterator<T> IT>
	constexpr typename __dynamic_queue<T, A>::__pointer __dynamic_queue<T, A>::__push_elements(IT start, IT end)
	requires(!is_same_v<IT, typename __dynamic_queue<T, A>::__const_pointer>)
	{
		if(__unlikely(end < start)) return nullptr;
		if(end == start) return __end();
		__size_type n	= end - start;
		if(__qmax() <= __end() + n && !__q_grow_buffer(static_cast<__size_type>(n - __cap_rem()))) return nullptr;
		__qtransfer(__end(), start, end);
		__bumpe(n);
		__qallocator.__op_cnt++;
		on_modify_queue();
		return __end();
	}
	template<typename T, allocator_object<T> A>
	template<output_iterator<T> IT>
	constexpr typename __dynamic_queue<T, A>::__size_type __dynamic_queue<T, A>::__pop_elements(IT out_start, IT out_end)
	requires(!is_same_v<IT, typename __dynamic_queue<T, A>::__pointer>)
	{
		if(__unlikely(out_end <= out_start)) return 0UL;
		__size_type n			= 0UZ;
		for(IT i = out_start; i != out_end && __qcur() != __end(); ++i, ++n, __bumpn()) *i = *__qcur();
		__qallocator.__op_cnt	= 0;
		on_modify_queue();
		return n;
	}
	template<typename T, allocator_object<T> A>
	constexpr typename __dynamic_queue<T, A>::__size_type __dynamic_queue<T, A>::__erase_elements(__const_pointer start, __size_type n)
	{
		if(__unlikely(__q_out_of_range(start))) return 0UL;
		if(start + n > __qmax()) { n = static_cast<__size_type>(__qmax() - start); }
		if(start == __qbeg() && start + n >= __end())
		{
			size_t r	= __qsize();
			__qdestroy();
			__my_queue_data.__reset();
			return r;
		}
		__ptr_container tmp(__qallocator.allocate(__qcapacity() - n), __qcapacity() - n);
		if(__end() > start + n) tmp.__bumpn(__qsize() - n);
		else if(__end() > start) tmp.__bumpn(start - __qbeg());
		else tmp.__bumpn(__qsize());
		if(__qcur() > start + n) tmp.__bumpc(__tell() - n);
		else if(__qcur() > start) tmp.__bumpc(start - __qbeg());
		else { tmp.__bumpc(__tell()); }
		if(tmp.__next > tmp.__end) tmp.__next = tmp.__end;
		__size_type pstart	= static_cast<__size_type>(start - __qbeg());
		__size_type srem	= static_cast<__size_type>(__qmax() - (start + n));
		array_move(tmp.__begin, __qbeg(), pstart);
		if(srem) array_move(tmp.__q_get_ptr(pstart), __q_get_ptr(pstart) + n, srem);
		__qdestroy();
		__my_queue_data.__copy_ptrs(tmp);
		return n;
	}
	template<typename T, allocator_object<T> A>
	constexpr typename __dynamic_queue<T, A>::__pointer __dynamic_queue<T, A>::__insert(__const_pointer where, __const_reference what, __size_type how_many)
	{
		if(__unlikely(__q_out_of_range(where))) return nullptr;
		if(__unlikely(!how_many)) return nullptr;
		__ptr_container tmp(__qallocator.allocate(__qcapacity() + how_many), __qcapacity() + how_many);
		if(where < __qcur()) tmp.__bumpc(__tell() + how_many);
		else tmp.__bumpc(__tell());
		if(where < __end()) tmp.__bumpn(__qsize() + how_many);
		else tmp.__bumpn(where - __qbeg());
		__size_type preface_elems							= where - __qbeg();
		if(preface_elems) array_move(tmp.__begin, __qbeg(), preface_elems);
		if(how_many == 1) *(tmp.__q_get_ptr(preface_elems))	= what;
		else __qset(tmp.__q_get_ptr(preface_elems), what, how_many);
		array_move(tmp.__q_get_ptr(preface_elems + how_many), __q_get_ptr(preface_elems), static_cast<__size_type>(__end() - where));
		__qdestroy();
		__my_queue_data.__copy_ptrs(tmp);
		return __q_get_ptr(preface_elems + how_many - 1);
	}
}
#endif