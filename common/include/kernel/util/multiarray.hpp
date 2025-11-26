#ifndef __MULTIARRAY
#define __MULTIARRAY
#include <kernel_api.hpp>
#include <array>
namespace ooos
{
	template<typename T> concept numeric = std::integral<T> || std::floating_point<T>;
	template<numeric N, size_t R>
	struct numeric_vector : std::array<N, R>
	{
		using typename std::array<N, R>::size_type;
		using typename std::array<N, R>::value_type;
		constexpr numeric_vector operator+(numeric_vector const& restrict that) const restrict noexcept
		{
			numeric_vector<N, R> result{};
			for(size_type i{}; i < R; i++) result[i] = this->__my_array[i] + that[i];
			return result;
		}
		constexpr numeric_vector operator-(numeric_vector const& restrict that) const restrict noexcept
		{
			numeric_vector<N, R> result{};
			for(size_type i{}; i < R; i++) result[i] = this->__my_array[i] - that[i];
			return result;
		}
		constexpr numeric_vector operator*(value_type scale) const noexcept
		{
			numeric_vector<N, R> result{};
			for(size_type i{}; i < R; i++) result[i] = this->__my_array[i] * scale;
			return result;
		}
		constexpr numeric_vector operator/(value_type divisor) const noexcept
		{
			numeric_vector<N, R> result{};
			for(size_type i{}; i < R; i++) result[i] = this->__my_array[i] / divisor;
			return result;
		}
		template<size_t S>
		constexpr value_type operator*(numeric_vector<N, S> const& restrict that) const restrict noexcept
		{
			value_type result{};
			size_type total = S < R ? S : R;
			for(size_type i{}; i < total; i++) result += this->__my_array[i] * that[i];
			return result;
		}
		template<size_t S> requires(S >= R)
		constexpr numeric_vector<N, R> operator%(numeric_vector<N, S> const& restrict that) const restrict noexcept
		{
			numeric_vector<N, R> result{};
			for(size_type i{}; i < R; i++) result[i] = this->__my_array[i] % that[i];
			return result;
		}
		template<size_t S> requires(S >= R)
		constexpr numeric_vector<N, R> operator/(numeric_vector<N, R> const& restrict that) const restrict noexcept
		{
			numeric_vector<N, R> result{};
			for(size_type i{}; i < R; i++) result[i] = this->__my_array[i] / that[i];
			return result;
		}
		template<size_t S> requires(S <= R)
		constexpr numeric_vector<N, S> sub() const noexcept
		{
			typedef numeric_vector<N, S> result_type;
			if constexpr(S == R) return result_type(*this);
			result_type result{};
			for(size_type i{}; i < S; i++) result[i] = this->__my_array[i];
			return result;
		}
		template<size_t S> requires(S <= R)
		constexpr numeric_vector<N, S> back_sub() const noexcept
		{
			typedef numeric_vector<N, S> result_type;
			if constexpr(S == R) return result_type(*this);
			size_type start_pos = static_cast<size_t>(R - S);
			result_type result{};
			for(size_type i = start_pos; i < R; i++) result[i - start_pos] = this->__my_array[i];
			return result;
		}
		constexpr value_type volume() const noexcept
		{
			value_type result = this->__my_array[0];
			for(size_type i = static_cast<size_type>(1UZ); i < R; i++) result *= this->__my_array[i];
			return result;
		}
		constexpr value_type sub_volume(size_type n) const noexcept
		{
			if(__unlikely(!n)) return value_type();
			if(__unlikely(n >= R)) return volume();
			value_type result = this->__my_array[0];
			for(size_type i = static_cast<size_type>(1UZ); i < n; i++) result *= this->__my_array[i];
			return result;
		}
		template<size_t S>
		constexpr numeric_vector<N, R + S> splice(numeric_vector<N, S> const& restrict that) const restrict noexcept
		{
			numeric_vector<N, R + S> result{};
			for(size_type i{}; i < R; i++) result[i] 		= this->__my_array[i];
			for(size_type j{}; j < S; j++) result[R + j] 	= that[j];
			return result;
		}
		template<size_t S> requires(S != 0UZ && R % S == 0)
		constexpr numeric_vector<N, R> distribute(numeric_vector<N, S> const& restrict that) const restrict noexcept
		{
			size_t count = R / S;
			numeric_vector<N, R> result{};
			for(size_type i{}; i < count; i++)
				for(size_type j{}; j < S; j++)
					result[i * S + j] = this->__my_array[i * S + j] * that[j];
			return result;
		}
		constexpr numeric_vector<N, R> divide(numeric_vector<N, R> const& restrict that) const restrict noexcept
		{
			numeric_vector<N, R> result{};
			for(size_type i{}; i < R; i++) result[i] = this->__my_array[i] / that[i];
			return result;
		}
	};
	template<size_t R>	using scale_vector	= numeric_vector<size_t, R>;
	template<size_t S>	using size_constant	= std::integral_constant<size_t, S>;
	template<numeric N> using n2vec			= numeric_vector<N, 2UZ>;
	template<numeric N> using n3vec			= numeric_vector<N, 3UZ>;
	template<numeric N> using n4vec			= numeric_vector<N, 4UZ>;
	typedef scale_vector<2UZ> vec2;
	typedef scale_vector<3UZ> vec3;
	typedef scale_vector<4UZ> vec4;
	template<numeric N, std::convertible_to<N> M>
	constexpr n3vec<N> cross(n3vec<N> const& restrict __this, n3vec<M> const& restrict __that) noexcept
	{
		return n3vec<N>
		{
			static_cast<N>(__this[1] * __that[2] - __this[2] * __that[1]),
			static_cast<N>(__this[2] * __that[0] - __this[0] * __that[2]),
			static_cast<N>(__this[0] * __that[1] - __this[1] * __that[0])
		};
	}
	template<std::convertible_to<size_t> ... Ns>
	constexpr scale_vector<sizeof...(Ns)> vec(Ns&& ... ns)
	{
		typedef scale_vector<sizeof...(Ns)> result_type;
		return result_type
		{
			// Calling the conversion operator on any inputs that are structs with an operator unsigned long or similar prevents creating copies unnecessarily.
			// We can't use std::forward here because the inputs might be const-qualified.
			static_cast<size_t>(ns)...
		};
	}
	template<typename T, size_t R>
	struct multiarray
	{
		typedef T value_type;
		constexpr static size_t rank		= R;
		typedef std::add_pointer_t<value_type> pointer;
		typedef std::add_pointer_t<std::add_const_t<std::remove_cv_t<value_type>>> const_pointer;
		typedef decltype(std::declval<pointer>()[std::declval<size_t>()]) reference;
		typedef decltype(std::declval<const_pointer>()[std::declval<size_t>()]) const_reference;
		typedef decltype(sizeof(value_type)) size_type;
		typedef scale_vector<rank> scale_type;
		constexpr multiarray() noexcept		= default;
		constexpr ~multiarray() noexcept	= default;
		constexpr multiarray(pointer a, scale_type const& d, scale_type const& s) noexcept : __my_array(a), __dimensions(d), __scales(s), __total_size(__dimensions.volume()) {}
		constexpr multiarray(pointer a, scale_type const& d) noexcept : __my_array(a), __dimensions(d), __scales(__compute_scales(d)), __total_size(__dimensions.volume()) {}
		template<size_t S> using sub_array	= multiarray<value_type, __sub_rank(size_constant<S>())>;
		template<size_t S> using sub_scale	= scale_vector<__sub_rank(size_constant<S>())>;
	private:
		pointer __my_array;
		scale_type __dimensions;
		scale_type __scales;
		size_type __total_size;
		template<typename U, size_t S> friend struct multiarray;
		constexpr static scale_type __compute_scales(scale_type const& dimensions) noexcept;
		template<size_t S> requires(S <= rank) constexpr static size_t __sub_rank(size_constant<S>) noexcept { return static_cast<size_t>(rank - S); }
		template<size_t S> requires(S < rank) constexpr sub_scale<S> __rem_dims() const noexcept { return __dimensions.template back_sub<__sub_rank(size_constant<S>())>(); }
		template<size_t S> requires(S < rank) constexpr sub_scale<S> __rem_scales() const noexcept { return __scales.template back_sub<__sub_rank(size_constant<S>())>(); }
		template<size_t S> requires(S <= rank) constexpr size_type __offset(scale_vector<S> const& target_pt) const noexcept { return __scales * (target_pt % __dimensions); }
		template<size_t S> requires(S + 1UZ < rank) constexpr sub_array<S> __index(scale_vector<S> const& pt) const noexcept { return sub_array<S>(__my_array + __offset(pt), __rem_dims<S>(), __rem_scales<S>()); }
		template<size_t S> requires(S + 1UZ == rank) constexpr pointer __index(scale_vector<S> const& pt) noexcept { return __my_array + __offset(pt); }
		template<size_t S> requires(S + 1UZ == rank) constexpr const_pointer __index(scale_vector<S> const& pt) const noexcept { return __my_array + __offset(pt); }
		template<size_t S> requires(S == rank) constexpr reference __index(scale_vector<S> const& pt) noexcept { return __my_array[__offset(pt)]; }
		template<size_t S> requires(S == rank) constexpr const_reference __index(scale_vector<S> const& pt) const noexcept { return __my_array[__offset(pt)]; }
	public:
		template<size_t S> requires(S <= rank) using index_result 		= decltype(std::declval<multiarray>().__index(std::declval<scale_vector<S> const>()));
		template<size_t S> requires(S <= rank) using const_index_result = decltype(std::declval<multiarray const>().__index(std::declval<scale_vector<S> const>()));
		template<size_t S> requires(S < rank) constexpr sub_array<S> operator[](scale_vector<S> const& pt) const noexcept { return __index(pt); }
		constexpr reference operator[](scale_type const& pt) noexcept { return __index(pt); }
		constexpr const_reference operator[](scale_type const& pt) const noexcept { return __index(pt); }
		template<std::convertible_to<size_t> ... Is> requires(sizeof...(Is) <= rank && sizeof...(Is) != 0UZ) constexpr index_result<sizeof...(Is)> operator[](Is&& ... indices) noexcept { return __index(vec(static_cast<size_t>(indices)...)); }
		template<std::convertible_to<size_t> ... Is> requires(sizeof...(Is) <= rank && sizeof...(Is) != 0UZ) constexpr const_index_result<sizeof...(Is)> operator[](Is&& ... indices) const noexcept { return __index(vec(static_cast<size_t>(indices)...)); }
		constexpr size_type size() const noexcept { return __total_size; }
		constexpr scale_type const& dimensions() const noexcept { return __dimensions; }
		constexpr scale_type const& scales() const noexcept { return __scales; }
		constexpr pointer data() const noexcept { return __my_array; }
	};
	template<typename T, size_t R>
	constexpr typename multiarray<T, R>::scale_type multiarray<T, R>::__compute_scales(scale_type const& dimensions) noexcept
	{
		if constexpr(R == 0) return scale_type{};
		else if constexpr(R == 1) return vec(1UZ);
		else if constexpr(R == 2) return vec(1UZ, dimensions[0]);
		else if constexpr(R == 3) return vec(1UZ, dimensions[0]);
		scale_type result{};
		if constexpr(rank % 2	!= 0)
			result[rank - 1]	= dimensions.sub_volume(static_cast<size_t>(rank - 1UZ));
		size_t count			= rank / 2;
		for(size_t i			= 0UZ; i < count; i++)
		{
			size_t xpos 			= i * 2UZ;
			size_t ypos				= xpos + 1UZ;
			result[xpos] 			= 1UZ;
			result[ypos] 			= dimensions[xpos];
			if(i) result[ypos]		*= dimensions[xpos - 2];
			for(size_t j = 0; j < i; j++)
			{
				size_t jpos			= j * 2;
				result[jpos++]		*= dimensions[xpos];
				result[jpos] 		*= dimensions[xpos];
				result[jpos] 		*= dimensions[ypos];
			}
		}
		return result;
	}
}
#endif