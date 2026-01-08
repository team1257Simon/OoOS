/**
 * Base structs for the contiguous STL containers such as vector, streambuf, and string, implementing a lot of the shared functionality of those types.
 * Like much of the namespace std the libk will see, there are things here that are not constexpr that normally would be.
 * Calls to the heap allocator, for instance, rely on the kernel frame pointer which resides somewhere that can't be constexpr.
 */
#ifndef __DYN_BUFFER
#define __DYN_BUFFER
#include <bits/stl_algobase.hpp>
#include <bits/stl_iterator.hpp>
#include <memory>
#include <limits>
#include <initializer_list>
#ifndef TARGET_SSO_SIZE
#define TARGET_SSO_SIZE 32UZ
#endif
namespace std::__impl
{
	template<input_iterator IT>
	constexpr size_t __clamp_diff(IT s, IT e) noexcept
	{
		if constexpr(requires{ std::distance(s, std::max(s, e)); }) 
			return static_cast<size_t>(std::distance(s, std::max(s, e)));
		else
		{
			size_t i = 0UZ;
			while(s != e)
				++s, ++i;
			return i;
		}
	}
	template<typename T> concept __can_sso = std::default_initializable<T> && std::is_trivially_copyable_v<T> && (std::is_move_assignable_v<T> || std::move_constructible<T>) && std::is_trivially_destructible_v<T>;
	template<typename C, typename A> concept __container_type = requires(C const& cclref, C& clref, C&& crref, A& aref)
	{
		typename C::__value_type;
		typename C::__pointer;
		typename C::__const_pointer;
		typename C::__size_type;
		typename C::__difference_type;
		requires(allocator_object<A, typename C::__value_type>);
		requires(std::convertible_to<std::remove_cvref_t<decltype(*std::declval<typename C::__pointer>())>, typename C::__value_type>);
		requires(std::convertible_to<std::remove_cvref_t<decltype(*std::declval<typename C::__const_pointer>())>, typename C::__value_type>);
		requires(std::integral<typename C::__difference_type>);
		requires(std::convertible_to<typename C::__size_type, std::size_t>);
		requires(std::is_default_constructible_v<C> && std::constructible_from<C, A&, typename C::__size_type> && std::copy_constructible<C> && std::move_constructible<C>);
		clref.__reset();
		clref.__move_ptrs(std::move(crref));
		clref.__swap_ptrs(clref);
		clref.__copy_ptrs(cclref);
		clref.__create(aref, std::declval<typename C::__size_type>());
		clref.__destroy(aref);
		clref.__resize(aref, std::declval<typename C::__size_type>(), std::declval<typename C::__size_type>());
		clref.__setc(std::declval<typename C::__size_type>());
		{ cclref.__get_ptr(std::declval<typename C::__size_type>()) } -> std::convertible_to<typename C::__pointer>;
		{ cclref.__beg() } -> std::convertible_to<typename C::__pointer>;
		{ cclref.__cur() } -> std::convertible_to<typename C::__pointer>;
		{ cclref.__max() } -> std::convertible_to<typename C::__pointer>;
		{ cclref.__capacity() } -> std::convertible_to<typename C::__size_type>;
		{ cclref.__remaining() } -> std::convertible_to<typename C::__size_type>;
		{ cclref.__size() } -> std::convertible_to<typename C::__size_type>;
	};
	/**
	 * The tri-pointer buffer that is the underlying data storage for std::vector and std::streambuf.
	 * This implementation uses a size value for the buffer-max pointer; std::streambuf will invoke the __max() function to compute that pointer's value.
	 * It has hooks for managing pointer ownership (move-assignment, swapping, deallocation, etc.) but does not tie them to the relevant operators.
	 * This is the responsibility of the container that uses it.
	 */
	template<typename T>
	struct __buf_ptrs
	{
		typedef T __value_type;
		typedef std::add_pointer_t<__value_type> __pointer;
		typedef std::add_pointer_t<std::add_const_t<__value_type>> __const_pointer;
		typedef decltype(sizeof(__value_type)) __size_type;
		typedef decltype(declval<__pointer>() - declval<__pointer>()) __difference_type;
		__pointer __begin{};
		__pointer __end{};
		__size_type __cap{};
		constexpr __buf_ptrs() noexcept = default;
		constexpr __buf_ptrs(__pointer start, __pointer end, __size_type cap) noexcept : __begin(start), __end(end), __cap(cap) {}
		constexpr __buf_ptrs(__pointer start, __pointer end, __pointer max) noexcept : __begin(start), __end(end), __cap(static_cast<__size_type>(end - start)) {}
		constexpr void __reset() noexcept { __begin = __end = __pointer(); __cap = 0UZ; }
		constexpr __buf_ptrs(__buf_ptrs const& that) noexcept : __begin(that.__begin), __end(that.__end), __cap(that.__cap) {}
		constexpr __buf_ptrs(__buf_ptrs&& that) noexcept : __begin(that.__begin), __end(that.__end), __cap(that.__cap) { that.__reset(); }
		constexpr __buf_ptrs(__pointer start, __pointer end) noexcept : __begin(start), __end(end), __cap(static_cast<__size_type>(end - start)) {}
		constexpr __buf_ptrs(__pointer start, __size_type cap) noexcept : __begin(start), __end(start), __cap(cap) {}
		template<allocator_object<__value_type> A> constexpr __buf_ptrs(A& alloc, __size_type cap) : __begin(alloc.allocate(cap)), __end(__begin), __cap(cap) {}
		constexpr void __copy_ptrs(__buf_ptrs const& that) noexcept { __begin = that.__begin; __end = that.__end; __cap = that.__cap; }
		constexpr void __set_ptrs(__pointer begin, __pointer end, __size_type ncap) noexcept { __begin = begin; __end = end; __cap = ncap; }
		constexpr void __set_ptrs(__pointer begin, __pointer end, __pointer max) noexcept { __begin = begin; __end = end; __cap = static_cast<__size_type>(max - begin); }
		constexpr void __set_ptrs(__pointer begin, __size_type ncap) noexcept { __set_ptrs(begin, begin + min(ncap, static_cast<__size_type>(__end - __begin)), ncap); }
		constexpr __pointer __get_ptr(__size_type offs) const noexcept { return __begin + offs; }
		constexpr void __setc(__pointer where) { __end = where; }
		constexpr void __setc(__size_type offs) { __end = __begin + offs; }
		constexpr void __adv(__size_type n) { __end += n; }
		constexpr void __bck(__size_type n) { __end -= n; }
		constexpr __size_type __capacity() const noexcept { return __cap; }
		constexpr __size_type __size() const noexcept { return static_cast<__size_type>(__end - __begin); }
		constexpr __size_type __remaining() const noexcept { return static_cast<__size_type>(__cap - __size()); }
		constexpr __pointer __beg() const noexcept { return __begin; }
		constexpr __pointer __cur() const noexcept { return __end; }
		constexpr __pointer __max() const noexcept { return __begin + __cap; }
		constexpr void __swap_ptrs(__buf_ptrs& that) noexcept
		{
			__buf_ptrs tmp(this->__begin, this->__end, this->__cap);
			this->__copy_ptrs(that);
			that.__copy_ptrs(tmp);
		}
		constexpr void __move_ptrs(__buf_ptrs&& that)
		{
			this->__begin	= that.__begin;
			this->__end		= that.__end;
			this->__cap		= that.__cap;
			that.__reset();
		}
		template<allocator_object<__value_type> A>
		constexpr void __destroy(A& alloc)
		{
			if consteval { if(!__begin) return; }
			else { if(__unlikely(!__begin)) return; }
			if constexpr(!std::is_trivially_destructible_v<__value_type>)
				for(__pointer p = __begin; p < __end; p++)
					p->~T();
			alloc.deallocate(__begin, __cap);
			__reset();
		}
		template<allocator_object<__value_type> A>
		constexpr void __create(A& alloc, __size_type cap)
		{
			if consteval { if(__begin != nullptr) this->__destroy(alloc); }
			else { if(__unlikely(__begin != nullptr)) this->__destroy(alloc); }
			__begin	= alloc.allocate(cap);
			__end	= __begin;
			__cap	= cap;
		}
		template<allocator_object<__value_type> A>
		constexpr void __resize(A& alloc, __size_type ncur, __size_type ncap)
		{
			if constexpr(__has_resize<A, __value_type>)
				__begin		= alloc.resize(__begin, ncur, ncap);
			else __begin	= std::resize(__begin, __cap, ncap, alloc);
			__cap			= ncap;
			if(ncur > __cap)
				ncur		= __cap;
			__end			= __begin + ncur;
		}
	};
	/**
	 * Special buffer container for SSO strings.
	 * This implementation, structurally, is heavily based on the implementation in the GCC libstdc++-v3.
	 * However, it only provides the underlying structure and handles the differences in memory management logic versus the tri-pointer container.
	 * That logic does include managing the null-terminator of the string.
	 * The remaining functionality, including appending, insertion, erasure, and splicing, is handled by the __dynamic_buffer container-base class.
	 */
	template<__can_sso T>
	struct __sso_buffer
	{
		typedef T __value_type;
		typedef std::add_pointer_t<__value_type> __pointer;
		typedef std::add_pointer_t<std::add_const_t<__value_type>> __const_pointer;
		typedef decltype(sizeof(__value_type)) __size_type;
		typedef decltype(declval<__pointer>() - declval<__pointer>()) __difference_type;
		constexpr static __size_type __local_length		= static_cast<__size_type>(TARGET_SSO_SIZE - (1UZ + sizeof(__pointer) + sizeof(__size_type))) / sizeof(__value_type);
		constexpr static __size_type __local_capacity	= __local_length + 1UZ;
		typedef __value_type __local_buffer[__local_capacity];
		__pointer __begin;
		__size_type __length;
		union {
			__local_buffer __local_data_buf;
			__size_type __allocated_size;
		};
		constexpr __size_type __range(__size_type c) noexcept { return this->__is_local() ? std::min(c, __local_length) : c; }
		constexpr __pointer __local_data() noexcept { return std::addressof(*__local_data_buf); }
		constexpr __const_pointer __local_data() const noexcept { return std::addressof(*__local_data_buf); }
		constexpr __pointer __beg() const noexcept { return __begin; }
		constexpr __pointer __cur() const noexcept { return __begin + __length; }
		constexpr __pointer __max() const noexcept { return __begin + this->__capacity(); }
		constexpr __pointer __get_ptr(__size_type n) const noexcept { return __begin + n; }
		constexpr void __beg(__pointer p) noexcept { __begin = p; }
		constexpr void __cur(__size_type c) noexcept { __length = __range(c); }
		constexpr void __setc(__size_type c) noexcept { __cur(c); __begin[__length] = __value_type(); }
		constexpr void __setc(__pointer p) noexcept { __cur(static_cast<__size_type>(std::addressof((p[0] = __value_type())) - __begin)); }
		constexpr __size_type __capacity() const noexcept { return this->__is_local() ? __local_length : __allocated_size; }
		constexpr __size_type __size() const noexcept { return __length; }
		constexpr void __capacity(__size_type n) noexcept { __allocated_size = n; }
		constexpr void __reset() noexcept { this->__use_local(); this->__setc(0UZ); }
		constexpr void __adv(__size_type n) noexcept { this->__setc(__length + n); }
		constexpr void __bck(__size_type n) noexcept { this->__setc(static_cast<__size_type>(__length < n ? 0UZ : __length - n)); }
		constexpr __sso_buffer() noexcept : __begin(this->__local_data()), __length(0UZ), __local_data_buf() {}
		constexpr __sso_buffer(__sso_buffer const& that) noexcept : __sso_buffer() { this->__copy_ptrs(that); }
		constexpr __sso_buffer(__sso_buffer&& that) noexcept : __sso_buffer() { this->__move_ptrs(std::move(that)); }
		template<allocator_object<T> A> constexpr __sso_buffer(A& a, __size_type sz) : __sso_buffer() { this->__create(a, sz); }
		constexpr __size_type __remaining() const noexcept
		{
			if(this->__is_local())
				return static_cast<__size_type>(__local_length - __length);
			return static_cast<__size_type>(__allocated_size - __length - 1UZ);
		}
		constexpr void __move_ptrs(__sso_buffer&& that) noexcept
		{
			if(that.__is_local()) array_init(this->__use_local(), that.__local_data(), that.__length);
			else { this->__begin = that.__begin; this->__capacity(that.__capacity()); }
			this->__setc(that.__length);
			that.__reset();
		}
		constexpr void __swap_ptrs(__sso_buffer& that) noexcept
		{
			__sso_buffer tmp(*this);
			this->__copy_ptrs(that);
			that.__move_ptrs(std::move(tmp));
		}
		constexpr void __copy_ptrs(__sso_buffer const& that) noexcept
		{
			if(that.__is_local()) array_init(this->__use_local(), that.__local_data(), that.__length);
			else { this->__begin = that.__begin; this->__capacity(that.__capacity()); }
			this->__setc(that.__length);
		}
		constexpr bool __is_local() const noexcept
		{
			if(__beg() == __local_data())
			{
				if(__length > __local_length)
					__builtin_unreachable();
				return true;
			}
			return false;
		}
		__always_inline constexpr __pointer __use_local() noexcept
		{
			if consteval {
				for(__size_type i		= 0UZ; i < __local_capacity; i++)
					__local_data()[i]	= __value_type();
			}
			return this->__begin		= __local_data();
		}
		template<allocator_object<__value_type> A>
		constexpr void __create(A& a, __size_type sz)
		{
			if consteval { if(__beg() != nullptr) this->__destroy(a); }
			else { if(__unlikely(__beg() != nullptr)) this->__destroy(a); }
			if(sz > __local_capacity)
			{
				__pointer result	= a.allocate(sz);
				this->__length		= 0UZ;
				this->__begin		= result;
				this->__capacity(sz);
			}
			else
			{
				sz					= std::max(sz, 1UZ);
				array_zero(this->__use_local(), __local_capacity);
				this->__length		= 0UZ;
			}
		}
		template<allocator_object<__value_type> A>
		constexpr void __destroy(A& a)
		{
			if(!__is_local())
				a.deallocate(__begin, __allocated_size);
			else __reset();
		}
		template<allocator_object<__value_type> A>
		constexpr void __resize(A& alloc, __size_type ncur, __size_type ncap)
		{
			ncap	= std::max(ncap, ncur + 1UZ);
			if(ncap > __local_capacity)
			{
				__pointer result	= alloc.allocate(ncap);
				if(this->__length)
				{
					if(this->__is_local()) array_copy(result, this->__local_data(), this->__length);
					else if(this->__begin) array_copy(result, this->__begin, this->__length);
					else array_zero(result, this->__length);
				}
				this->__destroy(alloc);
				this->__begin		= result;
				this->__setc(ncur);
				this->__capacity(ncap);
			}
			else
			{
				if(!__is_local())
				{
					__pointer old	= this->__begin;
					__size_type c	= this->__capacity();
					this->__beg(array_copy(this->__use_local(), old, ncur));
					alloc.deallocate(old, c);
				}
				this->__setc(ncur);
			}
		}
	};
	template<typename A, __container_type<A> C>
	struct __buffer_container : A, C
	{
		using typename C::__value_type;
		using typename C::__pointer;
		using typename C::__const_pointer;
		using typename C::__size_type;
		using typename C::__difference_type;
		constexpr __buffer_container() noexcept(noexcept(A())) : A(), C() {}
		constexpr __buffer_container(__size_type s) : A(), C(*this, s) {}
		constexpr __buffer_container(A const& that) noexcept(std::is_nothrow_copy_constructible_v<A>) : A(that), C() {}
		constexpr __buffer_container(A const& that, __size_type s) : A(that), C(*this, s) {}
		constexpr __buffer_container(__buffer_container&& that) noexcept(std::is_nothrow_move_constructible_v<A>) : A(forward<A>(that)), C(forward<C>(that)) {}
		constexpr __buffer_container(__buffer_container const& that) noexcept(std::is_nothrow_copy_constructible_v<A>) : A(that), C(that) {}
		constexpr __buffer_container(__buffer_container const& that, A const& a) : A(a), C(that) {}
		constexpr __buffer_container(__buffer_container&& that, A const& a) : A(a), C(std::forward<C>(that)) {}
		constexpr void __create(__size_type cap) { C::__create(*this, cap); }
		constexpr void __destroy() { C::__destroy(*this); }
		constexpr void __resize(__size_type ncur, __size_type ncap) { C::__resize(*this, ncur, ncap); }
		constexpr __buffer_container& operator=(__buffer_container const& that)
		{
			this->__destroy();
			this->__copy_ptrs(that);
			if constexpr(__has_copy_propagate<A>)
				*static_cast<A*>(this)	= that;
			return *this;
		}
		constexpr __buffer_container& operator=(__buffer_container&& that)
		{
			this->__destroy();
			this->__move_ptrs(std::forward<C>(that));
			if constexpr(__has_move_propagate<A>)
				*static_cast<A*>(this)	= std::move(that);
			return *this;
		}
	};
	template<typename T, bool> struct __container_select { typedef __buf_ptrs<T> type; };
	template<__can_sso T> struct __container_select<T, true> { typedef __sso_buffer<T> type; };
	/**
	 * This base-type implements the functionality shared by the dynamic-container types (mainly string and vector).
	 * If the buffer is for a null-terminated string, use NTS = true.
	 * The container will select the SSO-string buffer implementation if possible when NTS is set to true.
	 * When NTS is false, it will select the tri-pointer buffer used with std::vector instead.
	 * It will also do so if the type T cannot be used with SSO (e.g. a type that is not default-initializable).
	 */
	template<typename T, allocator_object<T> A, bool NTS>
	struct __dynamic_buffer
	{
		typedef __buffer_container<A, typename __container_select<T, NTS>::type> __container;
		typedef typename __container::__value_type __value_type;
		typedef typename __container::__pointer __pointer;
		typedef typename __container::__const_pointer __const_pointer;
		typedef typename __container::__size_type __size_type;
		typedef typename __container::__difference_type __difference_type;
		typedef deref_t<__pointer> __reference;
		typedef deref_t<__const_pointer> __const_reference;
		typedef A __allocator_type;
		constexpr static bool __using_sso	= NTS && __can_sso<T>;
		constexpr static bool __trivial		= trivial_copy<__value_type>;
		__container __my_data;
		constexpr static __const_pointer __pmin(__const_pointer a, __const_pointer b) { return a < b ? a : b; }
		/**
		 * Copies data using iterators that might not be contiguous; i.e. not linear (e.g. tree iterators) or not forward-facing (e.g. reverse iterators).
		 * If the iterator happens to be contiguous (e.g. a pointer), this will simplify into the normal copy operation.
		 */
		template<matching_input_iterator<T> IT> constexpr void __transfer(__pointer where, IT start, IT end);
		constexpr void __set(__pointer where, __const_reference val, __size_type n) { fill_n(where, n, val); }
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
		template<matching_input_iterator<T> IT> constexpr __pointer __append_elements(IT start_it, IT end_it);
		template<matching_input_iterator<T> IT> constexpr __pointer __insert_elements(__const_pointer pos, IT start_ptr, IT end_ptr);
		template<typename ... Args> requires(constructible_from<T, Args...>) constexpr __pointer __emplace_element(__const_pointer pos, Args&& ... args);
		template<typename ... Args> requires(constructible_from<T, Args...>) constexpr __pointer __emplace_at_end(Args&& ... args);
		constexpr __pointer __replace_elements(__size_type pos, __size_type count, __const_pointer from, __size_type count2);
		constexpr __pointer __erase_range(__const_pointer start, __const_pointer end);
		constexpr __pointer __insert_element(__const_pointer pos, T const& t) { return __insert_elements(pos, addressof(t), addressof(t) + 1Z); }
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
		constexpr __reference __get_last() noexcept { return *__get_ptr(__size() - 1Z); }
		constexpr __const_reference __get_last() const noexcept { return *__get_ptr(__size() - 1); }
		constexpr void __setp(typename __container_select<T, NTS>::type const& ptrs) noexcept { __my_data.__copy_ptrs(ptrs); }
		constexpr void __setp(__pointer beg, __pointer cur, __pointer end) noexcept requires(!__using_sso) { __my_data.__set_ptrs(beg, cur, end); }
		constexpr void __setp(__pointer beg, __pointer end) noexcept requires(!__using_sso) { __setp(beg, beg, end); }
		constexpr void __setn(__pointer beg, __size_type c, __size_type n) noexcept requires(!__using_sso) { __setp(beg, beg + c, beg + n); }
		constexpr void __setn(__pointer beg, __size_type n) noexcept requires(!__using_sso) { __setp(beg, beg + n); }
		constexpr void __setc(__pointer pos) noexcept { if(__valid_end_pos(pos)) __my_data.__setc(pos); }
		constexpr void __setc(__size_type pos) noexcept { __setc(__get_ptr(pos)); }
		constexpr void __bumpc(int64_t off) noexcept { __setc(__cur() + off); }
		constexpr void __rst() noexcept { __setc(__beg()); }
		constexpr __size_type __max_capacity() const noexcept { return numeric_limits<__size_type>::max(); }
		constexpr void __advance(__size_type n) { if(__valid_end_pos(__cur() + n)) { __setc(__cur() + n); } else { __setc(__max()); } }
		constexpr void __backtrack(__size_type n) { if(__valid_end_pos(__cur() - n)) { __setc(__cur() - n); } else { __setc(__beg()); } }
		constexpr __size_type __size() const noexcept { return __my_data.__size(); }
		constexpr __size_type __capacity() const noexcept { return __my_data.__capacity(); }
		constexpr __size_type __rem() const noexcept { return __my_data.__remaining(); }
		constexpr __size_type __ediff(__const_pointer pos) const noexcept { return static_cast<__size_type>(__cur() - pos); }
		constexpr __allocator_type const& __get_alloc() const noexcept { return __my_data; }
		constexpr void __allocate_storage(__size_type n) { __my_data.__create(n); }
		constexpr __pointer __create_temp(__size_type n) { return __my_data.allocate(n); }
		constexpr void __destroy_temp(__pointer tmp, __size_type n) { __my_data.deallocate(tmp, n); }
		constexpr void __data_resize(__size_type ncur, __size_type ncap) { __my_data.__resize(ncur, ncap); }
		constexpr __container __temp_container(__size_type cap) { return __container(__my_data, cap); }
		constexpr explicit __dynamic_buffer(A const& alloc) noexcept(noexcept(A(alloc))) : __my_data(alloc) {}
		constexpr __dynamic_buffer() noexcept(noexcept(A())) : __my_data() {}
		constexpr __dynamic_buffer(__size_type sz) : __my_data(sz) {}
		constexpr __dynamic_buffer(__size_type sz, A const& alloc) : __my_data(alloc, sz) {}
		constexpr __dynamic_buffer(__size_type sz, __const_reference val, A const& alloc) : __my_data(alloc, sz) { if(sz) { __set(__beg(), val, sz); __advance(sz); } }
		constexpr __dynamic_buffer(__dynamic_buffer const& that) : __dynamic_buffer(that.__beg(), that.__cur(), that.__get_alloc()) {}
		constexpr __dynamic_buffer(__dynamic_buffer const& that, A const& alloc) : __dynamic_buffer(that.__beg(), that.__cur(), alloc) {}
		constexpr __dynamic_buffer(__dynamic_buffer const& that, __size_type start, A const& alloc) : __dynamic_buffer(that.__get_ptr(start), that.__cur(), alloc) {}
		constexpr __dynamic_buffer(__dynamic_buffer const& that, __size_type start, __size_type count, A const& alloc) : __dynamic_buffer(that.__get_ptr(start), __pmin(that.__get_ptr(start + count), that.__cur()), alloc) {}
		constexpr __dynamic_buffer(__dynamic_buffer&& that) noexcept(noexcept(__container(move(that.__my_data)))) : __my_data(move(that.__my_data)) {}
		constexpr  __dynamic_buffer(__dynamic_buffer&& that, A const& alloc) noexcept(noexcept(__container(alloc))) : __my_data(move(that.__my_data), alloc) {}
		constexpr ~__dynamic_buffer() { this->__destroy(); }
		constexpr __dynamic_buffer& operator=(__dynamic_buffer const& that) { this->__copy_assign(that); return *this; }
		constexpr __dynamic_buffer& operator=(__dynamic_buffer&& that) { this->__move_assign(move(that)); return *this; }
		constexpr __pointer __assign_elements(initializer_list<T> ini) { return this->__assign_elements(ini.begin(), ini.end()); }
		constexpr __pointer __erase(__const_pointer pos) { return this->__erase_range(pos, pos + 1Z); }
		constexpr void __assign_ptrs(__container const& c) noexcept { __my_data.__copy_ptrs(c); }
		constexpr void __move_assign(__dynamic_buffer&& that) { this->__my_data	= std::move(that.__my_data); }
		constexpr bool __grow_buffer_exact(__size_type added);
		constexpr __dynamic_buffer(initializer_list<T> const& __ils, A const& alloc) : __my_data(alloc, __ils.size())
		{
			for(size_t i = 0UZ; i < __ils.size(); i++)
				*this->__get_ptr(i)	= __ils.begin()[i];
			this->__setc(__ils.size());
		}
		template<matching_input_iterator<T> IT>
		constexpr __dynamic_buffer(IT start, IT end, A const& alloc) : __my_data(alloc, __clamp_diff(start, end))
		{
			__size_type n	= __clamp_diff(start, end);
			if constexpr(totally_ordered<IT>)
				__transfer(__beg(), start, std::max(start, end));
			else __transfer(__beg(), start, end);
			__setc(n);
		}
		constexpr void __trim_buffer()
		{
			__size_type num_elements	= __size();
			__data_resize(num_elements, num_elements);
			__setc(num_elements);
		}
		constexpr void __size_buffer(__size_type n)
		{
			__size_type num_elements	= __size();
			__data_resize(std::min(n, num_elements), n);
			__setc(num_elements);
		}
		constexpr void __construct_element(__pointer pos, T const& t)
		{
			if consteval { if(__out_of_range(pos)) return; }
			else { if(__unlikely(__out_of_range(pos))) return; }
			construct_at(pos, t);
			if(pos > __cur()) __setc(pos);
		}
		constexpr __pointer __assign_elements(__size_type count, T const& t)
		{
			if(count > __capacity())
			{
				if consteval { __grow_buffer(count - __capacity()); }
				else { if(__unlikely(!__grow_buffer(count - __capacity()))) return nullptr; }
			}
			__set(__beg(), t, count);
			__setc(count);
			if(count < __size()) __zero(__cur(), __size() - count);
			return __cur();
		}
		constexpr __pointer __assign_elements(__const_pointer start, __const_pointer end)
		{
			__size_type count	= end - start;
			if(count > __capacity())
			{
				if consteval { __grow_buffer(count - __capacity()); }
				else { if(__unlikely(!__grow_buffer(count - __capacity()))) return nullptr; }
			}
			__copy(__beg(), start, count);
			__setc(count);
			if(count < __size()) __zero(__cur(), __size() - count);
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
			__size_type bsz		= __size();
			if(!(__max() > __cur() + count))
			{
				if consteval { __grow_buffer(static_cast<__size_type>(count - __rem())); }
				else { if(__unlikely(!__grow_buffer(static_cast<__size_type>(count - __rem())))) return nullptr; }
			}
			__size_type tsz		= bsz + count;
			__pointer pos		= __get_ptr(bsz);
			for(__size_type i	= 0UZ; i < count; i++) construct_at(pos + i, t);
			__setc(tsz);
			return __cur();
		}
		constexpr __pointer __append_element(T const& t)
		{
			__size_type bsz		= __size();
			if(!(__max() > __cur()))
			{
				if consteval { __grow_buffer(1UZ); }
				else { if(__unlikely(!__grow_buffer(1UZ))) return nullptr; }
			}
			construct_at(__get_ptr(bsz), t);
			__setc(bsz + 1UZ);
			return __cur();
		}
		constexpr void __clear()
		{
			__size_type cap		= __capacity();
			__destroy();
			__allocate_storage(cap);
			if constexpr(__trivial)	__zero(__beg(), __capacity());
		}
		constexpr __pointer __erase_at_end(__size_type how_many)
		{
			if(__unlikely(how_many >= __size())) __clear();
			else { __backtrack(how_many); __zero(__cur(), how_many); }
			return __cur();
		}
		constexpr void __destroy() {
			if(this->__beg())
				__my_data.__destroy();
		}
		constexpr void __copy_assign(__dynamic_buffer const& that)
		{
			__destroy();
			if consteval { if(!that.__beg()) return; }
			else{ if(__unlikely(!that.__beg())) return; }
			if constexpr(__has_copy_propagate<__allocator_type>)
				*static_cast<__allocator_type*>(std::addressof(this->__my_data)) = that.__get_alloc();
			__allocate_storage(that.__capacity());
			__copy(__beg(), that.__beg(), that.__capacity());
			__advance(that.__size());
		}
		constexpr void __swap(__dynamic_buffer& that)
		{
			this->__my_data.__swap_ptrs(that.__my_data);
			if constexpr(__has_swap_propagate<__allocator_type>)
				std::swap<__allocator_type>(this->__my_data, that.__my_data);
		}
	};
	template<typename T, allocator_object<T> A, bool NTS>
	template<matching_input_iterator<T> IT>
	constexpr void __dynamic_buffer<T, A, NTS>::__transfer(__pointer where, IT start, IT end)
	{
		if consteval { array_init(where, start, end); }
		else
		{
			if constexpr(totally_ordered<IT>)
				if(__unlikely(!(end > start))) return;
			if constexpr(contiguous_iterator<IT>)
				array_copy(where, addressof(*start), static_cast<size_t>(distance(start, end)));
			else for(IT i = start; i != end; i++, where++)
				*where = *i;
		}
	}
	template<typename T, allocator_object<T> A, bool NTS>
	constexpr void __dynamic_buffer<T, A, NTS>::__zero(__pointer where, __size_type n)
	{
		if constexpr(!__trivial)
			for(__size_type i = 0UZ; i < n; i++)
				where[i].~T();
		else array_zero(where, n);
	}
	template<typename T, allocator_object<T> A, bool NTS>
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer
	__dynamic_buffer<T, A, NTS>::__replace_elements(__size_type pos, __size_type count, __const_pointer from, __size_type count2)
	{
		if(count2 == count) __copy(__get_ptr(pos), from, count);
		else
		{
			__difference_type diff	= count2 - count;
			__size_type target_cap	= __capacity() + diff;
			__size_type rem			= __size() - (pos + count);
			__container nwdat		= __temp_container(target_cap);
			__move(nwdat.__begin, __beg(), pos);
			__copy(nwdat.__get_ptr(pos), from, count2);
			__move(nwdat.__get_ptr(pos + count2), __get_ptr(pos + count), rem);
			__size_type rs_size		= __size() + diff;
			__destroy();
			__assign_ptrs(nwdat);
			__setc(rs_size);
		}
		return __get_ptr(pos + count);
	}
	template<typename T, allocator_object<T> A, bool NTS>
	constexpr bool __dynamic_buffer<T, A, NTS>::__grow_buffer(__size_type added)
	{
		if(!added) return true;					// Zero elements -> vacuous success
		__size_type num_elements	= __size();
		__size_type cur_capacity	= __capacity();
		__size_type target 			= min(max(cur_capacity << 1, cur_capacity + added + (__using_sso ? 1UZ : 0UZ)), __max_capacity());
		__data_resize(num_elements + (__using_sso ? added : 0UZ), target);
		if(__unlikely(!__cur())) return false;	// No-throw allocators will return null if they fail
		return true;
	}
	template<typename T, allocator_object<T> A, bool NTS>
	constexpr bool __dynamic_buffer<T, A, NTS>::__grow_buffer_exact(__size_type added)
	{
		if(!added) return true;					// Zero elements -> vacuous success
		__size_type num_elements	= __size();
		__size_type cur_capacity	= __capacity();
		__size_type target 			= cur_capacity + added + (__using_sso ? 1UZ : 0UZ);
		__data_resize(num_elements + (__using_sso ? added : 0UZ), target);
		return __cur() != nullptr;	// No-throw allocators will return null if they fail
	}
	template<typename T, allocator_object<T> A, bool NTS>
	template<matching_input_iterator<T> IT>
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer __dynamic_buffer<T, A, NTS>::__append_elements(IT start_it, IT end_it)
	{
		if constexpr(totally_ordered<IT>)
			if(__unlikely(!(end_it > start_it)))
				return nullptr;
		__size_type rem	= __rem();
		__size_type num	= distance(start_it, end_it);
		__size_type bsz	= __size();
		__size_type tsz	= bsz + num;
		if((!__beg() || num > rem))
		{
			if consteval { __grow_buffer(num - rem); }
			else { if(__unlikely(!__grow_buffer(num - rem))) return nullptr; }
		}
		__transfer(__get_ptr(bsz), start_it, end_it);
		__setc(tsz);
		return __cur();
	}
	template<typename T, allocator_object<T> A, bool NTS>
	template<matching_input_iterator<T> IT>
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer
	__dynamic_buffer<T, A, NTS>::__insert_elements(__const_pointer pos, IT start_ptr, IT end_ptr)
	{
		if consteval {}
		else { if(__unlikely(__out_of_range(pos))) return nullptr; }
		__pointer ncpos				= __get_ptr(__diff(pos));
		__size_type range_size	= distance(start_ptr, end_ptr);
		__size_type offs		= __diff(pos);
		bool prepending			= (pos < __cur());
		if(pos + range_size < __max())
		{
			if(prepending)
			{
				__size_type n	= __ediff(pos);
				__pointer temp	= __create_temp(n);
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
			__transfer(__get_ptr(offs), start_ptr, end_ptr);
			__setc(osz + range_size);
		}
		return __get_ptr(offs);
		
	}
	template<typename T, allocator_object<T> A, bool NTS>
	template<typename ... Args>
	requires(constructible_from<T, Args...>)
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer __dynamic_buffer<T, A, NTS>::__emplace_element(__const_pointer pos, Args&& ... args)
	{
		if consteval {}
		else { if(__unlikely(pos < this->__beg())) return nullptr; }
		if(pos >= __max()) return __emplace_at_end(forward<Args>(args)...);
		__size_type start_pos	= __diff(pos);
		__size_type rem			= __ediff(pos);
		__size_type target_cap	= (__size() < __capacity()) ? __capacity() : __capacity() + 1;
		__container nwdat		= __temp_container(target_cap);
		__pointer result		= construct_at(nwdat.__get_ptr(start_pos), forward<Args>(args)...);
		__move(nwdat.__begin, __beg(), start_pos);
		if(rem) __move(result + 1, __get_ptr(start_pos), rem);
		__destroy();
		__assign_ptrs(nwdat);
		return result;
	}
	template<typename T, allocator_object<T> A, bool NTS>
	template<typename ... Args>
	requires(constructible_from<T, Args...>)
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer __dynamic_buffer<T, A, NTS>::__emplace_at_end(Args && ...args)
	{
		if(__size() == __capacity())
		{
			if consteval { __grow_buffer(1UZ); }
			else { if(__unlikely(!__grow_buffer(1UZ))) return nullptr; }
		}
		__pointer p	= construct_at(__cur(), forward<Args>(args)...);
		__advance(1UZ);
		return p;
	}
	template<typename T, allocator_object<T> A, bool NTS>
	constexpr typename __dynamic_buffer<T, A, NTS>::__pointer __dynamic_buffer<T, A, NTS>::__erase_range(__const_pointer start, __const_pointer end)
	{
		if consteval {}
		else { if(__unlikely(__out_of_range(start, end))) return nullptr; }
		__size_type how_many	= end - start;
		__size_type rem			= __ediff(end);
		__size_type start_pos	= __diff(start);
		if(!rem) return __erase_at_end(how_many);
		else
		{
			__pointer temp		= __create_temp(rem);
			__pointer ncend		= __get_ptr(__diff(end));
			__move(temp, ncend, rem);
			__zero(__get_ptr(start_pos), __ediff(start));
			__move(__get_ptr(start_pos), temp, rem);
			__destroy_temp(temp, rem);
			__setc(start_pos + rem);
		}
		return __get_ptr(start_pos);
	}
}
#endif