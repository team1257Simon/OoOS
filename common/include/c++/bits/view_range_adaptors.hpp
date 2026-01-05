#ifndef __STD_VIEWS
#define __STD_VIEWS
/**
 * The various std::ranges and std::views structs.
 * Much of this is paraphrased from the gnu libstdc++ version, but not all of it (license in the project root directory to save space).
 * TODO: take, take_while, drop, drop_while, and all of the structures after join in the g++ library header other than ranges::to
 */
#include <bits/bind_back.hpp>
#include <bits/ranges_util.hpp>
#include <bits/refwrap.hpp>
#include <optional>
namespace std
{
	namespace ranges
	{
		namespace __detail
		{
			template<typename T, int DSC> struct __absent{};
			template<bool PB, typename T, int DSC = 0> using __maybe_present_t	= conditional_t<PB, T, __absent<T, DSC>>;
			template<bool CB, typename T> using __maybe_const_t					= conditional_t<CB, const T, T>;
		}
		namespace views
		{
			namespace __adaptor
			{
				template<typename T, typename ... Args> concept __adaptor_invocable		= requires { declval<T>()(declval<Args>()...); };
				template<typename T, typename ... Args> concept __partial_adaptor_valid	= (T::__arity > 1) && (sizeof...(Args) == (T::__arity - 1)) && (constructible_from<decay_t<Args>, Args> && ...);
				template<typename DT> struct __range_adaptor_closure;
				template<typename T, typename U> requires(!std::is_same_v<T, __range_adaptor_closure<U>>)
				void __is_adaptor_closure_helper(T const&, __range_adaptor_closure<U> const&); // not defined
				template<typename T> concept __is_adaptor_closure = requires(T t) { __adaptor::__is_adaptor_closure_helper(t, t); };
				template<typename T, typename U, typename R> concept __pipe_invocable = requires { std::declval<T>()(std::declval<U>()(std::declval<R>())); };
				template<typename ST, typename RT> requires(__is_adaptor_closure<ST> && __adaptor_invocable<ST, RT>) constexpr auto operator|(RT&& r, ST&& s);
				template<typename T, typename U> requires(__is_adaptor_closure<T> && __is_adaptor_closure<U>) constexpr auto operator|(T&& t, U&& u);
				template<typename T> concept __closure_has_simple_call_op = T::__has_simple_call_op;
				template<typename T, typename ... Args> concept __adaptor_has_simple_extra_args = T::__has_simple_extra_args || T::template __has_simple_extra_args<Args...>;
				template<typename DT>
				struct __range_adaptor_closure
				{
					template<typename ST, typename RT> requires(__is_adaptor_closure<ST> && __adaptor_invocable<ST, RT>) friend constexpr auto operator|(RT&& r, ST&& s);
					template<typename T, typename U> requires(__is_adaptor_closure<T> && __is_adaptor_closure<U>) friend constexpr auto operator|(T&& t, U&& u);
				};
				template<typename AT, typename ... Args>
				struct __partial_adaptor : __range_adaptor_closure<__partial_adaptor<AT, Args...>>
				{
					typedef __bind_back_expr<AT, Args...> __binder;
					[[no_unique_address]] __binder __bind;
					template<typename ... Ts> constexpr __partial_adaptor(int, Ts&& ... args) : __bind(0, AT(), forward<Ts>(args)...) {}
					#pragma GCC diagnostic push
					#pragma GCC diagnostic ignored "-Wc++23-extensions"
					template<typename ST, typename RT> requires(__adaptor_invocable<AT, RT, __like_t<ST, Args>...>)
					constexpr auto operator()(this ST&& self, RT&& r) { return __binder::__call(forward_like<ST, __partial_adaptor>(self).__bind, std::forward<RT>(r)); }
					#pragma GCC diagnostic pop
				};
				template<typename AT, typename ... Args>
				requires(__adaptor_has_simple_extra_args<AT, Args...> && (is_trivially_copy_constructible_v<Args> && ...))
				struct __partial_adaptor<AT, Args...> : __range_adaptor_closure<__partial_adaptor<AT, Args...>>
				{
					typedef __bind_back_expr<AT, Args...> __binder;
					[[no_unique_address]] __binder __bind;
					template<typename ... Ts> constexpr __partial_adaptor(int, Ts&& ... args) : __bind(0, AT(), forward<Ts>(args)...) {}
					template<typename ST, typename RT> requires(__adaptor_invocable<AT, RT, __like_t<ST, Args>...>)
					constexpr auto operator()(RT&& r) const { return __binder::__call(__bind, std::forward<RT>(r)); }
					constexpr static bool __has_simple_call_op = true;
				};
				template<typename T, typename U>
				struct __pipe : __range_adaptor_closure<__pipe<T, U>>
				{
					[[no_unique_address]] T t;
					[[no_unique_address]] U u;
					template<typename TT, typename UU>
					constexpr __pipe(TT&& __t, UU&& __u) : t(std::forward<TT>(__t)), u(std::forward<UU>(__u)) {}
					#pragma GCC diagnostic push
					#pragma GCC diagnostic ignored "-Wc++23-extensions"
					template<typename ST, typename RT> requires(__pipe_invocable<__like_t<ST, T>, __like_t<ST, U>, RT>)
					constexpr auto operator()(this ST&& self, RT&& r) { return (forward_like<ST, __pipe>(self).t(forward_like<ST, __pipe>(self).u(std::forward<RT>(r)))); }
					#pragma GCC diagnostic pop
				};
				template<typename T, typename U>
				requires(__closure_has_simple_call_op<T> && __closure_has_simple_call_op<U>)
				struct __pipe<T, U> : __range_adaptor_closure<__pipe<T, U>>
				{
					[[no_unique_address]] T t;
					[[no_unique_address]] U u;
					template<typename TT, typename UU>
					constexpr __pipe(TT&& __t, UU&& __u) : t(std::forward<TT>(__t)), u(std::forward<UU>(__u)) {}
					template<typename RT> requires(__pipe_invocable<T const&, U const&, RT>)
					constexpr auto operator()(RT&& r) const { return t(u(std::forward<RT>(r))); };
					constexpr static bool __has_simple_call_op = true;
				};
				#pragma GCC diagnostic push
				#pragma GCC diagnostic ignored "-Wdangling-reference"
				template<typename ST, typename RT> requires(__is_adaptor_closure<ST> && __adaptor_invocable<ST, RT>)
				constexpr auto operator|(RT&& r, ST&& s) { return std::forward<ST>(s)(std::forward<RT>(r)); }
				template<typename T, typename U> requires(__is_adaptor_closure<T> && __is_adaptor_closure<U>)
				constexpr auto operator|(T&& t, U&& u) { return __pipe<decay_t<T>, decay_t<U>>(std::forward<T>(t), std::forward<U>(u)); }
				#pragma GCC diagnostic pop
				template<typename DT>
				struct __range_adaptor {
					template<typename ... Args> requires(__partial_adaptor_valid<DT, Args...>)
					constexpr auto operator()(Args&& ... args) const { return __partial_adaptor<DT, decay_t<Args>...>(0, std::forward<Args>(args)...); }
				};
			}
			namespace __detail
			{
				template<typename RT> concept __ref_viewable = requires { ref_view(std::declval<RT>()); };
				template<typename RT> concept __owning_viewable = requires { owning_view(std::declval<RT>()); };
				template<typename RT> concept __valid_all = viewable_range<RT> && (view<decay_t<RT>> || __ref_viewable<RT> || __owning_viewable<RT>);
			}
			struct __all : __adaptor::__range_adaptor_closure<__all>
			{
				template<typename RT>
				consteval static bool __noexcept()
				{
					if constexpr(view<decay_t<RT>>)
						return is_nothrow_constructible_v<decay_t<RT>, RT>;
					else if constexpr(__detail::__ref_viewable<RT>)
						return true;
					else return noexcept(owning_view(std::declval<RT>()));
				}
				template<__detail::__valid_all RT>
				[[nodiscard]] constexpr auto operator()(RT&& r) const
				{
					if constexpr(view<decay_t<RT>>)
						return std::forward<RT>(r);
					else if constexpr(__detail::__ref_viewable<RT>)
						return ref_view(std::forward<RT>(r));
					else return owning_view(std::forward<RT>(r));
				}
			};
			constexpr inline __all all;
			template<viewable_range RT> using all_t = decltype(all(std::declval<RT>()));
		}
		template<typename DT> requires(is_class_v<DT> && same_as<DT, remove_cv_t<DT>>) class range_adaptor_closure : public views::__adaptor::__range_adaptor_closure<DT> {};
		namespace __detail
		{
			namespace __concept_structs
			{
				template<typename T> using __input_range			= bool_constant<input_range<T>>;
				template<typename T> using __bidirectional_range	= bool_constant<bidirectional_range<T>>;
				template<typename T> using __forward_range			= bool_constant<forward_range<T>>;
				template<typename T> using __common_range			= bool_constant<common_range<T>>;
				template<typename T> using __common_forward			= __and_<__common_range<T>, __forward_range<T>>;
			}
			template<typename T> concept __boxable = move_constructible<T> && is_object_v<T>;
			template<__boxable T>
			struct __box : std::optional<T>
			{
				using std::optional<T>::optional;
				constexpr __box() noexcept(is_nothrow_default_constructible_v<T>) requires(default_initializable<T>) : std::optional<T>(std::in_place){}
				__box(__box const&) = default;
				__box(__box&&) = default;
				constexpr __box& operator=(__box const& that) noexcept(is_nothrow_copy_constructible_v<T>)
				requires(!copyable<T> && copy_constructible<T>)
				{
					if(this != std::addressof(that))
					{
						if(static_cast<bool>(that))
							this->emplace(*that);
						else this->reset();
					}
					return *this;
				}
				constexpr __box& operator=(__box&& that) noexcept(is_nothrow_move_constructible_v<T>)
				requires(!movable<T>)
				{
					if(this != std::addressof(that))
					{
						if(static_cast<bool>(that))
							this->emplace(std::move(*that));
						else this->reset();
					}
					return *this;
				}
			};
			template<typename T>
			concept __easy_boxable = (copy_constructible<T> && (copyable<T> || (is_nothrow_move_constructible_v<T> && is_nothrow_copy_constructible_v<T>))) || ((!copy_constructible<T>) && (movable<T> || is_nothrow_move_constructible_v<T>));
			template<__boxable T> requires(__easy_boxable<T>)
			struct __box<T>
			{
			private:
				[[no_unique_address]] T __value = T();
			public:
				__box() requires(default_initializable<T>) = default;
				constexpr explicit __box(T const& t) noexcept(is_nothrow_copy_constructible_v<T>) requires(copy_constructible<T>) : __value(t) {}
				constexpr explicit __box(T&& t) noexcept(is_nothrow_move_constructible_v<T>) requires(move_constructible<T>) : __value(std::move(t)) {}
				template<typename ... Args> requires(constructible_from<T, Args...>)
				constexpr explicit __box(in_place_t, Args&& ... args) noexcept(is_nothrow_constructible_v<T, Args...>) : __value(std::forward<Args>(args)...) {}
				__box(__box const&) = default;
				__box(__box&&) = default;
				__box& operator=(__box const&) requires(copyable<T>) = default;
				__box& operator=(__box&&) requires(movable<T>) = default;
				constexpr __box& operator=(__box const& that) noexcept requires(!copyable<T> && copy_constructible<T>)
				{
					static_assert(is_nothrow_copy_constructible_v<T>);
					if(this != std::addressof(that)) {
						__value.~T();
						std::construct_at(std::addressof(__value), *that);
					}
					return *this;
				}
				constexpr __box& operator=(__box&& that) noexcept requires(!movable<T>)
				{
					static_assert(is_nothrow_move_constructible_v<T>);
					if(this != std::addressof(that)) {
						__value.~T();
						std::construct_at(std::addressof(__value), std::move(*that));
					}
					return *this;
				}
				constexpr bool has_value() const noexcept { return true; }
				constexpr T& operator*()& noexcept { return __value; }
				constexpr T const& operator*() const& noexcept { return __value; }
				constexpr T&& operator*()&& noexcept { return std::move(__value); }
				constexpr const T&& operator*() const&& noexcept { return std::move(__value); }
				constexpr T* operator->() noexcept { return std::addressof(__value); }
				constexpr T const* operator->() const noexcept { return std::addressof(__value); }
			};
			namespace __fn_handle
			{
				template<typename FT>
				struct __inplace
				{
				private:
					[[no_unique_address]] FT __f = FT();
				public:
					__inplace() = default;
					constexpr explicit __inplace(FT f) noexcept : __f(f) {}
					template<typename ... ITs>
					constexpr decltype(auto) __deref(ITs const& ... i) const noexcept(noexcept(__f(*i...))) { return __f(*i...); }
					template<typename DT, typename ... ITs>
					constexpr decltype(auto) __subscript(DT const& n, ITs const& ... i) const noexcept(noexcept(__f(i[iter_difference_t<ITs>(n)]...))) { return __f(i[iter_difference_t<ITs>(n)]...); }
				};
				template<typename FT>
				struct __mem_ptr_inplace
				{
				private:
					FT __ptr = nullptr;
				public:
					__mem_ptr_inplace() = default;
					constexpr explicit __mem_ptr_inplace(FT f) noexcept : __ptr(f) {}
					template<typename ... ITs>
					constexpr decltype(auto) __deref(ITs const& ... i) const noexcept(noexcept(std::__invoke(__ptr, *i...))) { return std::__invoke(__ptr, *i...); }
					template<typename DT, typename ... ITs>
					constexpr decltype(auto) __subscript(DT const& n, ITs const& ... i) const noexcept(noexcept(std::__invoke(__ptr, i[iter_difference_t<ITs>(n)]...))) { return std::__invoke(__ptr, i[iter_difference_t<ITs>(n)]...); }
				};
				template<typename FT>
				struct __via_ptr
				{
				private:
					FT* __ptr = nullptr;
				public:
					__via_ptr() = default;
					constexpr explicit __via_ptr(FT& f) noexcept : __ptr(std::addressof(f)) {}
					template<typename GT> requires(!is_const_v<GT> && is_same_v<GT const, FT>)
					constexpr __via_ptr(__via_ptr<GT> that) noexcept : __ptr(that.__ptr) {}
					template<typename ... ITs>
					constexpr decltype(auto) __deref(ITs const& ... i) const noexcept(noexcept((*__ptr)(*i...))) { return (*__ptr)(*i...); }
					template<typename DT, typename ... ITs>
					constexpr decltype(auto) __subscript(DT const& n, ITs const& ... i) const noexcept(noexcept((*__ptr)(i[iter_difference_t<ITs>(n)]...))) { return (*__ptr)(i[iter_difference_t<ITs>(n)]...); }
				};
				template<typename FT>
				struct __static_call
				{
					__static_call() = default;
					constexpr explicit __static_call(FT const&) noexcept {}
					template<typename ... ITs>
					constexpr decltype(auto) __deref(ITs const& ... i) const noexcept(noexcept(FT::operator()(*i...))) { return FT::operator()(*i...); }
					template<typename DT, typename ... ITs>
					constexpr decltype(auto) __subscript(DT const& n, ITs const& ... i) const noexcept(noexcept(FT::operator()(i[iter_difference_t<ITs>(n)]...))) { return FT::operator()(i[iter_difference_t<ITs>(n)]...); }
				};
				template<typename T, typename ... ITs> concept __static_callable = requires(ITs const& ... i) { T::operator()(*i...); };
				template<typename FT> concept __should_inplace = is_function_v<remove_pointer_t<remove_cv_t<FT>>> || (is_object_v<remove_cv_t<FT>> && is_empty_v<FT>);
				template<typename FT, typename ... ITs>
				consteval auto __select()
				{
					using F = remove_cv_t<FT>;
					if constexpr(is_member_pointer_v<F>) return __mem_ptr_inplace<F>();
					else if constexpr(__should_inplace<F>) return __inplace<F>();
					else if constexpr(__static_callable<F>) return __static_call<F>();
					else return __via_ptr<F>();
				}
			}
			template<typename FT, typename ... ITs> using __func_handle_t = decltype(__fn_handle::__select<FT, ITs...>());
			template<typename> struct __filter_view_iter_cat{};
			template<forward_range RT>
			struct __filter_view_iter_cat<RT>
			{
			private:
				using CT = typename iterator_traits<iterator_t<RT>>::iterator_category;
				static auto __iter_cat()
				{
					if constexpr(derived_from<CT, bidirectional_iterator_tag>) return bidirectional_iterator_tag();
					else if constexpr(derived_from<CT, forward_iterator_tag>) return forward_iterator_tag();
					else return CT();
				}
			public:
				typedef decltype(__iter_cat()) iterator_category;
			};
			template<typename> struct __non_propagating_cache{};
			template<object T>
			struct __non_propagating_cache<T> : protected __optional_base<T>
			{
				__non_propagating_cache() = default;
				using __optional_base<T>::__reset;
				constexpr __non_propagating_cache(__non_propagating_cache const&) noexcept {}
				constexpr __non_propagating_cache(__non_propagating_cache&& that) noexcept { that.__reset(); }
				constexpr explicit operator bool() const noexcept { return this->__is_present(); }
				constexpr T& operator*() noexcept { return this->__get(); }
				constexpr T const& operator*() const noexcept { return this->__get(); }
				constexpr __non_propagating_cache& operator=(__non_propagating_cache const& that) noexcept
				{
					if(this != std::addressof(that))
						this->__reset();
					return *this;
				}
				constexpr __non_propagating_cache& operator=(__non_propagating_cache&& that) noexcept
				{
					this->__reset();
					that.__reset();
					return *this;
				}
				constexpr __non_propagating_cache& operator=(T t)
				{
					this->__reset();
					this->__payload.__construct(std::move(t));
					return *this;
				}
				template<typename IT>
				constexpr T& __emplace_deref(IT const& i)
				{
					this->__reset();
					auto fn = [](auto& x) { return *x; };
					this->__payload.__apply(__optional_ftor(fn), i);
					return this->__get();
				}
			};
			template<range RT>
			struct __cached_position
			{
				constexpr bool __has_value() const { return false; }
				constexpr iterator_t<RT> __get(RT const&) const { __libk_assert(false); __builtin_unreachable(); }
				constexpr void __set(RT const&, iterator_t<RT> const&) const {}
			};
			template<forward_range RT>
			struct __cached_position<RT> : protected __non_propagating_cache<iterator_t<RT>>
			{
				constexpr bool __has_value() const { return this->__is_present(); }
				constexpr iterator_t<RT> __get(RT const&) const { __libk_assert(this->__has_value()); return **this; }
				constexpr void __set(RT const&, iterator_t<RT> const& i) { this->__construct(i); }
			};
			template<random_access_range RT>
			struct __cached_position<RT>
			{
			private:
				range_difference_t<RT> __pos = -1Z;
			public:
				__cached_position() = default;
				constexpr __cached_position(__cached_position const&) = default;
				constexpr __cached_position(__cached_position&& that) noexcept : __pos(that.__pos) { that.__pos = -1Z; }
				constexpr __cached_position& operator=(__cached_position const&) = default;
				constexpr __cached_position& operator=(__cached_position&& that) noexcept { return (*this = __cached_position(std::move(that))); }
				constexpr bool __has_value() const { return __pos >= 0Z; }
				constexpr iterator_t<RT> __get(RT& r) const { __libk_assert(this->__has_value()); return ranges::begin(r) + __pos; }
				constexpr void __set(RT& r, iterator_t<RT> const& i) { __libk_assert(!this->__has_value()); __pos = i - ranges::begin(r); }
			};
			template<typename CT, typename RT> concept __toable = requires{ requires(!input_range<CT> || convertible_to<range_reference_t<RT>, range_value_t<CT>>); };
		}
		template<input_range RT, indirect_unary_predicate<iterator_t<RT>> QT> requires(view<RT> && is_object_v<QT>)
		class filter_view : public view_interface<filter_view<RT, QT>>
		{
			RT __base = RT();
			struct __sentinel;
			[[no_unique_address]] __detail::__box<QT> __pred;
			[[no_unique_address]] __detail::__cached_position<RT> __cached_begin;
			struct __iterator : __detail::__filter_view_iter_cat<RT>
			{
			private:
				constexpr static auto __iter_concept()
				{
					if constexpr(bidirectional_range<RT>) return bidirectional_iterator_tag();
					else if constexpr(forward_range<RT>) return forward_iterator_tag();
					else return input_iterator_tag();
				}
				friend filter_view;
				using __range_iter		= iterator_t<RT>;
				__range_iter __curr{};
				filter_view* __parent{};
			public:
				typedef decltype(__iter_concept()) iterator_concept;
				typedef range_value_t<RT> value_type;
				typedef range_difference_t<RT> difference_type;
				__iterator() requires(default_initializable<__range_iter>) = default;
				constexpr __iterator(filter_view* parent, __range_iter cur) : __curr(cur), __parent(parent) {}
				constexpr __range_iter const& base() const& noexcept { return __curr; }
				constexpr __range_iter&& base()&& { return std::move(__curr); }
				constexpr range_reference_t<RT> operator*() const { return *__curr; }
				constexpr __range_iter operator->() requires(__detail::__has_arrow<__range_iter> && copyable<__range_iter>) { return __curr; }
				constexpr __iterator& operator++() { return __curr = ranges::find_if(std::move(++__curr), ranges::end(__parent->__base), std::ref(*__parent->__pred)), *this; }
				constexpr void operator++(int) { ++*this; }
				constexpr __iterator operator++(int) requires(forward_range<RT>) { auto i = *this; ++*this; return i; }
				constexpr __iterator& operator--() requires(bidirectional_range<RT>) { do --__curr; while(!std::__invoke(*__parent->__pred, *__curr)); return *this; }
				friend constexpr bool operator==(__iterator const& __this, __iterator const& __that) requires(equality_comparable<__range_iter>) { return __this.__curr == __that.__curr; }
			};
			struct __sentinel
			{
			private:
				sentinel_t<RT> __val{};
				constexpr bool __eq(__iterator const& i) const { return i.base() == __val; }
			public:
				__sentinel() = default;
				constexpr explicit __sentinel(filter_view* v) : __val(ranges::end(v->__base)) {}
				constexpr sentinel_t<RT> base() const { return __val; }
				friend constexpr bool operator==(__iterator const& __this, __sentinel const& __that) { return __that.__eq(__this); }
				friend constexpr bool operator==(__sentinel const& __this, __iterator const& __that) { return __this.__eq(__that); }
			};
		public:
			filter_view() requires(default_initializable<RT> && default_initializable<QT>) = default;
			constexpr explicit filter_view(RT r, QT pred) : __base(std::move(r)), __pred(std::move(pred)) {}
			constexpr RT base() const& requires(copy_constructible<RT>) { return __base; }
			constexpr RT base()&& { return std::move(__base); }
			constexpr QT const& pred() const { return *__pred; }
			constexpr __iterator begin()
			{
				if(__cached_begin.__has_value())
					return __iterator(this, __cached_begin.__get(__base));
				__libk_assert(__pred.has_value());
				auto i = ranges::find_if(ranges::begin(__base), ranges::end(__base), std::ref(*__pred));
				__cached_begin.__set(__base, i);
				return __iterator(this, std::move(i));
			}
			constexpr auto end()
			{
				if constexpr(common_range<RT>)
					return __iterator(this, ranges::end(__base));
				else return __sentinel(this);
			}
		};
		template<input_range RT, move_constructible FT>
		requires(view<RT> && is_object_v<FT> && regular_invocable<FT&, range_reference_t<RT>> && std::__detail::__can_reference<invoke_result_t<FT&, range_reference_t<RT>>>)
		class transform_view : public view_interface<transform_view<RT, FT>>
		{
			template<bool CB> using __base = __detail::__maybe_const_t<CB, RT>;
			template<bool> struct __iter_cat{};
			template<bool CB> requires(forward_range<__base<CB>>)
			struct __iter_cat<CB>
			{
			private:
				using __cbase	= transform_view::__base<CB>;
				using __res		= invoke_result_t<__detail::__maybe_const_t<CB, FT>&, range_reference_t<__cbase>>;
				static auto __cat()
				{
					if constexpr(is_reference_v<__res>)
					{
						using CT = typename iterator_traits<iterator_t<__cbase>>::iterator_category;
						if constexpr(derived_from<CT, contiguous_iterator_tag>) return random_access_iterator_tag();
						else return CT();
					}
					else return input_iterator_tag();
				}
			public:
				typedef decltype(__cat()) iterator_category;
			};
			RT __range{};
			[[no_unique_address]] __detail::__box<FT> __fn;
			template<bool CB> struct __sentinel;
			template<bool CB>
			struct __iterator : __iter_cat<CB>
			{
			private:
				using __parent		= __detail::__maybe_const_t<CB, transform_view>;
				using __cbase		= transform_view::__base<CB>;
				using __base_iter	= iterator_t<__cbase>;
				using __func_handle	= __detail::__func_handle_t<__detail::__maybe_const_t<CB, FT>, __base_iter>;
				__base_iter __curr{};
				[[no_unique_address]] __func_handle __fn;
				consteval static auto __iter_concept()
				{
					if constexpr(random_access_range<__cbase>) return random_access_iterator_tag();
					else if constexpr(bidirectional_range<__cbase>) return bidirectional_iterator_tag();
					else if constexpr(forward_range<__cbase>) return forward_iterator_tag();
					else return input_iterator_tag();
				}
			public:
				typedef decltype(__iter_concept()) iterator_concept;
				typedef remove_cvref_t<invoke_result_t<__detail::__maybe_const_t<CB, FT>&, range_reference_t<__cbase>>> value_type;
				typedef range_difference_t<__cbase> difference_type;
				__iterator() requires(default_initializable<__base_iter>) = default;
				constexpr __iterator(__func_handle f, __base_iter i) : __curr(std::move(i)), __fn(f) {}
				constexpr __iterator(__parent p, __base_iter i) : __curr(std::move(i)), __fn(*p->__fn) {}
				constexpr __iterator(__iterator<!CB> i) requires(CB && convertible_to<iterator_t<RT>, __base_iter>) : __curr(std::move(i.__curr)), __fn(i.__fn) {}
				constexpr __base_iter& base() const& noexcept { return __curr; }
				constexpr __base_iter&& base()&& noexcept { return std::move(__curr); }
				constexpr decltype(auto) operator*() const noexcept(noexcept(__fn.__deref(__curr))) { return __fn.__deref(__curr); }
				constexpr decltype(auto) operator[](difference_type n) const requires(random_access_range<__cbase>) { return __fn.__subscript(n, __curr); }
				constexpr __iterator& operator++() { ++__curr; return *this; }
				constexpr void operator++(int) { ++__curr; }
				constexpr __iterator operator++(int) requires(forward_range<__cbase>) { auto i = *this; ++*this; return i; }
				constexpr __iterator& operator--() requires(bidirectional_range<__cbase>) { --__curr; return *this; }
				constexpr __iterator operator--(int) requires(bidirectional_range<__cbase>) { auto i = *this; --*this; return i; }
				constexpr __iterator& operator+=(difference_type n) requires(random_access_range<__cbase>) { __curr += n; return *this; }
				constexpr __iterator& operator-=(difference_type n) requires(random_access_range<__cbase>) { __curr -= n; return *this; }
				friend constexpr bool operator==(__iterator const& __this, __iterator const& __that) requires(equality_comparable<__base_iter>) { return __this.__curr == __that.__curr; }
				friend constexpr bool operator<(__iterator const& __this, __iterator const& __that) requires(random_access_range<__cbase>) { return __this.__curr < __that.__curr; }
				friend constexpr bool operator>(__iterator const& __this, __iterator const& __that) requires(random_access_range<__cbase>) { return __this.__curr > __that.__curr; }
				friend constexpr bool operator<=(__iterator const& __this, __iterator const& __that) requires(random_access_range<__cbase>) { return !(__this > __that); }
				friend constexpr bool operator>=(__iterator const& __this, __iterator const& __that) requires(random_access_range<__cbase>) { return !(__this < __that); }
				friend constexpr auto operator<=>(__iterator const& __this, __iterator const& __that) requires(random_access_range<__cbase> && three_way_comparable<__base_iter>) { return __this.__curr <=> __that.__curr; }
				friend constexpr __iterator operator+(__iterator i, difference_type n) requires(random_access_range<__cbase>) { return __iterator(i.__fn, i.__curr + n); }
				friend constexpr __iterator operator+(difference_type n, __iterator i) requires(random_access_range<__cbase>) { return __iterator(i.__fn, i.__curr + n); }
				friend constexpr __iterator operator-(__iterator i, difference_type n) requires(random_access_range<__cbase>) { return __iterator(i.__fn, i.__curr - n); }
				friend constexpr difference_type operator-(__iterator const& __this, __iterator const& __that) requires(sized_sentinel_for<__base_iter, __base_iter>) { return __this.__curr - __that.__curr; }
				friend __iterator<!CB>;
				template<bool> friend struct __sentinel;
			};
			template<bool CB>
			struct __sentinel
			{
			private:
				using __parent		= __detail::__maybe_const_t<CB, transform_view>;
				using __cbase		= transform_view::__base<CB>;
				sentinel_t<__cbase> __val{};
				template<bool DB> constexpr auto __dist(__iterator<DB> const& i) const { return __val - i.__curr; }
				template<bool DB> constexpr bool __eq(__iterator<DB> const& i) const { return __val == i.__curr; }
			public:
				__sentinel() = default;
				constexpr explicit __sentinel(sentinel_t<__cbase> v) : __val(v) {}
				constexpr __sentinel(__sentinel<!CB> i) requires(CB && convertible_to<sentinel_t<RT>, sentinel_t<__cbase>>) : __val(std::move(i.__val)) {}
				constexpr sentinel_t<__cbase> base() const { return __val; }
				template<bool DB> requires(sentinel_for<sentinel_t<__cbase>, iterator_t<__detail::__maybe_const_t<DB, RT>>>)
				friend constexpr bool operator==(__iterator<DB> const& __this, __sentinel const& __that) { return __that.__eq(__this); }
				template<bool DB, typename B2 = __detail::__maybe_const_t<DB, RT>> requires(sized_sentinel_for<__cbase, iterator_t<B2>>)
				friend constexpr range_difference_t<B2> operator-(__iterator<DB> const& __this, __sentinel const& __that) { return -__that.__dist(__this); }
				template<bool DB, typename B2 = __detail::__maybe_const_t<DB, RT>> requires(sized_sentinel_for<__cbase, iterator_t<B2>>)
				friend constexpr range_difference_t<B2> operator-(__sentinel const& __this, __iterator<DB> const& __that) { return __this.__dist(__that); }
				friend __sentinel<!CB>;
			};
		public:
			transform_view() requires(default_initializable<RT> && default_initializable<FT>) = default;
			constexpr explicit transform_view(RT r, FT f) : __range(std::move(r)), __fn(std::move(f)) {}
			constexpr RT base() const& requires(copy_constructible<RT>) { return __range; }
			constexpr RT base()&& { return std::move(__range); }
			constexpr __iterator<false> begin() { return __iterator<false>(this, ranges::begin(__range)); }
			constexpr __sentinel<false> end() { return __sentinel<false>(ranges::end(__range)); }
			constexpr __iterator<false> end() requires(common_range<RT>) { return __iterator<false>(this, ranges::end(__range)); }
			constexpr __iterator<true> begin() const requires(range<RT const> && regular_invocable<FT const&, range_reference_t<RT const>>) { return __iterator<true>(this, ranges::begin(__range)); }
			constexpr __sentinel<true> end() const requires(range<RT const> && regular_invocable<FT const&, range_reference_t<RT const>>) { return __sentinel<true>(ranges::end(__range)); }
			constexpr __iterator<true> end() const requires(common_range<RT const> && regular_invocable<FT const&, range_reference_t<RT const>>) { return __iterator<true>(this, ranges::end(__range)); }
			constexpr auto size() requires(sized_range<RT>) { return ranges::size(__range); }
			constexpr auto size() const requires(sized_range<RT const>) { return ranges::size(__range); }
		};
		template<input_range VT> requires(view<VT> && input_range<range_reference_t<VT>>)
		class join_view : public view_interface<join_view<VT>>
		{
			template<typename T> using __bidirectional_range	= __detail::__concept_structs::__bidirectional_range<T>;
			template<typename T> using __common_range			= __detail::__concept_structs::__common_range<T>;
			template<typename T> using __forward_range			= __detail::__concept_structs::__forward_range<T>;
			template<typename T> using __input_range			= __detail::__concept_structs::__input_range<T>;
			template<typename T> using __common_forward			= __detail::__concept_structs::__common_forward<T>;
			typedef range_reference_t<VT> __inner_range;
			template<bool CB> using __outer			= __detail::__maybe_const_t<CB, VT>;
			template<bool CB> using __inner			= range_reference_t<__outer<CB>>;
			template<bool CB> using __outer_iter	= iterator_t<__outer<CB>>;
			template<bool CB> using __inner_iter	= iterator_t<__inner<CB>>;
			template<bool CB> constexpr static bool __is_ref_glvalue = is_reference_v<__inner<CB>>;
			template<bool CB> struct __iter_cat{};
			template<bool CB> requires(__is_ref_glvalue<CB> && forward_range<__outer<CB>> && forward_range<__inner<CB>>)
			struct __iter_cat<CB>
			{
			private:
				constexpr static auto __cat()
				{
					using OI	= join_view::__outer_iter<CB>;
					using II	= join_view::__inner_iter<CB>;
					using OC	= typename iterator_traits<OI>::iterator_category;
					using IC	= typename iterator_traits<II>::iterator_category;
					if constexpr(derived_from<OC, bidirectional_iterator_tag> && derived_from<IC, bidirectional_iterator_tag> && common_range<__inner<CB>>)
						return bidirectional_iterator_tag();
					else if constexpr(derived_from<OC, forward_iterator_tag> && derived_from<IC, forward_iterator_tag>)
						return forward_iterator_tag();
					else return input_iterator_tag();
				}
				// constexpr static auto __dog() = JK;
			public:
				typedef decltype(__cat()) iterator_category;
			};
			VT __base{};
			[[no_unique_address]] __detail::__maybe_present_t<!forward_range<VT>, __detail::__non_propagating_cache<iterator_t<VT>>> __outer_pos;
			[[no_unique_address]] __detail::__non_propagating_cache<remove_cv_t<__inner_range>> __cur_inner;
			template<bool CB> struct __sentinel;
			template<bool CB>
			struct __iterator : __iter_cat<CB>
			{
				friend join_view;
			private:
				using __parent_type	= __detail::__maybe_const_t<CB, join_view>;
				using __base		= join_view::__outer<CB>;
				constexpr static bool __is_ref_glvalue	= join_view::__is_ref_glvalue<CB>;
				constexpr static bool __is_all_bidir	= all_satisfy<__bidirectional_range, __base, range_reference_t<__base>>;
				constexpr static bool __is_all_forward	= all_satisfy<__forward_range, __base, range_reference_t<__base>>;
				constexpr static bool __req_bidir		= __is_ref_glvalue && __is_all_bidir && common_range<range_reference_t<__base>>;
				constexpr static bool __req_forward		= __is_ref_glvalue && __is_all_forward;
				constexpr static auto __concept()
				{
					if constexpr(__req_bidir)
						return bidirectional_iterator_tag();
					else if constexpr(__req_forward)
						return forward_iterator_tag();
					else return input_iterator_tag();
				}
				typedef join_view::__outer_iter<CB> __outer_iter;
				typedef join_view::__inner_iter<CB> __inner_iter;
				[[no_unique_address]] __detail::__maybe_present_t<forward_range<__base>, __outer_iter> __outer_pos	= decltype(__outer_pos)();
				conditional_t<forward_iterator<__inner_iter>, __inner_iter, optional<__inner_iter>> __inner_pos		= decltype(__inner_pos)();
				__parent_type* __parent																				= nullptr;
				constexpr __outer_iter& __get_outer()
				{
					if constexpr(forward_range<__base>)
						return __outer_pos;
					else return *__parent->__outer_pos;
				}
				constexpr __outer_iter const& __get_outer() const
				{
					if constexpr(forward_range<__base>)
						return __outer_pos;
					else return *__parent->__outer_pos;
				}
				constexpr __inner_iter& __get_inner() noexcept
				{
					if constexpr(forward_iterator<__inner_iter>)
						return __inner_pos;
					else return *__inner_pos;
				}
				constexpr void __sate()
				{
					auto update_inner = [this](iterator_t<__base> const& i) -> auto&&
					{
						if constexpr(__is_ref_glvalue)
							return *i;
						else return __parent->__cur_inner.__emplace_deref(i);
					};
					for(__outer_iter& o = __get_outer(); o != ranges::end(__parent->__base); ++o)
					{
						auto&& i	= update_inner(o);
						__inner_pos	= ranges::begin(i);
						if(__inner_pos != ranges::end(i)) return;
					}
					if constexpr(__is_ref_glvalue)
					{
						if constexpr(forward_iterator<__inner_iter>)
							__inner_pos	= __inner_iter();
						else __inner_pos.reset();
					}
				}
				constexpr decltype(auto) __inner_end()
				{
					if constexpr(__is_ref_glvalue)
						return ranges::end(*__get_outer());
					else return ranges::end(*__parent->__cur_inner);
				}
				constexpr __iterator(__parent_type* p, __outer_iter o) requires(forward_range<__base>) : __outer_pos(std::move(o)), __parent(p) { __sate(); }
				constexpr explicit __iterator(__parent_type* p) requires(!forward_range<__base>) : __parent(p) { __sate(); }
			public:
				typedef decltype(__concept()) iterator_concept;
				typedef range_value_t<range_reference_t<__base>> value_type;
				typedef common_type_t<range_difference_t<__base>, range_difference_t<range_reference_t<__base>>> difference_type;
				__iterator() = default;
				constexpr __iterator(__iterator<!CB> i)
				requires(CB && convertible_to<iterator_t<VT>, __outer_iter> && convertible_to<iterator_t<__inner_range>, __inner_iter>) :
					__outer_pos(i.__outer_pos),
					__inner_pos(i.__inner_pos),
					__parent(i.__parent)
				{}
				constexpr decltype(auto) operator*() const { return *__get_inner(); }
				constexpr __inner_iter operator->() const requires(__detail::__has_arrow<__inner_iter> && copyable<__inner_iter>) { return __get_inner(); }
				constexpr __iterator& operator++()
				{
					if(++__get_inner() == __inner_end()) {
						++__get_outer();
						__sate();
					}
					return *this;
				}
				constexpr __iterator& operator--()
				requires(__req_bidir)
				{
					if(__outer_pos	== ranges::end(__parent->__base))
						__inner_pos	= ranges::end(std::backward(*--__outer_pos));
					while(__get_inner()	== ranges::begin(std::backward(*--__outer_pos)))
						__get_inner()	= ranges::end(std::backward(*--__outer_pos));
					--__get_inner();
					return *this;
				}
				constexpr void operator++(int) { ++*this; }
				constexpr __iterator operator++(int) requires(__req_forward)
				{
					auto i	= *this;
					++*this;
					return i;
				}
				constexpr __iterator operator--(int)
				requires(__req_bidir)
				{
					auto i	= *this;
					--*this;
					return i;
				}
				friend constexpr bool operator==(__iterator const& __this, __iterator const& __that)
				requires(__is_ref_glvalue && forward_range<__base> && equality_comparable<__inner_iter>)
				{
					if(__this.__outer_pos != __that.__outer_pos)
						return false;
					return __this.__inner_pos == __that.__inner_pos;
				}
				friend __iterator<!CB>;
				template<bool> friend struct __sentinel;
			};
			template<bool CB>
			struct __sentinel
			{
			private:
				using __parent_type	= __detail::__maybe_const_t<CB, join_view>;
				using __base		= join_view::__outer<CB>;
				sentinel_t<__base> __val{};
				template<bool DB> constexpr bool __eq(__iterator<DB> const& i) const { return i.__get_outer() == __val; }
			public:
				__sentinel() = default;
				constexpr explicit __sentinel(__parent_type* p) : __val(ranges::end(p->__base)) {}
				constexpr __sentinel(__sentinel<!CB> s) requires(CB && convertible_to<sentinel_t<VT>, sentinel_t<__base>>) : __val(std::move(s.__val)) {}
				template<bool DB> requires(sentinel_for<sentinel_t<__base>, iterator_t<__detail::__maybe_const_t<DB, VT>>>)
				friend constexpr bool operator==(__iterator<DB> const& __this, __sentinel const& __that) { return __that.__eq(__this); }
				template<bool DB> requires(sentinel_for<sentinel_t<__base>, iterator_t<__detail::__maybe_const_t<DB, VT>>>)
				friend constexpr bool operator==(__sentinel const& __this, __iterator<DB> const& __that) { return __this.__eq(__that); }
				friend __sentinel<!CB>;
			};
			constexpr static bool __const_iters					= satisfies_all<__inner<true>, is_reference, __input_range> && forward_range<VT const>;
			template<bool CB> constexpr static bool __iter_end	= all_satisfy<__common_forward, __inner<CB>, __outer<CB>> && is_reference_v<__inner<CB>>;
		public:
			join_view() requires(default_initializable<VT>) = default;
			constexpr explicit join_view(VT b) : __base(std::move(b)) {}
			constexpr VT base() const& requires(copy_constructible<VT>) { return __base; }
			constexpr VT base()&& { return std::move(__base); }
			constexpr auto begin() const requires(__const_iters) { return __iterator<true>(this, ranges::begin(__base)); }
			constexpr auto begin()
			{
				if constexpr(forward_range<VT>) {
					constexpr bool __use_const	= (__detail::__simple_view<VT> && is_reference_v<range_reference_t<VT>>);
					return __iterator<__use_const>(this, ranges::begin(__base));
				}
				else {
					__outer_pos	= ranges::begin(__base);
					return __iterator<false>(this);
				}
			}
			constexpr auto end() const requires(__const_iters)
			{
				if constexpr(__iter_end<true>)
					return __iterator<true>(this, ranges::end(__base));
				else return __sentinel<true>(this);
			}
			constexpr auto end()
			{
				if constexpr(__iter_end<false>)
					return __iterator<__detail::__simple_view<VT>>(this, ranges::end(__base));
				else return __sentinel<__detail::__simple_view<VT>>(this);
			}
		};
		template<typename RT, typename QT> filter_view(RT&&, QT) -> filter_view<views::all_t<RT>, QT>;
		template<typename RT, typename FT> transform_view(RT&&, FT) -> transform_view<views::all_t<RT>, FT>;
		template<typename RT> explicit join_view(RT&&) -> join_view<views::all_t<RT>>;
		namespace views
		{
			namespace __detail
			{
				template<typename RT, typename QT> concept __filter_viewable = requires { filter_view(std::declval<RT>(), std::declval<QT>()); };
				template<typename RT, typename FT> concept __transform_viewable = requires { transform_view(std::declval<RT>(), std::declval<FT>()); };
				template<typename RT> concept __join_viewable = requires { join_view<all_t<RT>>(std::declval<RT>()); };
			}
			struct __filter : __adaptor::__range_adaptor<__filter>
			{
				constexpr static int __arity					= 2;
				constexpr static bool __has_simple_extra_args	= true;
				template<viewable_range RT, typename QT> requires(__detail::__filter_viewable<RT, QT>)
				[[nodiscard]] constexpr auto operator()(RT&& r, QT&& pred) const { return filter_view(std::forward<RT>(r), std::forward<QT>(pred)); }
				using __adaptor::__range_adaptor<__filter>::operator();
			};
			struct __transform : __adaptor::__range_adaptor<__transform>
			{
				constexpr static int __arity					= 2;
				constexpr static bool __has_simple_extra_args	= true;
				template<viewable_range RT, typename FT> requires(__detail::__transform_viewable<RT, FT>)
				[[nodiscard]] constexpr auto operator()(RT&& r, FT&& f) const { return transform_view(std::forward<RT>(r), std::forward<FT>(f)); }
				using __adaptor::__range_adaptor<__transform>::operator();
			};
			struct __join : __adaptor::__range_adaptor_closure<__join>
			{
				constexpr static bool __has_simple_call_op	= true;
				template<viewable_range RT> requires(__detail::__join_viewable<RT>)
				[[nodiscard]] constexpr auto operator()(RT&& r) const { return join_view<all_t<RT>>(std::forward<RT>(r)); }
			};
			constexpr inline __filter filter{};
			constexpr inline __transform transform{};
			constexpr inline __join join{};
		}
		namespace __detail
		{
			template<typename CT, typename RT, typename ... Args>
			concept __use_iterators_for_to = requires
			{
				requires(common_range<RT>);
				requires(derived_from<__iterator_category_t<iterator_t<RT>>, input_iterator_tag>);
				requires(constructible_from<CT, iterator_t<RT>, sentinel_t<RT>, Args...>);
			};
		}
		template<typename CT, input_range RT, typename ... Args>
		requires(!view<CT>)
		[[nodiscard]] constexpr CT to(RT&& r, Args&& ... args)
		{
			static_assert(!is_const_v<CT> && !is_volatile_v<CT>);
			static_assert(is_class_v<CT> || is_union_v<CT>);
			if constexpr(__detail::__toable<CT, RT>)
			{
				if constexpr(constructible_from<CT, RT, Args...>)
					return CT(std::forward<RT>(r), std::forward<Args>(args)...);
				else if constexpr(constructible_from<CT, from_range_t, RT, Args...>)
					return CT(from_range, std::forward<RT>(r), std::forward<Args>(args)...);
				else if constexpr(__detail::__use_iterators_for_to<CT, RT, Args...>)
					return CT(ranges::begin(r), ranges::end(r), std::forward<Args>(args)...);
				else
				{
					static_assert(constructible_from<CT, Args...>);
					CT c(std::forward<Args>(args)...);
					iterator_t<RT> i	= ranges::begin(r);
					sentinel_t<RT> s	= ranges::end(r);
					while(i != s)
					{
						if constexpr(requires{ c.emplace_back(*i); }) c.emplace_back(*i);
						else if constexpr(requires{ c.push_back(*i); }) c.push_back(*i);
						else if constexpr(requires{ c.emplace(c.end(), *i); }) c.emplace(c.end(), *i);
						else c.insert(c.end(), *i);
						i++;
					}
					return c;
				}
			}
			static_assert(input_range<range_reference_t<RT>>);
			typedef range_value_t<CT> cval;
			return ranges::to<CT>(ref_view(r) | views::transform([]<typename ET>(ET&& e) -> cval { return ranges::to<cval>(std::forward<ET>(e)); }), std::forward<Args>(args)...);
		}
		namespace __detail
		{
			namespace __deduce_to_expr
			{
				template<typename RT>
				struct __fake_input_iterator
				{
					typedef input_iterator_tag iterator_category;
					typedef range_value_t<RT> value_type;
					typedef range_difference_t<RT> difference_type;
					typedef range_reference_t<RT> reference;
					typedef decltype(std::addressof(std::declval<range_reference_t<RT>>())) pointer;
					reference operator*() const;
					pointer operator->() const;
					__fake_input_iterator& operator++();
					__fake_input_iterator operator++(int) const;
					bool operator==(__fake_input_iterator const&) const;
				};
				template<template<typename...> class CT, input_range RT, typename ... Args>
				using __from_range_implicit		= decltype(CT(std::declval<RT>(), std::declval<Args>()...));
				template<template<typename...> class CT, input_range RT, typename ... Args>
				using __from_range_explicit		= decltype(CT(from_range, std::declval<RT>(), std::declval<Args>()...));
				template<template<typename...> class CT, input_range RT, typename ... Args>
				using __from_input_iterators	= decltype(CT(std::declval<__fake_input_iterator<RT>>(), std::declval<__fake_input_iterator<RT>>(), std::declval<Args>()...));
				template<template<typename...> class CT, typename RT, typename ... Args>
				concept __deduce_from_range		= requires { typename __from_range_implicit<CT, RT, Args...>; };
				template<template<typename...> class CT, typename RT, typename ... Args>
				concept __deduce_from_tag		= requires { typename __from_range_explicit<CT, RT, Args...>; };
				template<template<typename...> class CT, typename RT, typename ... Args>
				concept __deduce_from_iterators	= requires { typename __from_input_iterators<CT, RT, Args...>; };
				template<template<typename...> class CT, typename RT, typename ... Args>
				concept __can_deduce = __deduce_from_range<CT, RT, Args...> || __deduce_from_tag<CT, RT, Args...> || __deduce_from_iterators<CT, RT, Args...>;
			}
		}
		template<template<typename...> class CT, input_range RT, typename ... Args>
		[[nodiscard]] constexpr auto to(RT&& r, Args&& ... args)
		{
			using namespace __detail::__deduce_to_expr;
			static_assert(__can_deduce<CT, RT, Args...>, "cannot deduce container specialization");
			if constexpr(__deduce_from_iterators<CT, RT, Args...>)
				return ranges::to<__from_input_iterators<CT, RT, Args...>>(std::forward<RT>(r), std::forward<Args>(args)...);
			else if constexpr(__deduce_from_tag<CT, RT, Args...>)
				return ranges::to<__from_range_explicit<CT, RT, Args...>>(std::forward<RT>(r), std::forward<Args>(args)...);
			else return ranges::to<__from_range_implicit<CT, RT, Args...>>(std::forward<RT>(r), std::forward<Args>(args)...);
		}
		namespace __detail
		{
			template<typename CT, typename RT, typename ... Args>
			concept __to_adaptable = requires { ranges::to<CT>(std::declval<RT>(), std::declval<Args>()...); };
			template<typename CT>
			struct __to {
				template<typename RT, typename ... Args> requires(__to_adaptable<CT, RT, Args...>)
				constexpr auto operator()(RT&& r, Args&&... args) const { return ranges::to<CT>(std::forward<RT>(r), std::forward<Args>(args)...); }
			};
		}
		template<typename CT, typename ... Args>
		requires(!view<CT>)
		[[nodiscard]] constexpr auto to(Args&& ... args)
		{
			static_assert(!is_const_v<CT> && !is_volatile_v<CT>);
			static_assert(is_class_v<CT> || is_union_v<CT>);
			typedef views::__adaptor::__partial_adaptor<__detail::__to<CT>, decay_t<Args>...> result_type;
			return result_type(0, std::forward<Args>(args)...);
		}
	}
}
#endif