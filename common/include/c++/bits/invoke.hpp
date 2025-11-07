#ifndef __BITS_INVOKE
#define __BITS_INVOKE
#include "concepts"
#include "bits/move.h"
namespace std
{
	template<typename T, typename U = typename __inv_unwrap<T>::type> constexpr U&&  __invfwd(typename remove_reference<T>::type& __t) noexcept { return static_cast<U&&>(__t); }
	template<typename FT, typename RT, typename ... Args> constexpr RT __invoke_impl(__invoke_other, FT&& ft, Args&& ... args) { return forward<FT>(ft)(forward<Args>(args)...); }
	template<typename FT, typename RT, typename ST, typename ... Args> constexpr RT __invoke_impl(__invoke_memfun_ref, FT&& ft, ST&& st, Args&& ... args) { return (__invfwd<ST>(st).*ft)(forward<Args>(args)...); }
	template<typename FT, typename RT, typename ST, typename ... Args> constexpr RT __invoke_impl(__invoke_memfun_deref, FT&& ft, ST&& st, Args&& ... args) { return ((*forward<ST>(st)).*ft)(forward<Args>(args)...); }
	template<typename FT, typename ... Args> using __result_t = typename __invoke_result<FT, Args...>::type;
	template<typename FT, typename ... Args> using __invtype_t = typename __invoke_result<FT, Args...>::__invoke_type;
	template<typename FT, typename ... Args> constexpr __result_t<FT, Args...> __invoke(FT&& ft, Args&& ... args) noexcept(__is_nothrow_invocable<FT, Args...>::value) { return __invoke_impl<FT, __result_t<FT, Args...>>(__invtype_t<FT, Args...>{}, forward<FT>(ft), forward<Args>(args)...); }
	template<typename RT, typename FT, typename ... Args> constexpr RT __invoke_r(FT&& ft, Args&& ... args) noexcept(is_nothrow_invocable_r_v<RT, FT, Args...>) { if constexpr(is_void_v<RT>) __invoke_impl<FT, RT>(__invtype_t<FT, Args...>{}, forward<FT>(ft), forward<Args>(args)...); else return __invoke_impl<FT, RT>(__invtype_t<FT, Args...>{}, forward<FT>(ft), forward<Args>(args)...); }
}
#endif