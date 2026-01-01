#ifndef __CIRC_Q
#define __CIRC_Q
#include <kernel_api.hpp>
namespace ooos
{
	/**
	 * A simple, single-sided queue structure with amortized constant-time insertion and extraction operations.
	 * If the type T is not trivially destructible, the bump() and clear() operation are linear; otherwise, they are also constant-time.
	 * The queue will never resize on an extraction, and will attempt to re-use the storage of extracted elements before expanding the buffer.
	 * Iterators have a partial validity guarantee across extractions.
	 * In particular, if an iterator X is valid and not the end() iterator, then X++ will yield a valid iterator after a single extraction.
	 * Similarly, X + N will yield a valid iterator after up to N extractions if it was valid in the first place (i.e. not past the end() iterator).
	 */
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
		class iterator_bound;
	private:
		constexpr static bool __copy_assign    		= std::is_copy_assignable_v<value_type>;
		constexpr static bool __copy_construct 		= std::is_copy_constructible_v<value_type> && !__copy_assign;
		constexpr static bool __move_assign     	= std::is_move_assignable_v<value_type>;
		constexpr static bool __move_construct  	= std::is_move_constructible_v<value_type> && !__move_assign;
		constexpr static bool __trivial				= std::is_trivially_destructible_v<value_type> || std::is_trivial_v<value_type>;
		constexpr static bool __nothrow_destruct	= __trivial || std::is_nothrow_destructible_v<value_type>;
		constexpr static bool __nothrow_zero    	= noexcept(array_zero(std::declval<pointer>(), std::declval<size_t>())) && __nothrow_destruct;
		struct __circular_buffer : allocator_type
		{
			pointer __base;
			pointer __curr;
			pointer __back;
			pointer __rmax;
			constexpr __circular_buffer() noexcept(noexcept(allocator_type())) :
				allocator_type(),
				__base(),
				__curr(),
				__back(),
				__rmax()
			{}
			constexpr __circular_buffer(size_type n) :
				allocator_type(),
				__base(allocator_type::allocate(n)),
				__curr(__base),
				__back(__base),
				__rmax(__base + n)
			{}
			constexpr __circular_buffer(__circular_buffer const& that)
			requires(__copy_assign || __copy_construct) :
				allocator_type(that),
				__base(allocator_type::allocate(that.__capacity())),
				__curr(this->__base + static_cast<size_type>(that.__curr - that.__base)),
				__back(this->__base + static_cast<size_type>(that.__back - that.__base)),
				__rmax(this->__base + that.__capacity())
				{ if(that.__base) array_copy(this->__base, that.__base, that.__capacity()); }
			constexpr __circular_buffer(__circular_buffer&& that)
			noexcept(std::is_nothrow_move_constructible_v<allocator_type>) :
				allocator_type(std::move(that)),
				__base(that.__base),
				__curr(that.__curr),
				__back(that.__back),
				__rmax(that.__rmax)
				{ that.__reset(); }
			constexpr void __reset() noexcept { __base = __curr = __back = __rmax = pointer(); }
			constexpr void __rewind() noexcept { __curr = __back = __base; }
			constexpr size_type __capacity() const noexcept { return static_cast<size_type>(__rmax - __base); }
			constexpr void __create(size_type n) { __base = allocator_type::allocate(n); __curr = __back = __base; __rmax = __base + n; }
			constexpr reference __peek() noexcept { return *__curr; }
			constexpr const_reference __peek() const noexcept { return *__curr; }
			constexpr reference __peek_back() noexcept { return *pclamp(__back - 1Z, __curr, __back); }
			constexpr const_reference __peek_back() const noexcept { return *pclamp(__back - 1Z, __curr, __back); }
			constexpr size_type __length() const noexcept { return this->__pos_of(__back); }
			constexpr pointer __get_ptr(size_type n) const noexcept { return this->__adv_ptr(__curr, n); }
			constexpr void __flush() noexcept requires(__trivial)
			{
				this->__rewind();
				if constexpr(std::is_default_constructible_v<value_type>)
					new(__curr) value_type();
			}
			constexpr void __swap(__circular_buffer& that)
			noexcept(std::is_nothrow_swappable_v<allocator_type> || !std::__has_swap_propagate<allocator_type>)
			{
				std::swap(this->__base, that.__base);
				std::swap(this->__curr, that.__curr);
				std::swap(this->__back, that.__back);
				std::swap(this->__rmax, that.__rmax);
				if constexpr(std::__has_swap_propagate<allocator_type>) std::swap<allocator_type>(*this, that);
			}
			constexpr __circular_buffer& operator=(__circular_buffer const& that) requires(__copy_assign || __copy_construct)
			{
				this->__destroy();
				if constexpr(std::__has_copy_propagate<allocator_type>)
					*static_cast<allocator_type*>(this)	= that;
				if(__unlikely(!that.__base)) this->__reset();
				else
				{
					this->__base	= allocator_type::allocate(that.__capacity());
					this->__curr	= this->__base + static_cast<size_type>(that.__curr - that.__base);
					this->__back	= this->__base + static_cast<size_type>(that.__back - that.__base);
					this->__rmax	= this->__base + that.__capacity();
				}
				return *this;
			}
			constexpr __circular_buffer& operator=(__circular_buffer&& that)
			noexcept(std::is_nothrow_move_constructible_v<allocator_type>)
			{
				this->__destroy();
				if constexpr(std::__has_move_propagate<allocator_type>)
					*static_cast<allocator_type*>(this)	= std::move(that);
				if(__unlikely(!that.__base)) this->__reset();
				else
				{
					this->__base	= that.__base;
					this->__curr	= that.__curr;
					this->__back	= that.__back;
					this->__rmax	= that.__rmax;
					that.__reset();
				}
				return *this;
			}
			constexpr void __destroy() noexcept
			{
				if(__unlikely(!__base)) return;
				if constexpr(!__trivial)
					for(size_type i = 0UZ; i < this->__length(); i++)
						this->__adv_pop();
				allocator_type::deallocate(__base, __capacity());
			}
			constexpr void __expand_splice()
			{
				size_type s         = __capacity();
				size_type l         = static_cast<size_type>(__back - __base) + 1UZ;
				size_type c         = static_cast<size_type>(__rmax - __curr);
				size_type tar		= s * 2UZ;
				pointer tmp         = allocator_type::allocate(tar);
				copy_or_move(tmp, __curr, c);
				copy_or_move(tmp + c, __base, l);
				allocator_type::deallocate(__base, s);
				__base              = tmp;
				__curr              = tmp;
				__back              = tmp + c + l;
				__rmax              = __base + tar;
			}
			constexpr void __expand_direct()
			{
				size_type s         = __capacity();
				size_type tar		= s * 2UZ;
				__base              = ooos::resize(__base, s, tar, *this);
				__curr              = __base;
				__back              = __base + s;
				__rmax              = __base + tar;
			}
			constexpr void __adv_push()
			{
				pointer target          = __back + 1Z;
				if(target == __rmax && __curr == __base) __expand_direct();
				else if(target == __curr) __expand_splice();
				else
				{
					size_type tpos          = static_cast<size_type>(target - __base);
					size_type actual_tpos   = tpos % __capacity();
					__back                  = __base + actual_tpos;
				}
			}
			constexpr pointer __adv_ptr(pointer p) const noexcept
			{
				if(!(++p < __rmax))
					p	= __base;
				return p;
			}
			constexpr pointer __adv_ptr(pointer p, size_type n) const noexcept
			{
				size_type tpos          = static_cast<size_type>((p + n) - __base);
				size_type actual_tpos   = tpos % __capacity();
				return pclamp(__base + actual_tpos, __base, __rmax);
			}
			constexpr void __adv_pop() noexcept
			{
				if(__capacity() && __curr != __back)
				{
					if constexpr(!__trivial)
						__curr->~T();
					__curr	= __adv_ptr(__curr);
				}
			}
			constexpr void __adv_pop(size_type n) noexcept
			{
				if(n == 1UZ) __adv_pop();
				else
				{
					size_type cap 	= __capacity();
					if(cap)
					{
						size_type l	= this->__length();
						if(n > l) n	= l;
						if constexpr(!__trivial) for(size_type i = 0; i < n; i++) __adv_pop();
						else __curr	= __adv_ptr(__curr, n);
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
				*__back = v;
				__adv_push();
			}
			constexpr void __push(value_type const& v)
			requires(__copy_construct) {
				new(__back) value_type(v);
				__adv_push();
			}
			constexpr void __push(value_type&& v)
			requires(__move_assign) {
				*__back = std::move(v);
				__adv_push();
			}
			constexpr void __push(value_type&& v)
			requires(__move_construct) {
				new(__back) value_type(std::move(v));
				__adv_push();
			}
			constexpr size_type __rem() const noexcept
			{
				difference_type diff 	= __back - __curr;
				if(diff < 0Z) diff 		*= -1Z;
				return static_cast<size_type>(diff);
			}
			constexpr size_type __pos_of(const_pointer p) const noexcept
			{
				difference_type diff	= p - __curr;
				if(!diff) return 0UZ;
				size_type cap 			= __capacity();
				return static_cast<size_type>((cap + diff) % cap);
			}
			template<typename ... Args> requires(std::constructible_from<value_type, Args...>)
			constexpr pointer __emplace(Args&& ... args)
			{
				pointer result	= new(__back) value_type(std::forward<Args>(args)...);
				__adv_push();
				return result;
			}
			template<std::output_iterator<value_type> IT, std::sentinel_for<IT> ET>
			constexpr size_type __sink(IT start, ET fin)
			noexcept(noexcept(copy_or_move(std::declval<pointer>(), std::declval<pointer>(), std::declval<size_type>())))
			{
				if constexpr(std::contiguous_iterator<IT>)
				{
					size_type n					= std::min(static_cast<size_type>(std::ranges::distance(start, fin)), this->__length());
					if(__back < __curr)
					{
						size_type back_part		= static_cast<size_type>(__rmax - __curr);
						size_type front_part	= static_cast<size_type>(n - back_part);
						copy_or_move(std::to_address(start), __curr, back_part);
						copy_or_move(std::to_address(start) + back_part, __base, front_part);
					}
					else copy_or_move(std::to_address(start), __curr, n);
					__adv_pop(n);
					return n;
				}
				else
				{
					size_type result{};
					for(IT i	= start; i != fin; ++i, result++)
						*i		= std::move(__pop());
					return result;
				}
			}
			struct __circular_iterator
			{
				typedef std::bidirectional_iterator_tag iterator_category;
				typedef std::bidirectional_iterator_tag iterator_concept;
				typedef circular_queue::value_type value_type;
				typedef circular_queue::difference_type difference_type;
				typedef circular_queue::size_type size_type;
				typedef circular_queue::pointer pointer;
				typedef circular_queue::reference reference;
				friend class circular_queue::iterator_bound;
			private:
				__circular_buffer const* __buff;
				pointer __pos;
				constexpr size_type __offset() const noexcept { return __buff ? __buff->__pos_of(__pos) : 0UZ; }
				constexpr pointer __next() const noexcept
				{
					if(__buff)
					{
						if(__buff->__capacity() && __pos != __buff->__back)
							return (__pos + 1 < __buff->__rmax) ? __pos + 1 : __buff->__base;
						else return __buff->__back;
					}
					return nullptr;
				}
				constexpr pointer __advance_pos(size_type n) const noexcept
				{
					if(__buff)
					{
							if(n == 1UZ) return __next();
						else if(__buff->__capacity() && __pos != __buff->__back)
							return __buff->__adv_ptr(__pos, n);
						else return __buff->__back;
					}
					return nullptr;
				}
				constexpr pointer __backtrack_pos(size_type n) const noexcept
				{
					if(__buff)
					{
						size_type o		= this->__offset();
						if(n >= o)
							return __buff->__curr;
						size_type noffs	= static_cast<size_type>(o - n);
						return __buff->__get_ptr(noffs);
					}
					return nullptr;
				}
			public:
				constexpr __circular_iterator() noexcept : __buff(), __pos() {}
				constexpr __circular_iterator(__circular_buffer const& b, pointer p) noexcept : __buff(std::addressof(b)), __pos(p) {}
				constexpr __circular_iterator(__circular_iterator const& that) noexcept : __buff(that.__buff), __pos(that.__pos) {}
				constexpr __circular_iterator(__circular_iterator&& that) noexcept : __buff(that.__buff), __pos(that.__pos) {}
				constexpr __circular_iterator& operator=(__circular_iterator const& that) noexcept { this->__pos = that.__pos; return *this; }
				constexpr __circular_iterator& operator=(__circular_iterator&& that) noexcept { this->__pos = that.__pos; return *this; }
				constexpr __circular_iterator& operator=(iterator_bound const&) noexcept;
				constexpr __circular_iterator& operator=(iterator_bound&&) noexcept;
				constexpr pointer operator->() const noexcept { return __pos; }
				constexpr reference operator*() const noexcept { return *__pos; }
				constexpr reference operator[](size_type n) const noexcept { return *(__advance_pos(n)); }
				constexpr __circular_iterator operator++(int) noexcept { __circular_iterator that(*this); __pos = __next(); return that; }
				constexpr __circular_iterator& operator++() noexcept { __pos = __next(); return *this; }
				constexpr __circular_iterator operator--(int) noexcept { __circular_iterator that(*this); __pos = __backtrack_pos(1UZ); return that; }
				constexpr __circular_iterator& operator--() noexcept { __pos = __backtrack_pos(1UZ); return *this; }
				constexpr __circular_iterator operator+(difference_type n) const noexcept { return __circular_iterator(__buff, n > 0 ? __advance_pos(static_cast<size_type>(n)) : __backtrack_pos(static_cast<size_type>(-n))); }
				constexpr __circular_iterator& operator+=(difference_type n) noexcept { return (*this = *this + n); }
				constexpr __circular_iterator operator-(difference_type n) const noexcept { return *this + (-n); }
				constexpr __circular_iterator& operator-=(difference_type n) noexcept { return (*this = *this - n); }
				constexpr bool operator==(__circular_iterator const& that) const noexcept { return this->__pos == that.__pos; }
				constexpr difference_type operator-(__circular_iterator const& that) noexcept { return this->__offset() - that.__offset(); }
				friend constexpr std::strong_ordering operator<=>(__circular_iterator const& __this, __circular_iterator const& __that) noexcept { return __this.__offset() <=> __that.__offset(); }
			};
			constexpr __circular_iterator __beg() const noexcept { return __circular_iterator(*this, __curr); }
			constexpr __circular_iterator __end() const noexcept { return __circular_iterator(*this, __back); }
			constexpr __circular_iterator __iter(pointer p) const noexcept { return __circular_iterator(*this, p); }
		} __buffer;
		consteval static bool __nt_swap() noexcept { return noexcept(std::declval<__circular_buffer&>().__swap(std::declval<__circular_buffer&>())); }
		template<std::output_iterator<value_type> IT, std::sentinel_for<IT> ET> consteval bool __nt_sink() noexcept { return noexcept(__buffer.__sink(std::declval<IT>(), std::declval<ET>())); }
		template<std::ranges::output_range<value_type> RT> consteval bool __nt_get() noexcept { return __nt_sink<std::ranges::iterator_t<RT>, std::ranges::sentinel_t<RT>>(); }
	public:
		typedef typename __circular_buffer::__circular_iterator iterator;
		typedef std::basic_const_iterator<iterator> const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::basic_const_iterator<reverse_iterator> const_reverse_iterator;
		class iterator_bound
		{
			pointer __bound_value;
		public:
			typedef circular_queue::iterator::difference_type difference_type;
		private:
			constexpr difference_type __sub_from(circular_queue::iterator const& that) const noexcept { return that.__pos - this->__bound_value; }
			constexpr difference_type __sub(circular_queue::iterator const& that) const noexcept { return this->__bound_value - that.__pos; }
		public:
			constexpr iterator_bound() noexcept = default;
			constexpr iterator_bound(pointer value) noexcept : __bound_value(value) {}
			constexpr iterator_bound(circular_queue::iterator const& i) noexcept : __bound_value(i.__pos) {}
			constexpr operator pointer() const noexcept { return __bound_value; }
			constexpr bool operator==(circular_queue::iterator const& that) const noexcept { return this->__bound_value == that.__pos; }
			constexpr bool operator==(circular_queue::reverse_iterator const& that) const noexcept { return (*this == that.base()); }
			friend constexpr difference_type operator-(iterator_bound const& __this, circular_queue::iterator const& __that) noexcept { return __this.__sub(__that); }
			friend constexpr difference_type operator-(circular_queue::iterator const& __this, iterator_bound const& __that) noexcept { return __that.__sub_from(__this); }
			friend constexpr difference_type operator-(iterator_bound const& __this, circular_queue::reverse_iterator const& __that) noexcept { return __this.__sub_from(__that.base()); }
			friend constexpr difference_type operator-(circular_queue::reverse_iterator const& __this, iterator_bound const& __that) noexcept { return __that.__sub(__this.base()); }
			friend constexpr std::strong_ordering operator<=>(iterator_bound const& __this, circular_queue::iterator const& __that) noexcept { return (__this - __that) <=> difference_type(0Z); }
			friend constexpr std::strong_ordering operator<=>(iterator_bound const& __this, circular_queue::reverse_iterator const& __that) noexcept { return (__this - __that) <=> difference_type(0Z); }
			friend constexpr std::strong_ordering operator<=>(circular_queue::iterator const& __this, iterator_bound const& __that) noexcept { return (__this - __that) <=> difference_type(0Z); }
			friend constexpr std::strong_ordering operator<=>(circular_queue::reverse_iterator const& __this, iterator_bound const& __that) noexcept { return (__this - __that) <=> difference_type(0Z); }
		};
		constexpr ~circular_queue() noexcept { __buffer.__destroy(); }
		constexpr circular_queue() noexcept(noexcept(allocator_type())) = default;
		constexpr circular_queue(size_type n) : __buffer(n) {}
		constexpr iterator begin() noexcept { return __buffer.__beg(); }
		constexpr const_iterator cbegin() const noexcept { return __buffer.__beg(); }
		constexpr const_iterator begin() const noexcept { return cbegin(); }
		constexpr iterator_bound end() noexcept { return __buffer.__end(); }
		constexpr iterator_bound cend() const noexcept { return __buffer.__end(); }
		constexpr iterator_bound end() const noexcept { return cend(); }
		constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(__buffer.__end()); }
		constexpr const_reverse_iterator crbegin() const noexcept { return reverse_iterator(__buffer.__end()); }
		constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
		constexpr iterator_bound rend() noexcept { return __buffer.__beg(); }
		constexpr iterator_bound crend() const noexcept { return __buffer.__beg(); }
		constexpr iterator_bound rend() const noexcept { return crend(); }
		constexpr size_type capacity() const noexcept { return __buffer.__capacity(); }
		constexpr size_type size() const noexcept { return __buffer.__length(); }
		constexpr size_type length() const noexcept { return __buffer.__length(); }
		constexpr size_type available_capacity() const noexcept { return __buffer.__rem(); }
		constexpr operator bool() const noexcept { return static_cast<bool>(__buffer.__length()); }
		constexpr bool empty() const noexcept { return !__buffer.__length(); }
		constexpr void create_if_empty(size_type n) { if(!__buffer.__base) __buffer.__create(n); }
		constexpr void push(value_type const& v) requires(__copy_assign || __copy_construct) { create_if_empty(4UZ); __buffer.__push(v); }
		constexpr void push(value_type&& v) requires(__move_assign || __move_construct) { create_if_empty(4UZ); __buffer.__push(std::move(v)); }
		constexpr value_type pop() { return __buffer.__pop(); }
		constexpr reference peek() noexcept { return __buffer.__peek(); }
		constexpr const_reference peek() const noexcept { return __buffer.__peek(); }
		constexpr reference peek_back() noexcept { return __buffer.__peek_back(); }
		constexpr const_reference peek_back() const noexcept { return __buffer.__peek_back(); }
		constexpr reference front() noexcept { return peek(); }
		constexpr const_reference front() const noexcept { return peek(); }
		constexpr reference back() noexcept { return peek_back(); }
		constexpr const_reference back() const noexcept { return peek_back(); }
		constexpr void bump(size_type n) noexcept { __buffer.__adv_pop(n); }
		constexpr void flush() noexcept requires(__trivial) { __buffer.__flush(); }
		constexpr void swap(circular_queue& that) noexcept(__nt_swap()) { this->__buffer.__swap(that.__buffer); }
		template<std::ranges::output_range<value_type> RT>
		constexpr size_type get(RT&& r) noexcept(__nt_get<RT>()) { return __buffer.__sink(std::ranges::begin(r), std::ranges::end(r)); }
		template<std::output_iterator<value_type> IT, std::sentinel_for<IT> ET>
		constexpr size_type get(IT first, ET last) noexcept(__nt_sink<IT, ET>()) { return __buffer.__sink(first, last); }
		constexpr void clear() noexcept(__nothrow_zero)
		{
			__buffer.__destroy();
			array_zero(__buffer.__base, __buffer.__capacity());
			__buffer.__rewind();
		}
		template<typename ... Args> requires(std::constructible_from<value_type, Args...>)
		constexpr iterator emplace(Args&& ... args)
		{
			create_if_empty(4UZ);
			pointer result	= __buffer.__emplace(std::forward<Args>(args)...);
			return __buffer.__iter(result);
		}
	};
	template<typename T, std::allocator_object<T> A>
	constexpr typename circular_queue<T, A>::__circular_buffer::__circular_iterator&
	circular_queue<T, A>::__circular_buffer::__circular_iterator::operator=(circular_queue<T, A>::iterator_bound const& that) noexcept {
		this->__pos	= that;
		return *this;
	}
	template<typename T, std::allocator_object<T> A>
	constexpr typename circular_queue<T, A>::__circular_buffer::__circular_iterator&
	circular_queue<T, A>::__circular_buffer::__circular_iterator::operator=(circular_queue<T, A>::iterator_bound&& that) noexcept {
		this->__pos	= that;
		return *this;
	}
}
#endif