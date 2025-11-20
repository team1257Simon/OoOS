#ifndef __POOL_ALLOC
#define __POOL_ALLOC
#include <kernel_api.hpp>
#include <kernel_mm.hpp>
#include <ext/delegate_ptr.hpp>
namespace ooos
{
	namespace __internal
	{
		constexpr size_t stbits		= __CHAR_BIT__ * sizeof(size_t);
		constexpr size_t nblocks 	= stbits;
		constexpr size_t log2ceil(size_t n) noexcept
		{
			if(__unlikely(n < 2UZ)) return 0;
			size_t result = static_cast<size_t>(stbits - (1 + __builtin_clzl(n)));
			if(n > (1UZ << result)) return result + 1UZ;
			return result;
		}
		constexpr size_t p2stairs(size_t n) noexcept
		{
			if(__unlikely(n < 2UZ))
				return 1UZ;
			return 1UZ << log2ceil(n);
		}
		struct __block_pool
		{
			block_tag* block_ptrs[nblocks];
			typedef block_tag** iterator;
			typedef block_tag* const* const_iterator;
			constexpr __block_pool() noexcept = default;
			constexpr block_tag*& operator[](size_t bsz) & noexcept { return block_ptrs[log2ceil(bsz)]; }
			constexpr block_tag* const& operator[](size_t bsz) const& noexcept { return block_ptrs[log2ceil(bsz)]; }
			constexpr iterator begin() noexcept { return block_ptrs; }
			constexpr const_iterator begin() const noexcept { return block_ptrs; }
			constexpr const_iterator cbegin() const noexcept { return block_ptrs; }
			constexpr iterator end() noexcept { return block_ptrs + stbits; }
			constexpr const_iterator end() const noexcept { return block_ptrs + stbits; }
			constexpr const_iterator cend() const noexcept { return block_ptrs + stbits; }
			inline ~__block_pool() noexcept { for(block_tag* t : *this) if(t) block_free(t); }
		};
	}
	/**
	 * Stateful allocator that uses a shared pool of memory blocks.
	 * The shared pool uses an std::ext::delegate_ptr for reference count tracking and garbage collection.
	 * The default constructor requests a new pool, using the next index available for a delegate_ptr.
	 * Copy and move semantics are inherited from delegate_ptr, which in turn mimic std::shared_ptr in most respects.
	 * Each allocation grabs a memory block big enough for a number of objects equal to the smallest power of 2 greater than or equal to the number actually requested.
	 * When a block is freed, it is placed in the shared pool if it does not already contain a block of that size. Otherwise, the block is released back to the system.
	 * Allocations will pull from the shared pool before calling the kernel MM to allocate a new block.
	 * The resize() function will attempt to resize the array in place (i.e. if the block is allocated to sufficient size) if possible.
	 * When the shared pool has no more references, all blocks still in the pool will be freed.
	 * The rebind constructor will ignore its argument and construct a new pool, since the size and alignment of the allocated blocks are non-arbitrary.
	 */
	template<typename T, size_t A = alignof(T)>
	struct pool_allocator
	{
		typedef T value_type;
		typedef std::add_pointer_t<value_type> pointer;
		typedef typename std::pointer_traits<pointer>::rebind<const value_type> const_pointer;
		typedef typename std::pointer_traits<pointer>::rebind<void> void_pointer;
		typedef typename std::pointer_traits<pointer>::rebind<const void> const_void_pointer;
		typedef std::true_type propagate_on_container_move_assignment;
		typedef decltype(sizeof(T)) size_type;
		typedef decltype(std::declval<pointer>() - std::declval<pointer>()) difference_type;
	private:
		struct __block_pool_delegate
		{
			std::ext::delegate_ptr<__internal::__block_pool> __pool;
			__block_pool_delegate() : __pool() {}
			block_tag*& operator[](size_t bsz) & noexcept { return __pool[bsz]; }
			block_tag* const& operator[](size_t bsz) const& noexcept { return __pool[bsz]; }
			constexpr bool operator==(__block_pool_delegate const& that) const noexcept { return this->__pool == that.__pool; }
			constexpr std::strong_ordering operator<=>(__block_pool_delegate const& that) const noexcept { return this->__pool <=> that.__pool; }
		} __blocks;
		void __destruct(pointer ptr, size_type n_objs) noexcept(std::is_nothrow_destructible_v<value_type>)
		{
			if constexpr(!std::is_trivially_destructible_v<value_type>)
				for(size_type i = 0; i < n_objs; i++)
					ptr[i].~T();
		}
		block_tag* __get_tag(size_type n_objs) noexcept
		{
			block_tag* t		= __blocks[n_objs];
			if(!t) return block_malloc(sizeof(value_type) * __internal::p2stairs(n_objs), A);
			__blocks[n_objs]	= nullptr;
			return t;
		}
	public:
		template<typename U,  size_t B = alignof(U)> struct rebind { typedef pool_allocator<U, B> other; };
		pool_allocator() : __blocks() {}
		pool_allocator(pool_allocator const&) noexcept					= default;
		pool_allocator(pool_allocator&&) noexcept						= default;
		pool_allocator& operator=(pool_allocator&& that) noexcept		= default;
		pool_allocator& operator=(pool_allocator const& that) noexcept { if(__builtin_expect(!(this->__blocks == that.__blocks), true)) { this->__blocks = that.__blocks; } return *this; }
		template<typename U, size_t B> requires(!std::is_same_v<T, U> || (A != B)) pool_allocator(pool_allocator<U, B> const& that) : __blocks() {}
		void deallocate(pointer p, size_type n) noexcept(std::is_nothrow_destructible_v<value_type>)
		{
			__destruct(p, n);
			block_tag* t			= locate_block(p, A);
			if(!__blocks[n] && t 	!= nullptr)
				__blocks[n]			= t;
			else aligned_free(p, A);
		}
		[[nodiscard]] pointer allocate(size_type n) noexcept(noexcept(array_zero(std::declval<pointer>(), n)))
		{
			if(block_tag* t				= __get_tag(n))
			{
				pointer result			= t->actual_start();
				array_zero(result, n);
				return result;
			}
			else return nullptr;
		}
		[[nodiscard]] pointer resize(pointer arr, size_type old, size_type target)
		noexcept(std::is_nothrow_destructible_v<value_type> && noexcept(copy_or_move(std::declval<pointer>(), arr, old)) && noexcept(array_zero(arr, target)))
		{
			pointer result				= nullptr;
			if(target)
			{
				size_type total			= target * sizeof(value_type);
				if(block_tag* t			= locate_block(arr, A))
				{
					if(t->allocated_size() >= total)
					{
						if(target < old) __destruct(arr + target, static_cast<size_type>(old - target));
						else if(target > old) __builtin_memset(std::addressof(arr[target]), 0, static_cast<size_type>(target - old) * sizeof(value_type));
						t->held_size	= total;
						return t->actual_start();
					}
				}
				result					= allocate(target);
			}
			if(result) copy_or_move(result, arr, target < old ? target : old);
			deallocate(arr, old);
			return result;
		}
	};
}
#endif