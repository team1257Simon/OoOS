#include <bits/stl_tree.hpp>
namespace std
{
	static inline __node_base*& __next(__node_base* x, const node_direction dir) { return (dir == RIGHT ? x->__my_right : x->__my_left); }
	__node_base::__pointer __node_base::__min(__pointer x) { while(x->__my_left) x = x->__my_left; return x; }
	__node_base::__const_pointer __node_base::__min(__const_pointer x) { while(x->__my_left) x = x->__my_left; return x; }
	__node_base::__pointer __node_base::__max(__pointer x) { while(x->__my_right) x = x->__my_right; return x; }
	__node_base::__const_pointer __node_base::__max(__const_pointer x)  { while(x->__my_right) x = x->__my_right; return x; }
	static inline node_direction __opposite(const node_direction dir) { return (dir == RIGHT ? LEFT : RIGHT); }
	static inline bool __is_black(__node_base const* n) { return !n || n->__my_color == BLACK; }
	static inline bool __double_black(__node_base* w, const node_direction dir) { return __is_black(__next(w, dir)); }
	static inline void __rotate(__node_base* const x, __node_base*& root, const node_direction dir)
	{
		const node_direction odir			= ~dir;
		__node_base* const y				= __next(x, odir);
		__next(x, odir)						= __next(y, dir);
		if(__node_base* n					= __next(y, dir))
			n->__my_parent					= x;
		y->__my_parent						= x->__my_parent;
		if(x == root) root					= y;
		else if(x == __next(x->__my_parent, dir))
			__next(x->__my_parent, dir)		= y;
		else __next(x->__my_parent, odir)	= y;
		__next(y, dir)						= x;
		x->__my_parent						= y;
	}
	static inline void __insertion_rebalance(__node_base*& x, __node_base*& root )
	{
		__node_base* const w	= x->__my_parent->__my_parent;
		__node_base* const y	= (x->__my_parent == w->__my_left ? w->__my_right : w->__my_left);
		__node_base* const z	= (x->__my_parent == w->__my_left ? x->__my_parent->__my_right : x->__my_parent->__my_left);
		if(y && y->__my_color == RED)
		{
			x->__my_parent->__my_color	= BLACK;
			y->__my_color				= BLACK;
			w->__my_color				= RED;
			x							= w;
		}
		else
		{
			node_direction dir			= x->__my_parent == w->__my_left ? RIGHT : LEFT;
			if(x == z) __rotate(x		= x->__my_parent, root, ~dir);
			x->__my_parent->__my_color	= BLACK;
			w->__my_color				= RED;
			__rotate(w, root, dir);
		}
	}
	[[gnu::nonnull]] void __insert_and_rebalance(const node_direction dir, __node_base* x, __node_base* p, __node_base& trunk)
	{
		__node_base*& root	= trunk.__my_parent;
		x->__my_parent		= p;
		x->__my_left		= nullptr;
		x->__my_right		= nullptr;
		x->__my_color		= RED;
		if(p == addressof(trunk))
			p->__my_left	= p->__my_right = p->__my_parent = x;
		else
		{
			__next(p, dir)						= x;
			if(p == __next(addressof(trunk), dir))
				__next(addressof(trunk), dir)	= x;
		}
		while(x != root && x->__my_parent->__my_color == RED) __insertion_rebalance(x, root);
		root->__my_color	= BLACK;
	}
	static inline bool __fix_double_black(__node_base*& x, __node_base*& x_parent, __node_base*& root, const node_direction dir)
	{
		const node_direction odir	= ~dir;
		__node_base* w				= __next(x_parent, dir);
		if(w->__my_color == RED)
		{
			w->__my_color			= BLACK;
			x_parent->__my_color	= RED;
			__rotate(x, root, odir);
			w						= __next(x_parent, dir);
		}
		if(__double_black(w, LEFT) && __double_black(w, RIGHT))
		{
			w->__my_color	= RED;
			x				= x_parent;
			x_parent		= x_parent->__my_parent;
		}
		else
		{
			if(__double_black(w, dir))
			{
				__next(w, odir)->__my_color				= BLACK;
				w->__my_color							= RED;
				__rotate(w, root, dir);
				w										= __next(x_parent, dir);
			}
			w->__my_color								= x_parent->__my_color;
			x_parent->__my_color						= BLACK;
			if(__next(w, dir))
			{
				__next(w, dir)->__my_color				= BLACK;
				__rotate(x_parent, root, __opposite(dir));
				return true;
			}
		}
		return false;
	}
	[[gnu::nonnull]] [[gnu::returns_nonnull]] __node_base* __rebalance_for_erase(__node_base* const z, __node_base& trunk)
	{
		__node_base*& root		= trunk.__my_parent;
		__node_base*& leftmost	= trunk.__my_left;
		__node_base*& rightmost	= trunk.__my_right;
		__node_base* y			= z;
		__node_base* x			= nullptr;
		__node_base* x_parent	= nullptr;
		if(!y->__my_left) x		= y->__my_right;
		else if(!y->__my_right)
			x					= y->__my_left;
		else x					= (__node_base::__min(y->__my_right))->__my_right;
		if(y != z)
		{
			z->__my_left->__my_parent		= y;
			y->__my_left					= z->__my_left;
			if(y != z->__my_right)
			{
				x_parent					= y->__my_parent;
				if(x) x->__my_parent		= y->__my_parent;
				y->__my_parent->__my_left	= x;
				y->__my_right				= z->__my_right;
				z->__my_right->__my_parent	= y;
			}
			else x_parent					= y;
			if(root == z) root				= y;
			else if(z->__my_parent->__my_left == z)
				z->__my_parent->__my_left	= y;
			else z->__my_parent->__my_right	= y;
			y->__my_parent					= z->__my_parent;
			swap(y->__my_color, z->__my_color);
			y								= z;
		}
		else
		{
			x_parent						= y->__my_parent;
			if(root == z) root				= x;
			else if(z == z->__my_parent->__my_left)
				z->__my_parent->__my_left	= x;
			else z->__my_parent->__my_right	= x;
			if(z == leftmost) leftmost		= !z->__my_right ? z->__my_parent : __node_base::__min(x);
			if(z == rightmost) rightmost	= z->__my_left ? z->__my_parent : __node_base::__max(z);
		}
		if(y->__my_color	!= RED)
		{
			while(x != root && __is_black(x))
			{
				if(__fix_double_black(x, x_parent, root, (x == x_parent->__my_left ? RIGHT : LEFT)))
					break;
				if(x)x->__my_color	= BLACK;
			}
		}
		root->__my_color			= BLACK;
		return y;
	}
}