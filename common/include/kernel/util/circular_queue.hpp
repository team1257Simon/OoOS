#ifndef __CIRC_Q
#define __CIRC_Q
#include <kernel_api.hpp>
namespace ooos
{
	template<typename T, std::allocator_object<T> A = std::allocator<T>>
	struct circular_queue
	{
		typedef T value_type;
		typedef A allocator_type;
		typedef std::add_pointer_t<value_type> pointer;
		typedef std::add_pointer_t<std::add_const_t<value_type>> const_pointer;
		typedef decltype(*std::declval<pointer>()) reference;
		typedef decltype(*std::declval<const_pointer>()) const_reference;
		typedef decltype(sizeof(value_type)) size_type;
		typedef decltype(std::declval<pointer>() - std::declval<pointer>()) difference_type;
	private:
		constexpr static bool __copy_assign     = std::is_copy_assignable_v<value_type>;
		constexpr static bool __copy_construct  = std::is_copy_constructible_v<value_type> && !__copy_assign;
		constexpr static bool __move_assign     = std::is_move_assignable_v<value_type>;
		constexpr static bool __move_construct  = std::is_move_constructible_v<value_type> && !__move_assign;
		constexpr static bool __nothrow_zero    = noexcept(array_zero(std::declval<pointer>(), std::declval<size_t>()));
		constexpr static bool __trivial			= std::is_trivially_destructible_v<value_type> || std::is_trivial_v<value_type>;
		struct __circular_buffer : allocator_type
		{
			pointer __base;
			pointer __curr;
			pointer __last;
			pointer __bmax;
			constexpr __circular_buffer() noexcept(noexcept(allocator_type())) : allocator_type(), __base(), __curr(), __last(), __bmax() {}
			constexpr __circular_buffer(size_type n) : allocator_type(), __base(allocator_type::allocate(n)), __curr(__base), __last(__base), __bmax(__base + n) {}
			constexpr __circular_buffer(__circular_buffer const& that) requires(__copy_assign || __copy_construct) :
				allocator_type(that),
				__base(allocator_type::allocate(that.__capacity())),
				__curr(__base),
				__last(__base),
				__bmax(__base + that.__capacity())
				{ array_copy(__base, that.__base, that.__capacity()); }
			constexpr __circular_buffer(__circular_buffer&& that) noexcept(std::is_nothrow_move_constructible_v<allocator_type>) :
				allocator_type(std::move(that)),
				__base(that.__base),
				__curr(that.__curr),
				__last(that.__last),
				__bmax(that.__bmax)
				{ that.__reset(); }
			constexpr void __reset() noexcept { __base = __curr = __last = __bmax = pointer(); }
			constexpr void __rewind() noexcept { __curr = __last = __base; }
			constexpr void __flush() noexcept { __rewind(); if constexpr(std::is_default_constructible_v<value_type>) new(__curr) value_type(); }
			constexpr size_type __capacity() const noexcept { return static_cast<size_type>(__bmax - __base); }
			constexpr void __create() { __base = allocator_type::allocate(4UZ); __curr = __last = __base; __bmax = __base + 4UZ; }
			constexpr void __destroy() noexcept { allocator_type::deallocate(__base, __capacity()); }
			constexpr reference __peek() noexcept { return *__curr; }
			constexpr const_reference __peek() const noexcept { return *__curr; }
			constexpr void __expand_splice()
			{
				size_type s         = __capacity();
				size_type l         = static_cast<size_type>(__last - __base) + 1UZ;
				size_type c         = static_cast<size_type>(__bmax - __curr);
				size_type tar		= s * 2UZ;
				pointer tmp         = allocator_type::allocate(tar);
				copy_or_move(tmp, __curr, c);
				copy_or_move(tmp + c, __base, l);
				allocator_type::deallocate(__base, s);
				__base              = tmp;
				__curr              = tmp;
				__last              = tmp + c + l;
				__bmax              = __base + tar;
			}
			constexpr void __expand_direct()
			{
				size_type s         = __capacity();
				size_type tar		= s * 2UZ;
				__base              = ooos::resize(__base, s, tar, *this);
				__curr              = __base;
				__last              = __base + s;
				__bmax              = __base + tar;
			}
			constexpr void __adv_push()
			{
				pointer target          = __last + 1Z;
				if(target == __bmax && __curr == __base) __expand_direct();
				else if(target == __curr) __expand_splice();
				else
				{
					size_type tpos          = static_cast<size_type>(target - __base);
					size_type actual_tpos   = tpos % __capacity();
					__last                  = __base + actual_tpos;
				}
			}
			constexpr void __adv_pop() noexcept
			{
				if(__capacity() && __curr != __last)
				{
					if constexpr(!__trivial) __curr->~T();
					if(!(++__curr < __bmax))
						__curr 				= __base;
				}
			}
			constexpr void __adv_pop(size_type n) noexcept
			{
				if(n == 1UZ) __adv_pop();
				else
				{
					size_type cap 					= __capacity();
					if(cap)
					{
						size_type l = this->__length();
						if(n > l) n = l;
						if constexpr(!__trivial) for(size_type i = 0; i < n; i++) __adv_pop();
						else
						{
							size_type tpos          = static_cast<size_type>((__curr + n) - __base);
							size_type actual_tpos   = tpos % cap;
							__curr 					= pclamp(__base + actual_tpos, __base, __bmax);
						}
					}
				}
			}
			constexpr value_type __pop()
			requires(std::is_move_constructible_v<value_type>)
			{
				value_type result(std::move(*__curr));
				__adv_pop();
				return result;
			}
			constexpr value_type __pop()
			requires(std::is_copy_constructible_v<value_type> && !std::is_move_constructible_v<value_type>)
			{
				value_type result(*__curr);
				__adv_pop();
				return result;
			}
			constexpr void __push(value_type const& v)
			requires(__copy_assign) {
				*__last = v;
				__adv_push();
			}
			constexpr void __push(value_type const& v)
			requires(__copy_construct) {
				new(__last) value_type(v);
				__adv_push();
			}
			constexpr void __push(value_type&& v)
			requires(__move_assign) {
				*__last = std::move(v);
				__adv_push();
			}
			constexpr void __push(value_type&& v)
			requires(__move_construct) {
				new(__last) value_type(std::move(v));
				__adv_push();
			}
			constexpr size_type __length() const noexcept
			{
				difference_type diff 	= __last - __curr;
				if(!diff) return 0UZ;
				size_type cap 			= __capacity();
				return static_cast<size_type>((cap + diff) % cap);
			}
			template<typename ... Args> requires(std::constructible_from<value_type, Args...>)
			constexpr void __emplace(Args&& ... args) {
				new(__last) value_type(std::forward<Args>(args)...);
				__adv_push();
			}
		} __buffer;
		constexpr void __bcheck() { if(!__buffer.__base) __buffer.__create(); }
	public:
		constexpr ~circular_queue() noexcept { __buffer.__destroy(); }
		constexpr void push(value_type const& v) requires(__copy_assign || __copy_construct) { __bcheck(); __buffer.__push(v); }
		constexpr void push(value_type&& v) requires(__move_assign || __move_construct) { __bcheck(); __buffer.__push(std::move(v)); }
		template<typename ... Args> requires(std::constructible_from<value_type, Args...>) constexpr void emplace(Args&& ... args) { __bcheck(); __buffer.__emplace(std::forward<Args>(args)...); }
		constexpr value_type pop() { return __buffer.__pop(); }
		constexpr reference peek() noexcept { return __buffer.__peek(); }
		constexpr circular_queue() noexcept(noexcept(allocator_type())) = default;
		constexpr circular_queue(size_type n) : __buffer(n) {}
		constexpr circular_queue(circular_queue&& that) noexcept(std::is_nothrow_move_constructible_v<allocator_type>) : __buffer(std::move(that.__buffer)) {}
		constexpr circular_queue(circular_queue const& that) requires(__copy_assign || __copy_construct) : __buffer(that.__buffer) {}
		constexpr size_type capacity() const noexcept { return __buffer.__capacity(); }
		constexpr size_type size() const noexcept { return __buffer.__length(); }
		constexpr size_type length() const noexcept { return __buffer.__length(); }
		constexpr operator bool() const noexcept { return static_cast<bool>(__buffer.__length()); }
		constexpr void bump(size_type n) noexcept { __buffer.__adv_pop(n); }
		constexpr void flush() noexcept requires(__trivial) { __buffer.__flush(); }
		constexpr void clear() noexcept(__nothrow_zero && (__trivial || std::is_nothrow_destructible_v<value_type>))
		{
			if constexpr(!__trivial)
				while(__buffer.__length())
					__buffer.__pop();
			array_zero(__buffer.__base, __buffer.__capacity());
			__buffer.__rewind();
		}
	};
}
#endif