#ifndef __SIMPLEX_STREAM
#define __SIMPLEX_STREAM
#include <tuple>
#include <bits/stl_algobase.hpp>
#include <bits/range_access.hpp>
namespace std
{
	namespace ext
	{
		namespace __detail
		{
			template<typename CT, typename T> concept __can_push			= requires(CT& c) { c.push(std::declval<T>()); };
			template<typename CT, typename ... Args> concept __can_emplace	= requires(CT& c) { c.emplace(std::declval<Args&&>()...); };
			template<typename ORT, typename IRT>
			concept __gettable = std::ranges::input_range<IRT>
				&& std::ranges::sized_range<IRT>
				&& std::ranges::sized_range<ORT>
				&& std::ranges::output_range<ORT, typename std::ranges::range_value_t<IRT>>
				&& requires(IRT& irt, ORT& ort) { ort.size(std::ranges::size(irt)); irt.get(std::ranges::begin(ort), std::ranges::end(ort)); };
			template<typename RT>
			concept __shiftable = std::ranges::range<RT>
				&& std::is_move_assignable_v<RT>
				&& std::constructible_from<RT, std::ranges::iterator_t<RT>, std::ranges::sentinel_t<RT>>;
		}
		/**
		 * Stream backbone that ties together an input region that is vaguely queue-like to an output region that is vaguely vector-like.
		 * Like the other stream-esque structures in std::ext, it is intended to be a base for more concrete implementations.
		 * Unlike those structures, it has no virtual methods of its own.
		 * Instead, instantiating the template with the right choice of parameters will mean easy implementations of stream-type interfaces in terms of its methods.
		 */
		template<std::ranges::input_range IRT, __detail::__gettable<IRT> ORT>
		requires(__detail::__shiftable<ORT>)
		struct simplex_stream
		{
			typedef std::ranges::range_reference_t<IRT> reference;
			typedef decltype(std::addressof(std::declval<reference>())) pointer;
			typedef std::ranges::iterator_t<ORT> out_range_iterator;
			typedef std::ranges::range_size_t<ORT> size_type;
			typedef std::iter_difference_t<out_range_iterator> difference_type;
			typedef typename std::ranges::range_value_t<ORT> value_type;
			typedef std::add_pointer_t<std::add_const_t<value_type>> const_pointer;
		protected:
			std::pair<IRT, ORT> regions;
			size_type out_pos;
		public:
			constexpr simplex_stream() noexcept(std::is_nothrow_default_constructible_v<std::pair<IRT, ORT>>) requires(std::is_default_constructible_v<std::pair<IRT, ORT>>) : regions(), out_pos() {}
			constexpr ~simplex_stream() noexcept(std::is_nothrow_destructible_v<std::pair<IRT, ORT>>) {}
			template<typename ... Args> requires(std::constructible_from<std::pair<IRT, ORT>, Args...>)
			constexpr simplex_stream(Args&& ... args) noexcept(std::is_nothrow_constructible_v<std::pair<IRT, ORT>, Args...>) :
				regions(std::forward<Args>(args)...), 
				out_pos()
			{}
			constexpr size_type in_avail() const noexcept { return std::ranges::size(regions.first); }
			constexpr size_type out_rem() const noexcept { return static_cast<size_type>(std::ranges::size(regions.second) - out_pos); }
			constexpr size_type seek(int whence, difference_type diff) { return this->seek(static_cast<size_type>(std::max(difference_type(0), diff + this->__start_for(whence)))); }
			template<typename T> requires(__detail::__can_push<IRT, T>) constexpr void push(T t) { regions.first.push(std::forward<T>(t)); }
			template<typename ... Args> requires(__detail::__can_emplace<IRT, Args...>) constexpr void emplace(Args&& ... args) { regions.first.emplace(std::forward<Args>(args)...); }
			constexpr void flush()
			{
				if(out_pos) {
					this->__shift(out_pos);
					out_pos	= size_type(0);
				}
				this->input_flush();
			}
			constexpr size_type seek(size_type pos)
			{
				size_type avail_size	= std::ranges::size(regions.second);
				if(pos > avail_size) this->input_flush();
				size_type actual		= std::min(pos, std::ranges::size(regions.second));
				return (out_pos	= actual);
			}
			constexpr void input_flush()
			{
				size_type avail_size		= in_avail();
				if(avail_size)
				{
					size_type current_size	= std::ranges::size(regions.second);
					size_type target_size	= current_size + avail_size;
					regions.second.size(target_size);
					out_range_iterator i	= this->__get_pos(current_size);
					regions.first.get(i, std::ranges::end(regions.second));
				}
			}
			constexpr size_type read(pointer dest, size_type n)
			{
				if(out_pos) {
					this->__shift(out_pos);
					out_pos		= size_type(0);
				}
				size_type available		= out_rem();
				if(available < n) input_flush();
				size_type actual		= std::min(n, out_rem());
				out_range_iterator i	= __get_pos(out_pos);
				if constexpr(std::contiguous_iterator<out_range_iterator>) copy_or_move(dest, std::to_address(i), actual);
				else for(size_type j	= 0UZ; j < actual; j++, i = std::ranges::next(i))
					dest[j]				= std::move(*i);
				out_pos			+= actual;
				return actual;
			}
			template<std::input_iterator IT, std::sentinel_for<IT> ST> requires(__detail::__can_push<IRT, std::iter_value_t<IT>>)
			constexpr size_type write(IT start, ST finish)
			{
				size_type result{};
				for(IT i = start; i != finish; i = std::ranges::next(i), result++)
					push(*i);
				input_flush();
				return result;
			}
		private:
			constexpr void __shift(size_type where) { regions.second = std::move(ORT(this->__get_pos(where), std::ranges::end(regions.second))); }
			constexpr difference_type __start_for(int whence) const noexcept { return static_cast<difference_type>(whence > 0 ? std::ranges::size(regions.second) : whence == 0 ? out_pos : size_type(0)); }
			constexpr out_range_iterator __get_pos(size_type where) noexcept
			{
				out_range_iterator it = std::ranges::begin(regions.second);
				std::ranges::advance(it, static_cast<difference_type>(where));
				return it;
			}
		};
	}
}
#endif