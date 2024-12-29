#include "bits/stl_tree.hpp"
namespace std
{
    static __node_base * __local_increment(__node_base *x) throw()
    {
        if(x->__my_right != NULL) return __node_base::__min(x->__my_right);
        __node_base* y = x->__my_parent;
        while(x == y->__my_right)
        {
            x = y;
            y = y->__my_parent;
        }
        if(x->__my_right != y) x = y;
        return x;
    }
    static __node_base* __local_decrement(__node_base* x) throw()
    {
        if(x->__my_color == RED && x->__my_parent->__my_parent == x) return x->__my_right;
        else if(x->__my_left != NULL) return __node_base::__max(x->__my_left);
        else
        {
            __node_base* y = x->__my_parent;
            while(x == y->__my_left)
            {
                x = y;
                y = y->__my_parent;
            }
            x = y;
        }
        return x;
    }
    static void __local_lrotate(__node_base* const x, __node_base* &root)
    {
        __node_base* const y = x->__my_right;
        x->__my_right = y->__my_left;
        if(y->__my_left != NULL) y->__my_left->__my_parent = x;
        y->__my_parent = x->__my_parent;
        if(x == root) root = y;
        else if(x == x->__my_parent->__my_left) x->__my_parent->__my_left = y;
        else x->__my_parent->__my_right = y;
        y->__my_left = x;
        x->__my_parent = y;
    }
    static void __local_rrotate(__node_base* const x, __node_base* &root)
    {
        __node_base* const y = x->__my_left;
        x->__my_left = y->__my_right;
        if(y->__my_right != NULL)  y->__my_right->__my_parent = x;
        y->__my_parent = x->__my_parent;
        if(x == root) root = y;
        else if(x == x->__my_parent->__my_right) x->__my_parent->__my_right = y;
        else x->__my_parent->__my_left = y;
        y->__my_right = x;
        x->__my_parent = y;
    }
    static inline __node_base* __next(__node_base* x, const node_direction dir) { return (dir == RIGHT ? x->__my_right : x->__my_left); }
    __node_base::__ptr __node_base::__min(__ptr x) { while(x->__my_left != NULL) x = x->__my_left; return x; }
    __node_base::__const_ptr __node_base::__min(__const_ptr x) { while(x->__my_left != NULL) x = x->__my_left; return x; }
    __node_base::__ptr __node_base::__max(__ptr x) { while(x->__my_right != NULL) x = x->__my_right; return x; }
    __node_base::__const_ptr __node_base::__max(__const_ptr x)  { while(x->__my_right != NULL) x = x->__my_right; return x; }
    __node_base *__increment_node(__node_base *x) throw() { return __local_increment(x); }
    __node_base *__decrement_node(__node_base *x) throw() { return __local_decrement(x); }
    __node_base const *__increment_node(__node_base const *x) throw() { return __local_increment(const_cast<__node_base*>(x)); }
    __node_base const *__decrement_node(__node_base const *x) throw() { return __local_decrement(const_cast<__node_base*>(x)); }
    static inline node_direction __opposite(const node_direction dir) { return (dir == RIGHT ? LEFT : RIGHT); }
    static inline void __local_rotate(__node_base* const x, __node_base* &root, const node_direction dir) { if(dir == RIGHT) __local_rrotate(x, root); else __local_lrotate(x, root); }
    static inline void __local_rb_1(__node_base* x, __node_base* &root, const node_direction dir)
    {
        __node_base* const grandparent = x->__my_parent->__my_parent;
        __node_base* const y = (dir == RIGHT ? grandparent->__my_right : grandparent->__my_left);
        if(y && y->__my_color == RED)
        {
             // double-red clash
            x->__my_parent->__my_color = BLACK;
            y->__my_color = BLACK;
            grandparent->__my_color = RED;
            x = grandparent;
        }
        else
        {
            __node_base* const z = (dir == RIGHT ? x->__my_parent->__my_right : x->__my_parent->__my_left);
            if(x == z)
            {
                x = x->__my_parent;
                __local_rotate(x, root, __opposite(dir));
            }
            x->__my_parent->__my_color = BLACK;
            grandparent->__my_color = RED;
            __local_rotate(x, root, dir);
        }
    }
    void __insert_and_rebalance(const node_direction dir, __node_base *x, __node_base *p, __node_base &trunk) throw()
    {
        __node_base* &root = trunk.__my_parent;
        x->__my_parent = p;
        x->__my_left = NULL;
        x->__my_right = NULL;
        x->__my_color = RED;
        if(dir == LEFT)
        {
            p->__my_left = x;
            if(p == &trunk)
            {
                trunk.__my_parent = x;
                trunk.__my_right = x;
            }
            else if(p == trunk.__my_left) trunk.__my_left = x;
        }
        else
        {
            p->__my_right = x;
            if(p == trunk.__my_right) trunk.__my_right = x;
        }
        while(x != root && x->__my_parent->__my_color == RED) __local_rb_1(x, root, (x == x->__my_parent->__my_parent->__my_right ? LEFT : RIGHT));
        root->__my_color = BLACK;
    }
    static inline bool __local_rb_check_1(__node_base* w, const node_direction dir)
    {
        __node_base* w1 = (dir == RIGHT ? w->__my_right : w->__my_left);
        return (!w1 || w1->__my_color == BLACK);
    }
    static inline bool __local_rb_2(__node_base* &x, __node_base* &x_parent, __node_base* &root, const node_direction dir)
    {
        __node_base* w = __next(x_parent, dir);
        if(w->__my_color == RED)
        {
            w->__my_color = BLACK;
            x_parent->__my_color = RED;
            __local_rotate(x, root, __opposite(dir));
            w = __next(x_parent, dir);
        }
        if(__local_rb_check_1(w, LEFT) && __local_rb_check_1(w, RIGHT))
        {
            w->__my_color = RED;
            x = x_parent;
            x_parent = x_parent->__my_parent;
        }
        else
        {
            if(__local_rb_check_1(w, dir))
            {
                __next(w, __opposite(dir))->__my_color = BLACK;
                w->__my_color = RED;
                __local_rotate(w, root, dir);
                w = __next(x_parent, dir);
            }
            w->__my_color = x_parent->__my_color;
            x_parent->__my_color = BLACK;
            if(__next(w, dir))
            {
                __next(w, dir)->__my_color = BLACK;
                __local_rotate(x_parent, root, __opposite(dir));
                return true;
            }
        }
        return false;
    }
    __node_base *__rebalance_for_erase(__node_base *const z, __node_base &trunk) throw()
    {
        __node_base* &root = trunk.__my_parent;
        __node_base* &leftmost = trunk.__my_left;
        __node_base* &rightmost = trunk.__my_right;
        __node_base* y = z;
        __node_base* x = NULL;
        __node_base* x_parent = NULL;
        if(y->__my_left == NULL) x = y->__my_right;
        else if(y->__my_right == NULL) x = y->__my_left;
        else
        {
            y = __node_base::__min(y->__my_right);
            x = y->__my_right;
        }
        if(y != z)
        {
            z->__my_left->__my_parent = y;
            y->__my_left = z->__my_left;
            if(y != z->__my_right)
            {
                x_parent = y->__my_parent;
                if(x) x->__my_parent = y->__my_parent;
                y->__my_parent->__my_left = x;
                y->__my_right = z->__my_right;
                z->__my_right->__my_parent = y;
            }
            else x_parent = y;
            if(root == z) root = y;
            else if(z->__my_parent->__my_left == z) z->__my_parent->__my_left = y;
            else z->__my_parent->__my_right = y;
            y->__my_parent = z->__my_parent;
            swap(y->__my_color, z->__my_color);
            y = z;
        }
        else
        {
            x_parent = y->__my_parent;
            if(root == z) root = x;
            else if(z == z->__my_parent->__my_left) z->__my_parent->__my_left = x;
            else z->__my_parent->__my_right = x;
            if(z == leftmost)
            {
                if(z->__my_right == NULL) leftmost = z->__my_parent;
                else leftmost = __node_base::__min(x);
            }
            if(z == rightmost)
            {
                if(z->__my_left == NULL) rightmost = z->__my_parent;
                else rightmost = __node_base::__max(z);
            }
        }
        if(y->__my_color != RED)
        {
            while(x != root && (x == NULL || x->__my_color == BLACK)) if(__local_rb_2(x, x_parent, root, (x == x_parent->__my_left ? RIGHT : LEFT))) break;
            if(x) x->__my_color = BLACK;
        }
        root->__my_color = BLACK;
        return y;
    }
    unsigned int __black_count(const __node_base *node, const __node_base *root)
    {
        if(!node) return 0;
        unsigned int sum = 0;
        do
        {
            if(node->__my_color == BLACK) sum++;
            if(node == root) break;
            node = node->__my_parent;
        } while(1);
        return sum;
    }
}