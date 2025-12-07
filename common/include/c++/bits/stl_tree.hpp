#ifndef __STL_TREE
#define __STL_TREE
/*
 * The most important thing eco-activists and programmers have in common is that we both love trees.
 * — Someone, somewhere, at some time, probably
 */
#include <bits/move.h>
#include <memory>
#include <bits/iterator_concepts.hpp>
#include <bits/stl_pair.hpp>
#include <bits/stl_algobase.hpp>
#include <bits/aligned_buffer.hpp>
namespace std
{
	template<typename CT, typename T, typename U = T> concept __valid_comparator = is_default_constructible_v<CT> && requires { { declval<CT>()(declval<T>(), declval<U>()) } -> __detail::__boolean_testable; { declval<CT>()(declval<U>(), declval<T>()) } -> __detail::__boolean_testable; };
	template<typename U, typename KT, typename CT, typename VT> concept __tree_searchable	= __valid_comparator<CT, VT, U> || is_convertible_v<U, KT>;
	// Couldn't not make the Les Mis reference
	enum node_color
	{
		RED,	// the blood of angry men; the world about to dawn; I feel my heart on fire; the color of desire
		BLACK	// the dark of ages past; the night that ends at last; my world when she's not there; the color of despair
	};
	enum node_direction
	{
		// sliiiide to the...
		LEFT,
		// sliiiide to the...
		RIGHT
		// two hops this time
		// cha-cha now y'all
	};
	struct __node_base
	{
		typedef __node_base* __pointer;
		typedef __node_base const* __const_pointer;
		node_color __my_color;
		__pointer __my_parent;
		__pointer __my_left;
		__pointer __my_right;
		static __pointer __min(__pointer x);
		static __const_pointer __min(__const_pointer x);
		static __pointer __max(__pointer x);
		static __const_pointer __max(__const_pointer x);
	};
	constexpr __node_base* __increment_node(__node_base* x)
	{
		if(x->__my_right) { x = x->__my_right; while(x->__my_left) x = x->__my_left; }
		else
		{
			__node_base* y = x->__my_parent;
			while(x == y->__my_right) { x = y; y = y->__my_parent; }
			if(x->__my_right != y) x = y;
		}
		return x;
	}
	constexpr __node_base const* __increment_node(__node_base const* x)
	{
		if(x->__my_right) { x = x->__my_right; while(x->__my_left) x = x->__my_left; }
		else
		{
			__node_base* y = x->__my_parent;
			while(x == y->__my_right) { x = y; y = y->__my_parent; }
			if(x->__my_right != y) x = y;
		}
		return x;
	}
	constexpr __node_base* __decrement_node(__node_base* x)
	{
		if(x->__my_color == RED && x->__my_parent->__my_parent == x) x = x->__my_right;
		else if(x->__my_left) { x = x->__my_left; while(x->__my_right) x = x->__my_right; }
		else
		{
			__node_base* y = x->__my_parent;
			while(x == y->__my_left) { x = y; y = y->__my_parent; }
			x = y;
		}
		return x;
	}
	constexpr __node_base const* __decrement_node(__node_base const* x)
	{
		if(x->__my_color == RED && x->__my_parent->__my_parent == x) x = x->__my_right;
		else if(x->__my_left) { x = x->__my_left; while(x->__my_right) x = x->__my_right; }
		else
		{
			__node_base* y = x->__my_parent;
			while(x == y->__my_left) { x = y; y = y->__my_parent; }
			x = y;
		}
		return x;
	}
	template<typename A>
	struct __tree_base
	{
		// This node's parent is the root node; its left is the min node; its right is the max node

		struct __trunk_node : __node_base, A
		{
			constexpr void __on_assign() noexcept { if(this->__my_parent) this->__my_parent->__my_parent = this; }
			constexpr __trunk_node() noexcept(noexcept(A())) : __node_base(RED, nullptr, this, this), A() {}
			constexpr __trunk_node(__trunk_node const& that) noexcept(std::is_nothrow_copy_constructible_v<A>) : __node_base(that), A(that) { __on_assign(); }
			constexpr __trunk_node(__trunk_node&& that) noexcept(std::is_nothrow_move_constructible_v<A>) : __node_base(move(that)), A(move(that)) { __on_assign(); that.__reset(); }
			constexpr __trunk_node& operator=(__trunk_node const& that) noexcept(std::is_nothrow_copy_assignable_v<A> || !__has_copy_propagate<A>)
			{
				this->__my_color			= that.__my_color;
				this->__my_parent			= that.__my_parent;
				this->__my_left				= that.__my_left;
				this->__my_right			= that.__my_right;
				if constexpr(__has_copy_propagate<A>)
					*static_cast<A*>(this)	= that;
				this->__on_assign();
				return *this;
			}
			constexpr __trunk_node& operator=(__trunk_node&& that) noexcept(std::is_nothrow_move_assignable_v<A> || !__has_move_propagate<A>)
			{
				this->__my_color			= that.__my_color;
				this->__my_parent			= that.__my_parent;
				this->__my_left				= that.__my_left;
				this->__my_right			= that.__my_right;
				if constexpr(__has_move_propagate<A>)
					*static_cast<A*>(this)	= std::move(that);
				this->__on_assign();
				that.__reset();
				return *this;
			}
			constexpr void __reset() noexcept
			{
				__my_color	= RED;
				__my_parent	= nullptr;
				__my_left	= this;
				__my_right	= this;
			}
		} __trunk;
		size_t __count;
		constexpr void __reset() noexcept {
			__trunk.__reset();
			__count	= 0;
		}
		constexpr __tree_base() noexcept(noexcept(A())) :  __trunk(), __count(0) {}
		constexpr ~__tree_base() { if(__trunk.__my_parent) __trunk.__my_parent->__my_parent = nullptr; }
		constexpr void __copy(__tree_base const& that) noexcept {
			__trunk				= that.__trunk;
			__count				= that.__count;
		}
		constexpr void __move(__tree_base&& that) noexcept
		{
			__trunk				= std::move(that.__trunk);
			__count				= that.__count;
			that.__count		= 0UZ;
		}
		constexpr void __swap(__tree_base& that) noexcept
		{
			if constexpr(__has_swap_propagate<A>)
				std::swap<A>(*this, that);
			__tree_base tmp(*this);
			this->__copy(that);
			that.__copy(tmp);
			tmp.__reset();
		}
		constexpr __tree_base(__tree_base&& that) noexcept : __trunk(std::move(that.__trunk)), __count(that.__count) { that.__count = 0UZ; }
		constexpr __tree_base(__tree_base const& that) noexcept : __trunk(that.__trunk), __count(that.__count) {}
		constexpr __tree_base& operator=(__tree_base const& that) noexcept { __copy(that); return *this; }
		constexpr __tree_base& operator=(__tree_base&& that) noexcept { __move(std::move(that)); return *this; }
	};
	template<typename T>
	struct __node : public __node_base
	{
		typedef __node<T>* __link_ptr;
	private:
		::__impl::__aligned_buffer<T> __my_data;
	public:
		constexpr __node() : __my_data() {}
		constexpr T* __get_ptr() { return __my_data.__get_ptr(); }
		constexpr T const* __get_ptr() const { return __my_data.__get_ptr(); }
		constexpr T& __get_ref() { return *__get_ptr(); }
		constexpr T const& __get_ref() const { return *__get_ptr(); }
	};
	template<typename T>
	struct __tree_iterator
	{
		typedef T value_type;
		typedef add_lvalue_reference_t<T> reference;
		typedef add_pointer_t<T> pointer;
		typedef ptrdiff_t difference_type;
	private:
		typedef __tree_iterator<T> __iterator_type;
		typedef __node_base::__pointer __base_ptr;
		typedef __node<T>* __node_ptr;
		__base_ptr __my_node;
	public:
		constexpr __tree_iterator() noexcept : __my_node() {}
		constexpr explicit __tree_iterator(__base_ptr x) noexcept : __my_node(x) {}
		extension constexpr __node_ptr get_node() const noexcept { return static_cast<__node_ptr>(__my_node); }
		constexpr pointer base() const noexcept { return get_node()->__get_ptr(); }
		constexpr pointer operator->() const noexcept { return base(); }
		constexpr reference operator*() const noexcept { return *base(); }
		constexpr __iterator_type& operator++() noexcept { __my_node = __increment_node(__my_node); return *this; }
		constexpr __iterator_type operator++(int) noexcept { __iterator_type tmp = *this; __my_node = __increment_node(__my_node); return tmp; }
		constexpr __iterator_type& operator--() noexcept { __my_node = __decrement_node(__my_node); return *this; }
		constexpr __iterator_type operator--(int) noexcept { __iterator_type tmp = *this; __my_node = __decrement_node(__my_node); return tmp; }
		friend constexpr bool operator==(__tree_iterator<T> const& x, __tree_iterator<T> const& y) { return x.__my_node == y.__my_node; }
		constexpr operator bool() const { return __my_node != nullptr; }
	};
	template<typename T>
	struct __tree_const_iterator
	{
		typedef T value_type;
		typedef add_lvalue_reference_t<add_const_t<T>> reference;
		typedef add_pointer_t<add_const_t<T>> pointer;
		typedef ptrdiff_t difference_type;
	private:
		typedef __tree_iterator<T> __iterator_type;
		typedef __tree_const_iterator<T> __const_iterator_type;
		typedef __node_base::__const_pointer __base_ptr;
		typedef __node<T> const* __node_ptr;
		__base_ptr __my_node;
	public:
		constexpr __tree_const_iterator() noexcept : __my_node() {}
		constexpr explicit __tree_const_iterator(__base_ptr x) noexcept : __my_node(x) {}
		constexpr __tree_const_iterator(__iterator_type const& i) noexcept : __my_node{ i.get_node() } {}
		extension constexpr __node_ptr get_node() const noexcept { return static_cast<__node_ptr>(__my_node); }
		constexpr pointer base() const noexcept { return get_node()->__get_ptr(); }
		constexpr pointer operator->() const noexcept { return base(); }
		constexpr reference operator*() const noexcept { return *base(); }
		constexpr __const_iterator_type& operator++() noexcept { __my_node = __increment_node(__my_node); return *this; }
		constexpr __const_iterator_type operator++(int) noexcept { __const_iterator_type tmp = *this; __my_node = __increment_node(__my_node); return tmp; }
		constexpr __const_iterator_type& operator--() noexcept { __my_node = __decrement_node(__my_node); return *this; }
		constexpr __const_iterator_type operator--(int) noexcept { __const_iterator_type tmp = *this; __my_node = __decrement_node(__my_node); return tmp; }
		friend constexpr bool operator==(__tree_const_iterator<T> const& x, __tree_const_iterator<T> const& y) noexcept { return x.__my_node == y.__my_node; }
		constexpr operator bool() const noexcept { return __my_node != nullptr; }
	};
	[[gnu::nonnull]] void __insert_and_rebalance(const node_direction dir, __node_base* x, __node_base* p, __node_base& trunk);
	[[gnu::nonnull, gnu::returns_nonnull]] __node_base* __rebalance_for_erase(__node_base* const z, __node_base& trunk);
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	class __rb_tree : protected __tree_base<typename A::template rebind<__node<T>>::other>
	{
		using __rebind_alloc = typename A::template rebind<__node<T>>;
	protected:
		typedef __node_base* __b_ptr;
		typedef __node_base const* __cb_ptr;
		typedef __node<T>* __link;
		typedef __node<T> const* __const_link;
		typedef pair<__link, __link> __pos_pair;
		typedef pair<__link, bool> __res_pair;
		typedef __tree_iterator<T> __iterator;
		typedef __tree_const_iterator<T> __const_iterator;
		typedef T __value_type;
		typedef typename __rebind_alloc::other __allocator_type;
		typedef CP __compare_type;
		typedef __tree_base<typename A::template rebind<__node<T>>::other> __base;
		constexpr __allocator_type const& __get_alloc() const noexcept { return this->__trunk; }
		template<typename U> requires(__valid_comparator<CP, T, U>) constexpr __pos_pair __pos_for_unique(U const& u);
		template<typename U> requires(__valid_comparator<CP, T, U>) constexpr __pos_pair __pos_for_equal(U && u);
		template<typename U> requires(__valid_comparator<CP, T, U>) constexpr __pos_pair __insert_unique_hint_pos(__const_link hint, U const& u);
		template<typename ... Args> requires(constructible_from<T, Args...>) constexpr __res_pair __emplace_unique(Args&& ... args);
		template<typename ... Args> requires(constructible_from<T, Args...>) constexpr __res_pair __hint_emplace_unique(__const_link hint, Args&& ... args);
		template<typename U> requires(__valid_comparator<CP, T, U>) __const_link __lower_bound(__const_link x, __const_link y, U const& u) const noexcept;
		template<typename U> requires(__valid_comparator<CP, T, U>) __const_link __upper_bound(__const_link x, __const_link y, U const& u) const noexcept;
		template<typename U> requires(__valid_comparator<CP, T, U>) __link __lower_bound(__link x, __link y, U const& u) noexcept;
		template<typename U> requires(__valid_comparator<CP, T, U>) __link __upper_bound(__link x, __link y, U const& u) noexcept;
		template<std::convertible_to<T> U> constexpr __link __hint_insert_unique(__const_link hint, U const& u);
		template<std::convertible_to<T> U> constexpr __link __hint_insert_unique(__const_link hint, U && u);
		template<std::convertible_to<T> U> constexpr __link __insert_lower(__b_ptr p, U&& u);
		template<std::convertible_to<T> U> constexpr __link __insert_lower_equal(U&& u);
		template<std::convertible_to<T> U> constexpr __res_pair __insert_unique(U const& u);
		template<std::convertible_to<T> U> constexpr __res_pair __insert_unique(U && u);
		template<typename ... Args> requires(constructible_from<T, Args...>) constexpr __link __construct_node(Args&& ... args);
		constexpr __link __insert_node(__b_ptr x, __b_ptr p, __link l);
		constexpr static __link __left_of(__link x) noexcept { return static_cast<__link>(x->__my_left); }
		constexpr static __link __right_of(__link x) noexcept { return static_cast<__link>(x->__my_right); }
		constexpr static __const_link __left_of(__const_link x) noexcept { return static_cast<__const_link>(x->__my_left); }
		constexpr static __const_link __right_of(__const_link x) noexcept { return static_cast<__const_link>(x->__my_right); }
		constexpr static __b_ptr __mini(__b_ptr x) noexcept { return __node_base::__min(x); }
		constexpr static __b_ptr __maxi(__b_ptr x) noexcept { return __node_base::__max(x); }
		constexpr static __cb_ptr __mini(__cb_ptr x) noexcept { return __node_base::__min(x); }
		constexpr static __cb_ptr __maxi(__cb_ptr x) noexcept { return __node_base::__max(x); }
		constexpr bool __compare(__b_ptr p, __b_ptr q) { return CP()(static_cast<__link>(p)->__get_ref(), static_cast<__link>(q)->__get_ref()); }
		template<typename U> requires(__valid_comparator<CP, T, U>) constexpr bool __compare_r(__cb_ptr p, U const& u) const { return CP()(static_cast<__const_link>(p)->__get_ref(), u); }
		template<typename U> requires(__valid_comparator<CP, T, U>) constexpr bool __compare_l(U const& u, __cb_ptr p) const { return CP()(u, static_cast<__const_link>(p)->__get_ref()); }
		constexpr __link __get_root() noexcept { return static_cast<__link>(this->__trunk.__my_parent); }
		constexpr __link __end() noexcept { return addr_t(addressof(this->__trunk)); }
		constexpr __const_link __get_root() const noexcept { return static_cast<__const_link>(this->__trunk.__my_parent); }
		constexpr __const_link __end() const noexcept { return addr_t(addressof(this->__trunk)); }
		constexpr __b_ptr& __leftmost() noexcept { return this->__trunk.__my_left; }
		constexpr __b_ptr& __rightmost() noexcept { return this->__trunk.__my_right; }
		constexpr __link __l_begin() noexcept {  return static_cast<__link>(this->__trunk.__my_left); }
		constexpr __link __l_rightmost() noexcept {  return static_cast<__link>(this->__trunk.__my_right); }
		constexpr __iterator __begin() noexcept { return __iterator(__l_begin()); }
		constexpr __const_link __l_begin() const noexcept { return static_cast<__const_link>(this->__trunk.__my_left); }
		constexpr __const_link __l_rightmost() const noexcept { return static_cast<__const_link>(this->__trunk.__my_right); }
		constexpr __const_iterator __begin() const noexcept { return __const_iterator(__l_begin()); }
		template<std::convertible_to<T> U> constexpr __link __insert(__b_ptr x, __b_ptr p, U const& u) { return __insert_node(x, p, __construct_node(u)); }
		template<std::convertible_to<T> U> constexpr __link __insert(__b_ptr x, __b_ptr p, U&& u) { return __insert_node(x, p, __construct_node(forward<U>(u)));}
		template<std::convertible_to<T> U> constexpr __link __insert_equal(U && u) { __pos_pair p = __pos_for_equal(u); return __insert(p.first, p.second, forward<U>(u)); }
		template<matching_input_iterator<T> IT> constexpr void __insert_range_unique(IT st, IT ed) { for(IT i = st; i != ed; i++) __insert_unique(*i); }
		template<typename U> requires(__valid_comparator<CP, T, U>) constexpr __link __find_node(U const& u) noexcept;
		template<typename U> requires(__valid_comparator<CP, T, U>) constexpr __const_link __find_node(U const& u) const noexcept;
		template<typename U> requires(__valid_comparator<CP, T, U>) constexpr bool __contains(U const& u) const noexcept { return (this->__find_node(u) != this->__end()); }
		constexpr void __recursive_destroy(__link n);
		constexpr void __clear();
		__b_ptr __erase_node(__b_ptr n);
		__b_ptr __erase_nodes(__b_ptr first, __b_ptr last);
		constexpr void __destroy_node(__b_ptr n);
		__rb_tree& __copy_assign(__rb_tree const& that);
		__rb_tree& __move_assign(__rb_tree&& that);
	public:
		constexpr size_t size() const noexcept { return this->__count; }
		constexpr bool empty() const noexcept { return !this->size(); }
		constexpr ~__rb_tree() { __recursive_destroy(__get_root()); }
		constexpr __rb_tree() : __base() {}
		template<matching_input_iterator<T> IT> constexpr __rb_tree(IT st, IT ed) : __rb_tree() { this->__insert_range_unique(st, ed); }
		constexpr __rb_tree(__rb_tree const& that) : __rb_tree(__const_iterator(that.__l_begin()), __const_iterator(that.__end())) {}
		constexpr __rb_tree(__rb_tree&& that) : __base(forward<__base>(that)) {}
		constexpr __rb_tree& operator=(__rb_tree const& that) { return __copy_assign(that); }
		constexpr __rb_tree& operator=(__rb_tree&& that) { return __move_assign(move(that)); }
	};
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename... Args>
	requires(constructible_from<T, Args...>)
	constexpr typename __rb_tree<T, CP, A>::__link __rb_tree<T, CP, A>::__construct_node(Args&& ... args)
	{
		__link l		= construct_at(this->__trunk.allocate(1UL));
		construct_at(l->__get_ptr(), forward<Args>(args)...);
		l->__my_color	= RED;
		return l;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	constexpr typename __rb_tree<T, CP, A>::__link __rb_tree<T, CP, A>::__insert_node(__b_ptr x, __b_ptr p, __link l)
	{
		bool left	= (x || p == __end() || __compare_l(l->__get_ref(), p));
		__insert_and_rebalance(left ? LEFT : RIGHT, l, p, this->__trunk);
		this->__count++;
		return l;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	typename __rb_tree<T, CP, A>::__b_ptr __rb_tree<T, CP, A>::__erase_node(__b_ptr n)
	{
		__b_ptr y		= __rebalance_for_erase(n, this->__trunk);
		__b_ptr result	= __increment_node(y);
		this->__destroy_node(y);
		this->__count--;
		return result;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	typename __rb_tree<T, CP, A>::__b_ptr __rb_tree<T, CP, A>::__erase_nodes(__b_ptr first, __b_ptr last)
	{
		__b_ptr result	= __end();
		for(__b_ptr cur	= first; cur != last; ++cur)
			result		= this->__erase_node(cur);
		return result;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	constexpr void __rb_tree<T, CP, A>::__destroy_node(__b_ptr n)
	{
		if(__unlikely(!n)) return;
		__link l		= static_cast<__link>(n);
		if constexpr(!std::is_trivially_destructible_v<__value_type>)
			l->__get_ref().~__value_type();
		this->__trunk.deallocate(l, 1);
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename U>
	requires(__valid_comparator<CP, T, U>)
	constexpr typename __rb_tree<T, CP, A>::__const_link __rb_tree<T, CP, A>::__find_node(U const& u) const noexcept
	{
		__const_link result	= __lower_bound(__get_root(), __end(), u);
		if(result == __end()) return result;
		return __compare_l(u, result) ? __end() : result;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename U>
	requires(__valid_comparator<CP, T, U>)
	constexpr typename __rb_tree<T, CP, A>::__link __rb_tree<T, CP, A>::__find_node(U const& u) noexcept
	{
		__link result	= __lower_bound(__get_root(), __end(), u);
		if(result == __end()) return result;
		return __compare_l(u, result) ? __end() : result;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename U>
	requires(__valid_comparator<CP, T, U>)
	constexpr typename __rb_tree<T, CP, A>::__pos_pair __rb_tree<T, CP, A>::__pos_for_unique(U const& u)
	{
		__link x	= __get_root();
		__link y	= __end();
		bool comp	= true;
		while(x)
		{
			y		= x;
			comp	= __compare_l(u, x);
			x		= comp ? __left_of(x) : __right_of(x);
		}
		__iterator j(y);
		if(comp)
		{
			if(j == __begin())
				return __pos_pair(x, y);
			else --j;
		}
		if(__compare_r(j.get_node(), u)) return __pos_pair(x, y);
		return __pos_pair(static_cast<__link>(j.get_node()), nullptr);
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename U>
	requires(__valid_comparator<CP, T, U>)
	constexpr typename __rb_tree<T, CP, A>::__pos_pair __rb_tree<T, CP, A>::__pos_for_equal(U && u)
	{
		__link x    = __get_root();
		__b_ptr y   = __end();
		while(x) {
			y		= x;
			x		= !__compare_r(x, u) ? __left_of(x) : __right_of(x);
		}
		return __pos_pair(x, y);
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename U>
	requires(__valid_comparator<CP, T, U>)
	constexpr typename __rb_tree<T, CP, A>::__pos_pair __rb_tree<T, CP, A>::__insert_unique_hint_pos(__const_link hint, U const& u)
	{
		__link pos	= const_cast<__link>(hint);
		if(!hint || pos == __end())
		{
			if(this->__count > 0 && __compare_r(__rightmost(), u))
				return __pos_pair(nullptr, __l_rightmost());
			else return __pos_for_unique(u);
		}
		else if(__compare_l(u, pos))
		{
			__link before	= static_cast<__link>(__decrement_node(pos));
			if(pos == __l_begin()) return __pos_pair(__l_begin(), __l_begin());
			else if(__compare_r(before, u))
			{
				if(!before->__my_right)
					return __pos_pair(nullptr, before);
				else return __pos_pair(pos, pos);
			}
			else return __pos_for_unique(u);
		}
		else if(__compare_r(pos, u))
		{
			__link after = static_cast<__link>(__increment_node(pos));
			if(pos == __l_rightmost()) return __pos_pair(nullptr, __l_rightmost());
			else if(__compare_l(u, after))
			{
				if(!pos->__my_right)
					return __pos_pair(nullptr, pos);
				else return __pos_pair(after, after);
			}
			else return __pos_for_unique(u);
		}
		return __pos_pair(pos, nullptr);
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename ... Args>
	requires(constructible_from<T, Args...>)
	constexpr typename __rb_tree<T, CP, A>::__res_pair __rb_tree<T, CP, A>::__emplace_unique(Args&& ...args)
	{
		__link l        = __construct_node(forward<Args>(args)...);
		__pos_pair r    = __pos_for_unique(l->__get_ref());
		if(r.second) return __res_pair(__insert_node(r.first, r.second, l), true);
		else __destroy_node(l);
		return __res_pair(r.first, false);
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename... Args>
	requires(constructible_from<T, Args...>)
	constexpr typename __rb_tree<T, CP, A>::__res_pair __rb_tree<T, CP, A>::__hint_emplace_unique(__const_link hint, Args&& ... args)
	{
		__link l        = __construct_node(forward<Args>(args)...);
		__pos_pair r    = __insert_unique_hint_pos(hint, l->__get_ref());
		if(r.second) return __res_pair(__insert_node(r.first, r.second, l), true);
		else __destroy_node(l);
		return __res_pair(r.first, false);
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<std::convertible_to<T> U>
	constexpr typename __rb_tree<T, CP, A>::__res_pair __rb_tree<T, CP, A>::__insert_unique(U const& u)
	{
		__pos_pair p	= __insert_unique_hint_pos(nullptr, u);
		if(p.second) return __res_pair(__insert(p.first, p.second, u), true);
		return __res_pair(p.first, false);
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<std::convertible_to<T> U>
	constexpr typename __rb_tree<T, CP, A>::__link __rb_tree<T, CP, A>::__insert_lower(__b_ptr p, U&& u)
	{
		bool left	= (p == __end() || __compare_l(u, p));
		__link l	= __construct_node(forward<U>(u));
		__insert_and_rebalance(left ? LEFT : RIGHT, l, p, this->__trunk);
		this->__count++;
		return l;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<std::convertible_to<T> U>
	constexpr typename __rb_tree<T, CP, A>::__link __rb_tree<T, CP, A>::__insert_lower_equal(U&& u)
	{
		__link x	= __get_root(), y = __end();
		while(x) { y = x; x = !__compare_r(x, u) ? __left_of(x) : __right_of(x); }
		return __insert_lower(y, forward<U>(u));
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<std::convertible_to<T> U>
	constexpr typename __rb_tree<T, CP, A>::__res_pair __rb_tree<T, CP, A>::__insert_unique(U&& u)
	{
		__pos_pair p	= __insert_unique_hint_pos(nullptr, u);
		if(p.second) return __res_pair(__insert(p.first, p.second, forward<U>(u)), true);
		return __res_pair(p.first, false);
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename U>
	requires(__valid_comparator<CP, T, U>)
	typename __rb_tree<T, CP, A>::__const_link __rb_tree<T, CP, A>::__upper_bound(__const_link x, __const_link y, U const& u) const noexcept
	{
		while(x)
		{
			if(__compare_l(u, x))
				y	= x, x	= __left_of(x);
			else x	= __right_of(x);
		}
		return y;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename U>
	requires(__valid_comparator<CP, T, U>)
	typename __rb_tree<T, CP, A>::__const_link __rb_tree<T, CP, A>::__lower_bound(__const_link x, __const_link y, U const& u) const noexcept
	{
		while(x)
		{
			if(!__compare_r(x, u))
				y	= x, x	= __left_of(x);
			else x	= __right_of(x);
		}
		return y;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename U>
	requires(__valid_comparator<CP, T, U>)
	typename __rb_tree<T, CP, A>::__link __rb_tree<T, CP, A>::__upper_bound(__link x, __link y, U const& u) noexcept
	{
		while(x)
		{
			if(__compare_l(u, x))
				y	= x, x	= __left_of(x);
			else x	= __right_of(x);
		}
		return y;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<typename U>
	requires(__valid_comparator<CP, T, U>)
	typename __rb_tree<T, CP, A>::__link __rb_tree<T, CP, A>::__lower_bound(__link x, __link y, U const& u) noexcept
	{
		while(x)
		{
			if(!__compare_r(x, u))
				y	= x, x	= __left_of(x);
			else x	= __right_of(x);
		}
		return y;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<std::convertible_to<T> U>
	constexpr typename __rb_tree<T, CP, A>::__link __rb_tree<T, CP, A>::__hint_insert_unique(__const_link hint, U&& u)
	{
		__pos_pair r	= __insert_unique_hint_pos(hint, u);
		if(r.second) return __insert(r.first, r.second, forward<U>(u));
		return r.first;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	template<std::convertible_to<T> U>
	constexpr typename __rb_tree<T, CP, A>::__link __rb_tree<T, CP, A>::__hint_insert_unique(__const_link hint, U const& u)
	{
		__pos_pair r	= __insert_unique_hint_pos(hint, u);
		if(r.second) return __insert(r.first, r.second, u);
		return r.first;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	__rb_tree<T, CP, A>& __rb_tree<T, CP, A>::__copy_assign(__rb_tree const &that)
	{
		__clear();
		this->__insert_range_unique(__const_iterator(that.__l_begin()), __const_iterator(that.__end()));
		return *this;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	__rb_tree<T, CP, A>& __rb_tree<T, CP, A>::__move_assign(__rb_tree&& that)
	{
		__clear();
		this->__trunk		= move(that.__trunk);
		return *this;
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	constexpr void __rb_tree<T, CP, A>::__clear()
	{
		__recursive_destroy(__get_root());
		this->__count	= 0;
		this->__reset();
	}
	template<typename T, __valid_comparator<T> CP, allocator_object<T> A>
	constexpr void __rb_tree<T, CP, A>::__recursive_destroy(__link n)
	{
		while(n)
		{
			__recursive_destroy(__right_of(n));
			__link m	= __left_of(n);
			this->__destroy_node(n);
			n			= m;
		}
	}
}
#endif