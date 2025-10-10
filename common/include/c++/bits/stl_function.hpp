#ifndef __STL_FUNCTION
#define __STL_FUNCTION
#include "concepts"
#include "bits/move.h"
#include "bits/invoke.hpp"
#include "bits/typeinfo.h"
#include "exception"
#ifndef __isrcall
#define __isrcall
#endif
namespace std
{
    __isrcall [[noreturn]] void __throw_bad_function_call();
    class bad_function_call : public exception
    {
    public:
        virtual ~bad_function_call() noexcept;
        const char* what() const noexcept;
    };
    template<typename FT> class function;
    template<typename T> concept __location_invariant = is_trivially_copyable_v<T>;
    class __undefined;
    union __no_copy
    {
        void*       __object;
        const void* __object_const;
        void (*__function_ptr)();
        void (__undefined::*__memfn_ptr)();
    };
    union [[gnu::may_alias]] __data_store
    {
        __no_copy __ignored;
        char __stored_data[sizeof(__no_copy)];
        constexpr void* __access() noexcept { return &__stored_data[0]; }
        constexpr const void* __access() const noexcept { return &__stored_data[0]; }
        template<typename T> constexpr T& __access() noexcept { return *static_cast<T*>(__access()); }
        template<typename T> constexpr T const& __access() const noexcept { return *static_cast<T const*>(__access()); }
    };
    enum __func_manage_op
    {
        __get_type_info,
        __get_functor_ptr,
        __clone_functor,
        __destroy_functor
    };
    template<typename R, typename C, typename ... Args> using member_fn = R (C::*)(Args...);
    struct __function_base
    {
        template<typename FT>
        class __fn_manager
        {
            constexpr static size_t __max_size  = sizeof(__no_copy);
            constexpr static size_t __max_align = alignof(__no_copy);
            template<typename F2> constexpr static void __create_wrapper(__data_store& dest, F2&& src, true_type) { ::new(dest.__access()) FT{ forward<F2>(src) }; }
            template<typename F2> constexpr static void __create_wrapper(__data_store& dest, F2&& src, false_type) { dest.__access<FT*>() = new FT{ forward<F2>(src) }; }
            constexpr static void __delete_wrapper(__data_store& target, true_type) { target.__access<FT>().~FT(); }
            constexpr static void __delete_wrapper(__data_store& target, false_type) { ::operator delete(target.__access<FT*>()); }
        protected:
            using __is_locally_storable = __and_<is_trivially_copyable<FT>, bool_constant<sizeof(FT) <= __max_size && __alignof__(FT) <= __max_align && __max_align % __alignof__(FT) == 0>>;
            constexpr static bool __is_local_store = __is_locally_storable::value;
            constexpr static FT* __get_pointer(__data_store const& data) { if constexpr(__is_local_store) return addressof(const_cast<FT&>(data.__access<FT>())); else return data.__access<FT*>(); }
        public:
            template<typename F2> constexpr static void __init_fn(__data_store& dest, F2&& src) noexcept(__and_<__is_locally_storable, is_nothrow_constructible<FT, F2>>::value) { __create_wrapper(dest, forward<F2>(src), __is_locally_storable{}); }
            template<typename F2> constexpr static bool __not_empty(F2* f2) noexcept { return f2 != nullptr; }
            template<typename R, typename C, typename ... Args> constexpr static bool __not_empty(member_fn<R, C, Args...> mf) noexcept { return mf != nullptr; }
            template<typename F2> constexpr static bool __not_empty(function<F2> const& f2) noexcept { return static_cast<bool>(f2); }
            template<typename T> constexpr static bool __not_empty(T const&) noexcept { return true; }
            __isrcall static bool __manager(__data_store& dest, __data_store const& src, __func_manage_op op)
            {
                switch(op)
                {
                case __get_functor_ptr:
                    dest.__access<FT*>() = __get_pointer(src);
                    break;
                case __clone_functor:
                    __init_fn(dest, *const_cast<FT const*>(__get_pointer(src)));
                    break;
                case __destroy_functor:
                    __delete_wrapper(dest, __is_locally_storable{});
                    break;
                case __get_type_info:
                    dest.__access<type_info const*>() = &typeid(FT);
                    break;
                default:
                    break;
                }
                return false;
            }
        };
        using __manager_type = bool(*)(__data_store&, __data_store const&, __func_manage_op);
        __data_store __my_functor{};
        __manager_type __my_manager{};
        constexpr __function_base() noexcept = default;
        constexpr ~__function_base() noexcept { if(__my_manager) __my_manager(__my_functor, __my_functor, __destroy_functor); }
        constexpr bool __empty() const noexcept { return !__my_manager; }
    };
    template<typename ST, typename FT> class __function_helper;
    template<typename RT, typename FT, typename ... Args>
    class __function_helper<RT(Args...), FT> : public __function_base::__fn_manager<FT>
    {
        using __base = __function_base::__fn_manager<FT>;
    public:
        __isrcall static bool __manager(__data_store& dest, __data_store const& src, __func_manage_op op) { if(op == __get_functor_ptr) { dest.__access<FT*>() = __base::__get_pointer(src); return false; } return __base::__manager(dest, src, op); }
        __isrcall static RT __invoke_fn(__data_store const& fn, Args&& ... args) noexcept(is_nothrow_invocable_r_v<RT, FT, Args...>) { return __invoke_r<RT>(*(__base::__get_pointer(fn)), forward<Args>(args)...); }
        template<typename F2> constexpr static bool __is_nothrow_init() noexcept { return __and_<is_nothrow_constructible<F2>, typename __base::__is_locally_storable>::value; }
    };
    template<> class __function_helper<void, void> { public: constexpr static bool __manager(__data_store& dest, __data_store const& src, __func_manage_op op) { return false; } };
    template<typename S, typename F> class __target_helper;
    template<typename S, object F> class __target_helper<S, F> : public __function_helper<S, F>{};
    template<typename S, non_object F> class __target_helper<S, F> : public __function_helper<void, void>{};
    template<typename FT, typename RT, typename ... Args> using __invocable_as = typename __is_invocable_impl<__invoke_result<decay_t<FT>&, Args...>, RT>::type;
    template<typename FT, typename RT, typename ... Args> concept __alternate_callable = not_self<FT, function<RT(Args...)>> && __invocable_as<FT, RT, Args...>::value;
    template<typename RT, typename ... Args>
    class function<RT(Args...)> : __function_base
    {
        template<typename FT> using __helper = __function_helper<RT(Args...), __decay_t<FT>>;
        using __invoker_type = RT (*) (__data_store const&, Args&&...);
        __invoker_type __my_invoker = nullptr;
    public:
        typedef RT result_type;
        template<__alternate_callable<RT, Args...> FT>
        requires (is_copy_constructible<__decay_t<FT>>::value && is_constructible<__decay_t<FT>, FT>::value)
        constexpr function(FT&& ft) noexcept(__helper<FT>::template __is_nothrow_init<FT>()) : __function_base{}
        {
            using __my_helper = __helper<FT>;
            if(__my_helper::__not_empty(ft))
            {
                __my_helper::__init_fn(__my_functor, forward<FT>(ft));
                __my_invoker = &__my_helper::__invoke_fn;
                __my_manager = &__my_helper::__manager;
            }
        }
        constexpr operator bool() const noexcept { return !this->__empty(); }
        constexpr function(function const& that) : __function_base{} 
        {
            if(that.operator bool())
            {
                that.__my_manager(__my_functor, that.__my_functor, __clone_functor);
                __my_invoker = that.__my_invoker;
                __my_manager = that.__my_manager;
            }
        }
        constexpr function(function&& that) : __function_base{}, __my_invoker{ that.__my_invoker }
        {
            if(that.operator bool())
            {
                __my_functor        = that.__my_functor;
                __my_manager        = that.__my_manager;
                that.__my_manager   = nullptr;
                that.__my_invoker   = nullptr;
            }
        }
        constexpr function() noexcept : __function_base{} {}
        template<typename FT> const FT* target() const noexcept 
        {
            if constexpr(is_object_v<FT>)
            {
                using __handler = __target_helper<RT(Args...), FT>;
                if(__my_manager == &__handler::__manager || (__my_manager && typeid(FT) == target_type()))
                {
                    __data_store ptr;
                    __my_manager(ptr, __my_functor, __get_functor_ptr);
                    return ptr.__access<FT const*>();
                }
            }
            return nullptr;
        }
        template<typename FT> FT* target() noexcept { const function* __const_this = this; const FT* __const_ft = __const_this->template target<FT>(); return *const_cast<FT**>(&__const_ft); }
        constexpr void swap(function& that) noexcept { std::swap(__my_functor, that.__my_functor); std::swap(__my_invoker, that.__my_invoker); std::swap(__my_manager, that.__my_manager); }
        constexpr function& operator=(function const& that) noexcept { function{ that }.swap(*this); return *this; }
        constexpr function& operator=(function&& that) noexcept { function{ move(that) }.swap(*this); return *this; }
        constexpr function& operator=(nullptr_t) noexcept { if(__my_manager) { __my_manager(__my_functor, __my_functor, __destroy_functor); __my_manager = nullptr; __my_invoker = nullptr; } return *this; }
        template<__alternate_callable<RT, Args...> FT> constexpr function& operator=(FT&& ft) noexcept(__helper<FT>::template __is_nothrow_init<FT>()) { function{ forward<FT>(ft) }.swap(*this); return *this; }
        __isrcall RT operator()(Args ... args) const { if(this->__empty()) { __throw_bad_function_call(); } return __my_invoker(__my_functor, forward<Args>(args)...); }
        constexpr type_info const& target_type() const noexcept { if(__my_manager) { __data_store __ti_result; __my_manager(__ti_result, __my_functor, __get_type_info); if(type_info const* result = __ti_result.__access<type_info const*>()) return *result; } return typeid(void); }
    };
    template<typename> struct __function_guide_helper{};
    template<typename RT, typename ST, bool NT, typename... Args> struct __function_guide_helper<RT (ST::*) (Args...) noexcept(NT)> { using type = RT (Args...); };
    template<typename FT, typename O> using __function_guide_t = typename __function_guide_helper<O>::type;
    template<typename RT, typename... Args> function(RT(*)(Args...)) -> function<RT(Args...)>;
    template<typename FT, typename ST = __function_guide_t<FT, decltype(&FT::operator())>> function(FT) -> function<ST>;
    namespace ext
    {
        template<typename FT, typename TT> concept __explicitly_convertible_to = requires { static_cast<TT>(declval<FT>()); };
        template<typename FT, typename TT> requires __explicitly_convertible_to<FT, TT> struct static_cast_t { constexpr TT operator()(FT ft) const noexcept { return static_cast<TT>(ft); } };
        template<typename CT, typename MT>
        struct field_access
        {
            typedef MT CT::*member_object;
            typedef MT member_type;
            typedef CT object_type;
            template<member_object O>
            struct bind
            {
                constexpr static member_object member = O;
                constexpr member_type const& operator()(object_type const& o) const noexcept { return ((o).*(member)); }
                constexpr member_type& operator()(object_type& o) const noexcept { return ((o).*(member)); }
                constexpr member_type&& operator()(object_type&& o) const noexcept { return forward<member_type>((o).*(member)); }
                constexpr member_type& operator()(object_type* o) const noexcept { return ((o)->*(member)); }
                constexpr member_type const& operator()(object_type const* o) const noexcept { return ((o)->*(member)); }
            };
        };
        template<typename CT, typename MT, MT CT::*MO> using field_access_t = typename field_access<CT, MT>::template bind<MO>;
        template<typename CT, typename RT>
        struct getter_access
        {
            typedef RT (CT::*getter_fn)();
            typedef RT return_type;
            typedef CT object_type;
            template<getter_fn G>
            struct bind
            {
                constexpr static getter_fn getter = G;
                constexpr return_type operator()(object_type o) const noexcept { return ((o).*(getter))(); }
                constexpr return_type operator()(object_type* o) const noexcept { return ((o)->*(getter))(); }
            };
        };
        template<typename CT, typename RT, RT (CT::*G)()> using getter_t = typename getter_access<CT, RT>::template bind<G>;
    }
}
#endif