#ifndef __ATOMIC_BASE
#define __ATOMIC_BASE
#include "bits/move.h"
#define FORCE_INLINE __attribute__((__always_inline__))
namespace std
{
    enum class memory_order
    {
        relaxed = __ATOMIC_RELAXED,
        consume = __ATOMIC_CONSUME,
        acquire = __ATOMIC_ACQUIRE,
        release = __ATOMIC_RELEASE,
        acq_rel = __ATOMIC_ACQ_REL,
        seq_cst = __ATOMIC_SEQ_CST
    };
    constexpr inline memory_order memory_order_relaxed = memory_order::relaxed;
    constexpr inline memory_order memory_order_consume = memory_order::consume;
    constexpr inline memory_order memory_order_acquire = memory_order::acquire;
    constexpr inline memory_order memory_order_release = memory_order::release;
    constexpr inline memory_order memory_order_acq_rel = memory_order::acq_rel;
    constexpr inline memory_order memory_order_seq_cst = memory_order::seq_cst;
    enum __memory_order_modifier
    {
      __memory_order_mask          = 0x0ffff,
      __memory_order_modifier_mask = 0xffff0000,
      __memory_order_hle_acquire   = 0x10000,
      __memory_order_hle_release   = 0x20000
    };
    constexpr memory_order operator|(memory_order __m, __memory_order_modifier __mod)
    {
        return memory_order(int(__m) | int(__mod));
    }

    constexpr memory_order operator&(memory_order __m, __memory_order_modifier __mod)
    {
        return memory_order(int(__m) & int(__mod));
    }
    
    constexpr memory_order __cmpexch_failure_order2(memory_order __m) noexcept
    {
        return __m == memory_order_acq_rel ? memory_order_acquire : __m == memory_order_release ? memory_order_relaxed : __m;
    }

    constexpr memory_order __cmpexch_failure_order(memory_order __m) noexcept
    {
        return memory_order(__cmpexch_failure_order2(__m & __memory_order_mask) | __memory_order_modifier(__m & __memory_order_modifier_mask));
    }

    constexpr bool __is_valid_cmpexch_failure_order(memory_order __m) noexcept
    {
        return (__m & __memory_order_mask) != memory_order_release && (__m & __memory_order_mask) != memory_order_acq_rel;
    }
    namespace __detail
    {
        template<typename T>
        concept __can_atomic_base = std::is_trivially_copyable_v<T> 
                                    && std::is_copy_constructible_v<T> 
                                    && std::is_move_constructible_v<T> 
                                    && std::is_copy_assignable_v<T>
                                    && std::is_move_assignable_v<T>
                                    && std::is_same_v<T, typename std::remove_cv<T>::type>;
        template<typename T> concept __can_atomic_ptr = std::is_pointer_v<T>;
        template<typename T> concept __can_atomic_int = (std::is_integral_v<T> && (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8)) || __can_atomic_ptr<T>;
        template<typename T> concept __can_atomic_fp = std::is_floating_point_v<T>;
        template<typename T> concept __can_atomic = __can_atomic_base<T> || __can_atomic_int<T> || __can_atomic_fp<T>;
        template<typename T> concept __can_atomic_object = __can_atomic_base<T> && !__can_atomic_ptr<T> && !__can_atomic_int<T> && !__can_atomic_fp<T>;
    }
    namespace __impl
    {
        template<typename T> using __val = std::remove_volatile_t<T>;
        template<typename T> using __diff = std::conditional_t<std::is_pointer_v<T>, ptrdiff_t, __val<T>>;
        template<size_t S, size_t A>
        constexpr inline bool __is_lock_free() noexcept { return __atomic_is_lock_free(S, reinterpret_cast<void*>(-A)); }
        template<__detail::__can_atomic T> constexpr inline void store(T* __ptr, __val<T> __t, memory_order __m) noexcept
        {
            __atomic_store(__ptr, std::addressof(__t), static_cast<int>(__m));
        }
        template<__detail::__can_atomic T> constexpr inline __val<T> load(const T* __ptr, memory_order __m) noexcept
        {
            alignas(T) unsigned char __buf[sizeof(T)];
            auto* __result = reinterpret_cast<__val<T>*>(__buf);
            __atomic_load(__ptr, __result, static_cast<int>(__m));
            return *__result;
        }
        template<__detail::__can_atomic T> constexpr inline __val<T> exchange(T* __ptr, __val<T> __target, memory_order __m) noexcept
        {
            alignas(T) unsigned char __buf[sizeof(T)];
            auto* __result = reinterpret_cast<__val<T>*>(__buf);
            __atomic_exchange(__ptr, addressof(__target), __result, static_cast<int>(__m));
            return *__result;
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE bool compare_exchange_weak(T* __ptr, __val<T>& __expect, __val<T> __target, memory_order __success, memory_order __fail)
        {
            return __atomic_compare_exchange(__ptr, addressof(__expect), addressof(__target), true, static_cast<int>(__success), static_cast<int>(__fail));
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE bool compare_exchange_strong(T* __ptr, __val<T>& __expect, __val<T> __target, memory_order __success, memory_order __fail)
        {
            return __atomic_compare_exchange(__ptr, addressof(__expect), addressof(__target), false, static_cast<int>(__success), static_cast<int>(__fail));
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T fetch_add(T* __ptr, __diff<T> __i, memory_order __m)
        {
            return __atomic_fetch_add(__ptr, __i, static_cast<int>(__m));
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T fetch_sub(T* __ptr, __diff<T> __i, memory_order __m)
        {
            return __atomic_fetch_sub(__ptr, __i, static_cast<int>(__m));
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T fetch_and(T* __ptr, __val<T> __i, memory_order __m)
        {
            return __atomic_fetch_and(__ptr, __i, static_cast<int>(__m));
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T fetch_or(T* __ptr, __val<T> __i, memory_order __m)
        {
            return __atomic_fetch_or(__ptr, __i, static_cast<int>(__m));
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T fetch_xor(T* __ptr, __val<T> __i, memory_order __m)
        {
            return __atomic_fetch_xor(__ptr, __i, static_cast<int>(__m));
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T add_fetch(T* __ptr, __diff<T> __i)
        {
            return __atomic_add_fetch(__ptr, __i, __ATOMIC_SEQ_CST);
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T sub_fetch(T* __ptr, __diff<T> __i)
        {
            return __atomic_sub_fetch(__ptr, __i, __ATOMIC_SEQ_CST);
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T and_fetch(T* __ptr, __val<T> __i)
        {
            return __atomic_and_fetch(__ptr, __i, __ATOMIC_SEQ_CST);
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T or_fetch(T* __ptr, __val<T> __i)
        {
            return __atomic_or_fetch(__ptr, __i, __ATOMIC_SEQ_CST);
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T xor_fetch(T* __ptr, __val<T> __i)
        {
            return __atomic_xor_fetch(__ptr, __i, __ATOMIC_SEQ_CST);
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T __fetch_add_flt(T* __ptr, __val<T> __i, memory_order __m) noexcept
        {
            __val<T> __old = load(__ptr, memory_order_relaxed);
            __val<T> __new = __old + __i;
            while(!compare_exchange_weak(__ptr, __old, __new, __m, memory_order_relaxed)) __new = __old + i;
            return __old;
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T __fetch_sub_flt(T* __ptr, __val<T> __i, memory_order __m) noexcept
        {
            __val<T> __old = load(__ptr, memory_order_relaxed);
            __val<T> __new = __old - __i;
            while(!compare_exchange_weak(__ptr, __old, __new, __m, memory_order_relaxed)) __new = __old - i;
            return __old;
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T __add_fetch_flt(T* __ptr, __val<T> __i) noexcept
        {
            __val<T> __old = load(__ptr, memory_order_relaxed);
            __val<T> __new = __old + __i;
            while(!compare_exchange_weak(__ptr, __old, __new, memory_order_seq_cst, memory_order_relaxed)) __new = __old + i;
            return __old;
        }
        template<__detail::__can_atomic T> 
        constexpr FORCE_INLINE T __sub_fetch_flt(T* __ptr, __val<T> __i) noexcept
        {
            __val<T> __old = load(__ptr, memory_order_relaxed);
            __val<T> __new = __old - __i;
            while(!compare_exchange_weak(__ptr, __old, __new, memory_order_seq_cst, memory_order_relaxed)) __new = __old - i;
            return __old;
        }
    }
    #ifdef __GCC_ATOMIC_TEST_AND_SET_TRUEVAL
    #define TRUEVAL __GCC_ATOMIC_TEST_AND_SET_TRUEVAL
    #else
    #define TRUEVAL 1
    #endif
    #if TRUEVAL == 1
    typedef bool __atomic_flag_data_type;
    #else
    typedef unsigned char __atomic_flag_data_type;
    #endif
    #define FLG(x) x ? TRUEVAL : 0
    struct __atomic_flag_base
    {
        __atomic_flag_data_type __my_flag{};
    };
    struct atomic_flag : public __atomic_flag_base
    {
        atomic_flag() noexcept = default;
        ~atomic_flag() noexcept = default;
        atomic_flag(atomic_flag const&) = delete;
        atomic_flag& operator=(atomic_flag const&) = delete;
        atomic_flag& operator=(atomic_flag const&) = delete;
        constexpr atomic_flag(bool __x) : __atomic_flag_base{ FLG(__x) } {}
        FORCE_INLINE bool test_and_set(memory_order __m = memory_order_seq_cst) noexcept
        {
            return __atomic_test_and_set(&__my_flag, static_cast<int>(__m));
        }
        FORCE_INLINE bool test_and_set(memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return __atomic_test_and_set(&__my_flag, static_cast<int>(__m));
        }
        FORCE_INLINE bool test(memory_order __m = memory_order_seq_cst) const noexcept
        {
            __atomic_flag_data_type __v;
            __atomic_load(&__my_flag, &__v, static_cast<int>(__m));
            return __v == TRUEVAL;
        }
        FORCE_INLINE bool test(memory_order __m = memory_order_seq_cst) const volatile noexcept
        {
            __atomic_flag_data_type __v;
            __atomic_load(&__my_flag, &__v, static_cast<int>(__m));
            return __v == TRUEVAL;
        }
        FORCE_INLINE void clear(memory_order __m = memory_order_seq_cst) noexcept
        {
            __atomic_clear(&__my_flag, static_cast<int>(__m));
        }
        FORCE_INLINE void clear(memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            __atomic_clear(&__my_flag, static_cast<int>(__m));
        }
    };
    template<__detail::__can_atomic_int IT>
    struct atomic
    {
        using value_type = IT;
        using difference_type = __impl::__diff<IT>;
    private:
        constexpr static int __align = sizeof(IT) > alignof(IT) ? sizeof(IT) : alignof(IT);
        alignas(__align) IT __my_val {0};
    public:
        atomic() noexcept = default;
        ~atomic() noexcept = default;
        atomic(atomic<IT> const&) = delete;
        atomic<IT>& operator=(atomic<IT> const&) = delete;
        atomic<IT>& operator=(atomic<IT> const&) volatile = delete;
        constexpr atomic(IT __i) noexcept : __my_val {__i} {}
        bool is_lock_free() const noexcept
        {
            return __impl::__is_lock_free<sizeof(value_type), __align>();
        }
        bool is_lock_free() const volatile noexcept
        {
            return __impl::__is_lock_free<sizeof(value_type), __align>();
        }
        FORCE_INLINE void store(value_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            __atomic_store_n(&__my_val, static_cast<int>(__m));
        }
        FORCE_INLINE void store(value_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            __atomic_store_n(&__my_val, static_cast<int>(__m));
        }
        FORCE_INLINE value_type load(value_type __i, memory_order __m = memory_order_seq_cst) const noexcept
        {
            return __atomic_load_n(&__my_val, static_cast<int>(__m));
        }
        FORCE_INLINE value_type load(value_type __i, memory_order __m = memory_order_seq_cst) const volatile noexcept
        {
            return __atomic_load_n(&__my_val, static_cast<int>(__m));
        }
        FORCE_INLINE value_type exchange(value_type __i, memory_order __m) noexcept
        {
            return __atomic_exchange_n(&__my_val, static_cast<int>(__m));
        }
        FORCE_INLINE value_type exchange(value_type __i, memory_order __m) volatile noexcept
        {
            return __atomic_exchange_n(&__my_val, static_cast<int>(__m));
        }
        FORCE_INLINE bool compare_exchange_weak(value_type& __i1, value_type __i2, memory_order __m1, memory_order __m2) noexcept
        {
            return __atomic_compare_exchange_n(&__my_val, &__i1, __i2, 1, static_cast<int>(m1), static_cast<int>(__m2));
        }
        FORCE_INLINE bool compare_exchange_weak(value_type& __i1, value_type __i2, memory_order __m1, memory_order __m2) volatile noexcept
        {
            return __atomic_compare_exchange_n(&__my_val, &__i1, __i2, 1, static_cast<int>(m1), static_cast<int>(__m2));
        }
        FORCE_INLINE bool compare_exchange_weak(value_type& __i1, value_type __i2, memory_order __m = memory_order_seq_cst) noexcept
        {
            return compare_exchange_weak(__i1, __i2, __m, __cmpexch_failure_order(__m));
        }
        FORCE_INLINE bool compare_exchange_weak(value_type& __i1, value_type __i2, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return compare_exchange_weak(__i1, __i2, __m, __cmpexch_failure_order(__m));
        }
        FORCE_INLINE bool compare_exchange_strong(value_type& __i1, value_type __i2, memory_order __m1, memory_order __m2) noexcept
        {
            return __atomic_compare_exchange_n(&__my_val, &__i1, __i2, 0, static_cast<int>(__m1), static_cast<int>(__m2));
        }
        FORCE_INLINE bool compare_exchange_strong(value_type& __i1, value_type __i2, memory_order __m1, memory_order __m2) volatile noexcept
        {
            return __atomic_compare_exchange_n(&__my_val, &__i1, __i2, 0, static_cast<int>(__m1), static_cast<int>(__m2));
        }
        FORCE_INLINE bool compare_exchange_strong(value_type& __i1, value_type __i2, memory_order __m = memory_order_seq_cst) noexcept
        {
            return compare_exchange_strong(__i1, __i2, __m, __cmpexch_failure_order(__m));
        }
        FORCE_INLINE bool compare_exchange_strong(value_type& __i1, value_type __i2, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return compare_exchange_strong(__i1, __i2, __m, __cmpexch_failure_order(__m));
        } 
		FORCE_INLINE value_type fetch_add(difference_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            return __atomic_fetch_add(&__my_val, __i, static_cast<int>(__m));
        }
        FORCE_INLINE value_type fetch_sub(difference_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            return __atomic_fetch_sub(&__my_val, __i, static_cast<int>(__m));
        }
        FORCE_INLINE value_type fetch_and(value_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            return __atomic_fetch_and(&__my_val, __i, static_cast<int>(__m));
        }
        FORCE_INLINE value_type fetch_or(value_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            return __atomic_fetch_or(&__my_val, __i, static_cast<int>(__m));
        }
        FORCE_INLINE value_type fetch_xor(value_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            return __atomic_fetch_xor(&__my_val, __i, static_cast<int>(__m));
        }
        FORCE_INLINE value_type fetch_add(difference_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return __atomic_fetch_add(&__my_val, __i, static_cast<int>(__m));
        }
        FORCE_INLINE value_type fetch_sub(difference_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return __atomic_fetch_sub(&__my_val, __i, static_cast<int>(__m));
        }
        FORCE_INLINE value_type fetch_and(value_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return __atomic_fetch_and(&__my_val, __i, static_cast<int>(__m));
        }
        FORCE_INLINE value_type fetch_or(value_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return __atomic_fetch_or(&__my_val, __i, static_cast<int>(__m));
        }
        FORCE_INLINE value_type fetch_xor(value_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return __atomic_fetch_xor(&__my_val, __i, static_cast<int>(__m));
        }
        operator value_type() const noexcept { return load(); }
        operator value_type() const volatile noexcept { return load(); }
        value_type operator=(value_type __i) noexcept { store(__i); return __i; }
        value_type operator=(value_type __i) volatile noexcept { store(__i); return __i; }
        value_type operator++(int) noexcept { return fetch_add(1); }
        value_type operator++(int) volatile noexcept { return fetch_add(1); }
        value_type operator--(int) noexcept { return fetch_sub(1); }
        value_type operator--(int) volatile noexcept { return fetch_sub(1); }
        value_type operator++() noexcept { return __atomic_add_fetch(&__my_val, 1, __ATOMIC_SEQ_CST); }
        value_type operator++() volatile noexcept { return __atomic_add_fetch(&__my_val, 1, __ATOMIC_SEQ_CST); }
        value_type operator--() noexcept { return __atomic_sub_fetch(&__my_val, 1, __ATOMIC_SEQ_CST); }
        value_type operator--() volatile noexcept { return __atomic_sub_fetch(&__my_val, 1, __ATOMIC_SEQ_CST); }
        value_type operator+=(difference_type __i) noexcept { return __atomic_add_fetch(&__my_val, __i, __ATOMIC_SEQ_CST); }
        value_type operator-=(difference_type __i) noexcept { return __atomic_sub_fetch(&__my_val, __i, __ATOMIC_SEQ_CST); }
        value_type operator&=(value_type __i) noexcept { return __atomic_and_fetch(&__my_val, __i, __ATOMIC_SEQ_CST); }
        value_type operator|=(value_type __i) noexcept { return __atomic_or_fetch(&__my_val, __i, __ATOMIC_SEQ_CST); }
        value_type operator^=(value_type __i) noexcept { return __atomic_xor_fetch(&__my_val, __i, __ATOMIC_SEQ_CST); }
        value_type operator+=(difference_type  __i) volatile noexcept { return __atomic_add_fetch(&__my_val, __i, __ATOMIC_SEQ_CST); }
        value_type operator-=(difference_type  __i) volatile noexcept { return __atomic_sub_fetch(&__my_val, __i, __ATOMIC_SEQ_CST); }
        value_type operator&=(value_type __i) volatile noexcept { return __atomic_and_fetch(&__my_val, __i, __ATOMIC_SEQ_CST); }
        value_type operator|=(value_type __i) volatile noexcept { return __atomic_or_fetch(&__my_val, __i, __ATOMIC_SEQ_CST); }
        value_type operator^=(value_type __i) volatile noexcept { return __atomic_xor_fetch(&__my_val, __i, __ATOMIC_SEQ_CST); }
    };
    template<typename T>
    struct atomic<T*>
    {
        using pointer_type = T*;
        using difference_type = ptrdiff_t;
    private:
        pointer_type __my_ptr { nullptr };
        constexpr difference_type __type_size(difference_type __d) const { return __d * sizeof(T); }
        constexpr difference_type __type_size(difference_type __d) const volatile { return __d * sizeof(T); }
        atomic() noexcept = default;
        ~atomic() noexcept = default;
        atomic(atomic<T*> const&) = delete;
        atomic<T*>& operator=(atomic<T*> const&) = delete;
        atomic<T*>& operator=(atomic<T*> const&) volatile = delete;
        constexpr atomic(pointer_type __p) noexcept : __my_ptr{__p} {}
        bool is_lock_free() const noexcept
        {
            return __impl::__is_lock_free<sizeof(pointer_type), __align>();
        }
        bool is_lock_free() const volatile noexcept
        {
            return __impl::__is_lock_free<sizeof(pointer_type), __align>();
        }
        FORCE_INLINE void store(pointer_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            __atomic_store_n(&__my_ptr, static_cast<int>(__m));
        }
        FORCE_INLINE void store(pointer_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            __atomic_store_n(&__my_ptr, static_cast<int>(__m));
        }
        FORCE_INLINE pointer_type load(pointer_type __i, memory_order __m = memory_order_seq_cst) const noexcept
        {
            return __atomic_load_n(&__my_ptr, static_cast<int>(__m));
        }
        FORCE_INLINE pointer_type load(pointer_type __i, memory_order __m = memory_order_seq_cst) const volatile noexcept
        {
            return __atomic_load_n(&__my_ptr, static_cast<int>(__m));
        }
        FORCE_INLINE pointer_type exchange(pointer_type __i, memory_order __m) noexcept
        {
            return __atomic_exchange_n(&__my_ptr, static_cast<int>(__m));
        }
        FORCE_INLINE pointer_type exchange(pointer_type __i, memory_order __m) volatile noexcept
        {
            return __atomic_exchange_n(&__my_ptr, static_cast<int>(__m));
        }
        FORCE_INLINE bool compare_exchange_weak(pointer_type& __i1, pointer_type __i2, memory_order __m1, memory_order __m2) noexcept
        {
            return __atomic_compare_exchange_n(&__my_ptr, &__i1, __i2, 1, static_cast<int>(m1), static_cast<int>(__m2));
        }
        FORCE_INLINE bool compare_exchange_weak(pointer_type& __i1, pointer_type __i2, memory_order __m1, memory_order __m2) volatile noexcept
        {
            return __atomic_compare_exchange_n(&__my_ptr, &__i1, __i2, 1, static_cast<int>(m1), static_cast<int>(__m2));
        }
        FORCE_INLINE bool compare_exchange_weak(pointer_type& __i1, pointer_type __i2, memory_order __m = memory_order_seq_cst) noexcept
        {
            return compare_exchange_weak(__i1, __i2, __m, __cmpexch_failure_order(__m));
        }
        FORCE_INLINE bool compare_exchange_weak(pointer_type& __i1, pointer_type __i2, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return compare_exchange_weak(__i1, __i2, __m, __cmpexch_failure_order(__m));
        }
        FORCE_INLINE bool compare_exchange_strong(pointer_type& __i1, pointer_type __i2, memory_order __m1, memory_order __m2) noexcept
        {
            return __atomic_compare_exchange_n(&__my_ptr, &__i1, __i2, 0, static_cast<int>(__m1), static_cast<int>(__m2));
        }
        FORCE_INLINE bool compare_exchange_strong(pointer_type& __i1, pointer_type __i2, memory_order __m1, memory_order __m2) volatile noexcept
        {
            return __atomic_compare_exchange_n(&__my_ptr, &__i1, __i2, 0, static_cast<int>(__m1), static_cast<int>(__m2));
        }
        FORCE_INLINE bool compare_exchange_strong(pointer_type& __i1, pointer_type __i2, memory_order __m = memory_order_seq_cst) noexcept
        {
            return compare_exchange_strong(__i1, __i2, __m, __cmpexch_failure_order(__m));
        }
        FORCE_INLINE bool compare_exchange_strong(pointer_type& __i1, pointer_type __i2, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return compare_exchange_strong(__i1, __i2, __m, __cmpexch_failure_order(__m));
        } 
		FORCE_INLINE pointer_type fetch_add(difference_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            return __atomic_fetch_add(&__my_ptr, __type_size(__i), static_cast<int>(__m));
        }
        FORCE_INLINE pointer_type fetch_sub(difference_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            return __atomic_fetch_sub(&__my_ptr, __type_size(__i), static_cast<int>(__m));
        }
        FORCE_INLINE pointer_type fetch_add(difference_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return __atomic_fetch_add(&__my_ptr, __type_size(__i), static_cast<int>(__m));
        }
        FORCE_INLINE pointer_type fetch_sub(difference_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return __atomic_fetch_sub(&__my_ptr, __type_size(__i), static_cast<int>(__m));
        }
        operator value_type() const noexcept { return load(); }
        operator value_type() const volatile noexcept { return load(); }
        pointer_type operator=(pointer_type __i) noexcept { store(__i); return __i; }
        pointer_type operator=(pointer_type __i) volatile noexcept { store(__i); return __i; }
        pointer_type operator++(int) noexcept { return fetch_add(1); }
        pointer_type operator++(int) volatile noexcept { return fetch_add(1); }
        pointer_type operator--(int) noexcept { return fetch_sub(1); }
        pointer_type operator--(int) volatile noexcept { return fetch_sub(1); }
        pointer_type operator++() noexcept { return __atomic_add_fetch(&__my_ptr, __type_size(1), __ATOMIC_SEQ_CST); }
        pointer_type operator++() volatile noexcept { return __atomic_add_fetch(&__my_ptr, __type_size(1), __ATOMIC_SEQ_CST); }
        pointer_type operator--() noexcept { return __atomic_sub_fetch(&__my_ptr, __type_size(1), __ATOMIC_SEQ_CST); }
        pointer_type operator--() volatile noexcept { return __atomic_sub_fetch(&__my_ptr, __type_size(1), __ATOMIC_SEQ_CST); }
        pointer_type operator+=(difference_type __i) noexcept { return __atomic_add_fetch(&__my_ptr, __type_size(__i), __ATOMIC_SEQ_CST); }
        pointer_type operator-=(difference_type __i) noexcept { return __atomic_sub_fetch(&__my_ptr, __type_size(__i), __ATOMIC_SEQ_CST); }
        pointer_type operator+=(difference_type  __i) volatile noexcept { return __atomic_add_fetch(&__my_ptr, __type_size(__i), __ATOMIC_SEQ_CST); }
        pointer_type operator-=(difference_type  __i) volatile noexcept { return __atomic_sub_fetch(&__my_ptr, __type_size(__i), __ATOMIC_SEQ_CST); }
    };
    template<__detail::__can_atomic_fp FT>
    struct atomic
    {
        constexpr static size_t __align = alignof(FT);
        using value_type = FT;
        using difference_type = FT;
    private:
        value_type __my_val{0};
    public:
        constexpr static bool is_always_lock_free = __atomic_always_lock_free(sizeof(FT), 0);
        atomic() = default;
        constexpr atomic(value_type __f) : __my_val{ __f } {}
        atomic(atomic const&) = delete;
        atomic& operator=(atomic const&) = delete;
        atomic& operator=(atomic const&) volatile = delete;
         bool is_lock_free() const noexcept
        {
            return __impl::__is_lock_free<sizeof(value_type), __align>();
        }
        bool is_lock_free() const volatile noexcept
        {
            return __impl::__is_lock_free<sizeof(value_type), __align>();
        }
        FORCE_INLINE void store(value_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            __impl::store(&__my_val, __m);
        }
        FORCE_INLINE void store(value_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            __impl::store(&__my_val, __m);
        }
        FORCE_INLINE value_type load(value_type __i, memory_order __m = memory_order_seq_cst) const noexcept
        {
            return __impl::load(&__my_val, __m);
        }
        FORCE_INLINE value_type load(value_type __i, memory_order __m = memory_order_seq_cst) const volatile noexcept
        {
            return __impl::load(&__my_val, __m);
        }
        FORCE_INLINE value_type exchange(value_type __i, memory_order __m) noexcept
        {
            return __impl::exchange(&__my_val, __m);
        }
        FORCE_INLINE value_type exchange(value_type __i, memory_order __m) volatile noexcept
        {
            return __impl::exchange(&__my_val, __m);
        }
        FORCE_INLINE bool compare_exchange_weak(value_type& __i1, value_type i2, memory_order __m1, memory_order __m2) noexcept
        {
            return __impl::compare_exchange_weak(&__my_val, &__i1, __i2, 1, __m1, __m2);
        }
        FORCE_INLINE bool compare_exchange_weak(value_type& __i1, value_type i2, memory_order __m1, memory_order __m2) volatile noexcept
        {
            return __impl::compare_exchange_weak(&__my_val, &__i1, __i2, 1, __m1, __m2);
        }
        FORCE_INLINE bool compare_exchange_weak(value_type& __i1, value_type __i2, memory_order __m = memory_order_seq_cst) noexcept
        {
            return compare_exchange_weak(__i1, __i2, __m, __cmpexch_failure_order(__m));
        }
        FORCE_INLINE bool compare_exchange_weak(value_type& __i1, value_type __i2, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return compare_exchange_weak(__i1, __i2, __m, __cmpexch_failure_order(__m));
        }
        FORCE_INLINE bool compare_exchange_strong(value_type& __i1, value_type i2, memory_order __m1, memory_order __m2) noexcept
        {
            return __impl::compare_exchange_strong(&__my_val, &__i1, __i2, 0, __m1, __m2);
        }
        FORCE_INLINE bool compare_exchange_strong(value_type& __i1, value_type i2, memory_order __m1, memory_order __m2) volatile noexcept
        {
            return __impl::compare_exchange_strong(&__my_val, &__i1, __i2, 0, __m, __m2);
        }
        FORCE_INLINE bool compare_exchange_strong(value_type& __i1, value_type __i2, memory_order __m = memory_order_seq_cst) noexcept
        {
            return compare_exchange_strong(__i1, __i2, __m, __cmpexch_failure_order(__m));
        }
        FORCE_INLINE bool compare_exchange_strong(value_type& __i1, value_type __i2, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return compare_exchange_strong(__i1, __i2, __m, __cmpexch_failure_order(__m));
        } 
		FORCE_INLINE value_type fetch_add(difference_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            return __impl::__fetch_add_flt(&__my_val, __i, __m);
        }
        FORCE_INLINE value_type fetch_sub(difference_type __i, memory_order __m = memory_order_seq_cst) noexcept
        {
            return __impl::__fetch_sub_flt(&__my_val, __i, __m);
        }
        FORCE_INLINE value_type fetch_add(difference_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return __impl::__fetch_add_flt(&__my_val, __i, __m);
        }
        FORCE_INLINE value_type fetch_sub(difference_type __i, memory_order __m = memory_order_seq_cst) volatile noexcept
        {
            return __impl::__fetch_sub_flt(&__my_val, __i, __m);
        }
        operator value_type() const noexcept { return load(); }
        operator value_type() const volatile noexcept { return load(); }
        value_type operator=(value_type __i) noexcept { store(__i); return __i; }
        value_type operator=(value_type __i) volatile noexcept { store(__i); return __i; }
        value_type operator++(int) noexcept { return fetch_add(1); }
        value_type operator++(int) volatile noexcept { return fetch_add(1); }
        value_type operator--(int) noexcept { return fetch_sub(1); }
        value_type operator--(int) volatile noexcept { return fetch_sub(1); }
        value_type operator++() noexcept { return __impl::__add_fetch_flt(&__my_val, 1, memory_order_seq_cst); }
        value_type operator++() volatile noexcept { return __impl::__add_fetch_flt(&__my_val, 1, memory_order_seq_cst); }
        value_type operator--() noexcept { return __impl::__sub_fetch_flt(&__my_val, 1, memory_order_seq_cst); }
        value_type operator--() volatile noexcept { return __impl::__sub_fetch_flt(&__my_val, 1, memory_order_seq_cst); }
        value_type operator+=(difference_type __i) noexcept { return __impl::__add_fetch_flt(&__my_val, __i, memory_order_seq_cst); }
        value_type operator-=(difference_type __i) noexcept { return __impl::__sub_fetch_flt(&__my_val, __i, memory_order_seq_cst); }
        value_type operator+=(difference_type  __i) volatile noexcept { return __impl::__add_fetch_flt(&__my_val, __i, memory_order_seq_cst); }
        value_type operator-=(difference_type  __i) volatile noexcept { return __impl::__sub_fetch_flt(&__my_val, __i, memory_order_seq_cst); }
    };
}

#endif