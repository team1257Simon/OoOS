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
		constexpr numeric_vector operator%(numeric_vector<N, S> const& restrict that) const restrict noexcept
		{
			numeric_vector<N, R> result{};
			for(size_type i{}; i < R; i++) result[i] = this->__my_array[i] % that[i];
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
	};
	template<numeric N, std::convertible_to<N> M>
	constexpr numeric_vector<N, 3UZ> cross(numeric_vector<N, 3UZ> const& restrict __this, numeric_vector<M, 3UZ> const& restrict __that) noexcept
	{
		return numeric_vector<N, 3UZ>
		{
			static_cast<N>(__this[1] * __that[2] - __this[2] * __that[1]),
			static_cast<N>(__this[2] * __that[0] - __this[0] * __that[2]),
			static_cast<N>(__this[0] * __that[1] - __this[1] * __that[0])
		};
	}
	template<size_t R> using scale_vector = numeric_vector<size_t, R>;
	template<typename T, size_t R>
	struct multiarray
	{
		typedef T value_type;
		typedef std::add_pointer_t<value_type> pointer;
		typedef std::add_pointer_t<std::add_const_t<std::remove_cv_t<value_type>>> const_pointer;
		typedef decltype(std::declval<pointer>()[std::declval<size_t>()]) reference;
		typedef decltype(std::declval<const_pointer>()[std::declval<size_t>()]) const_reference;
		typedef decltype(sizeof(value_type)) size_type;
	private:
		pointer __my_array;
		scale_vector<R> __dimensions;
		scale_vector<R> __scales;
		size_type __total_size;
		template<typename U, size_t S> friend struct multiarray;
		constexpr static scale_vector<R> __compute_scales(scale_vector<R> const& dimensions) noexcept;
	public:
		constexpr multiarray() noexcept = default;
		constexpr multiarray(pointer a, scale_vector<R> const& d) noexcept : __my_array(a), __dimensions(d), __scales(__compute_scales(d)), __total_size(__dimensions.volume()) {}
		constexpr ~multiarray() noexcept = default;
	private:
		constexpr multiarray(pointer a, scale_vector<R> const& d, scale_vector<R> const& s) noexcept : __my_array(a), __dimensions(d), __scales(s), __total_size(__dimensions.volume()) {}
		template<size_t S> requires(S < R) constexpr scale_vector<R - S> __rem_dims() const noexcept { return this->__dimensions.template back_sub<R - S>(); }
		template<size_t S> requires(S < R) constexpr scale_vector<R - S> __rem_scales() const noexcept { return this->__scales.template back_sub<R - S>(); }
		template<size_t S> requires(S <= R) constexpr size_type __offset(scale_vector<S> const& target_pt) const noexcept { return this->__scales * (target_pt % this->__dimensions); }
		template<size_t S> requires(S < R - 1) constexpr multiarray<T, R - S> __index(scale_vector<S> const& pt) const noexcept { return multiarray<T, R - S>(__my_array + this->template __offset(pt), this->template __rem_dims<S>(), this->template __rem_scales<S>()); }
		template<size_t S> requires(S == R - 1) constexpr pointer __index(scale_vector<S> const& pt) noexcept { return __my_array + this->__offset(pt); }
		template<size_t S> requires(S == R - 1) constexpr const_pointer __index(scale_vector<S> const& pt) const noexcept { return __my_array + this->__offset(pt); }
		template<size_t S> requires(S == R) constexpr reference __index(scale_vector<S> const& pt) noexcept { return __my_array[this->__offset(pt)]; }
		template<size_t S> requires(S == R) constexpr const_reference __index(scale_vector<S> const& pt) const noexcept { return __my_array[this->__offset(pt)]; }
		template<size_t S> using __index_result 		= std::conditional_t<(S < R - 1), multiarray<T, R - S>, std::conditional_t<(S == R - 1), pointer, reference>>;
		template<size_t S> using __const_index_result 	= std::conditional_t<(S < R - 1), multiarray<T, R - S>, std::conditional_t<(S == R - 1), const_pointer, const_reference>>;
	public:
		template<std::convertible_to<size_t> ... Is> requires(sizeof...(Is) <= R && sizeof...(Is) != 0UZ) constexpr __index_result<sizeof...(Is)> at(Is&& ... indices) noexcept;
		template<std::convertible_to<size_t> ... Is> requires(sizeof...(Is) <= R && sizeof...(Is) != 0UZ) constexpr __const_index_result<sizeof...(Is)> at(Is&& ... indices) const noexcept;
		template<size_t S> requires(S < R) constexpr multiarray<T, R - S> operator[](scale_vector<S> const& pt) const noexcept { return this->__index(pt); }
		constexpr reference operator[](scale_vector<R> const& pt) noexcept { return __my_array[pt * this->__dimensions]; }
		constexpr const_reference operator[](scale_vector<R> const& pt) const noexcept { return __my_array[pt * this->__dimensions]; }
		template<std::convertible_to<size_t> ... Is> requires(sizeof...(Is) <= R && sizeof...(Is) != 0UZ) constexpr __index_result<sizeof...(Is)> operator[](Is&& ... indices) noexcept { return at(std::forward<Is>(indices)...); }
		template<std::convertible_to<size_t> ... Is> requires(sizeof...(Is) <= R && sizeof...(Is) != 0UZ) constexpr __const_index_result<sizeof...(Is)> operator[](Is&& ... indices) const noexcept { return at(std::forward<Is>(indices)...); }
		constexpr size_type size() const noexcept { return __total_size; }
		scale_vector<R> const& dimensions() const noexcept { return __dimensions; }
	};
	template<typename T, size_t R>
	constexpr scale_vector<R> multiarray<T, R>::__compute_scales(scale_vector<R> const& dimensions) noexcept
	{
		if constexpr(R == 0) return scale_vector<R>{};
		else if constexpr(R == 1) return scale_vector<R>{ 1UZ };
		else if constexpr(R == 2) return scale_vector<R>{ 1UZ, dimensions[0] };
		else if constexpr(R == 3) return scale_vector<R>{ 1UZ, dimensions[0], dimensions[0] * dimensions[1] };
		scale_vector<R> result{};
		if constexpr(R % 2 != 0) result[R - 1] = dimensions.template sub<R - 1>().volume();
		size_t count = R / 2;
		for(size_t i = 0UZ; i < count; i++)
		{
			result[2 * i] 			= 1UZ;
			result[2 * i + 1] 		= dimensions[2 * i];
			if(i) result[2 * i + 1]	*= dimensions[2 * (i - 1)];
			for(size_t j = 0; j < i; j++)
			{
				result[2 * j] 		*= dimensions[2 * i];
				result[2 * j + 1] 	*= dimensions[2 * i];
				result[2 * j + 1] 	*= dimensions[2 * i + 1];
			}
		}
		return result;
	}
	template<typename T, size_t R>
	template<std::convertible_to<size_t> ... Is> requires(sizeof...(Is) <= R && sizeof...(Is) != 0UZ)
	constexpr typename multiarray<T, R>::template __index_result<sizeof...(Is)> multiarray<T, R>::at(Is&& ... indices) noexcept
	{
		typedef scale_vector<sizeof...(Is)> index_vec;
		index_vec point{ std::forward<Is>(indices)... };
		return this->__index(point);
	}
	template<typename T, size_t R>
	template<std::convertible_to<size_t> ... Is> requires(sizeof...(Is) <= R && sizeof...(Is) != 0UZ)
	constexpr typename multiarray<T, R>::template __const_index_result<sizeof...(Is)> multiarray<T, R>::at(Is&& ... indices) const noexcept
	{
		typedef scale_vector<sizeof...(Is)> index_vec;
		index_vec point{ std::forward<Is>(indices)... };
		return this->__index(point);
	}
}
#endif