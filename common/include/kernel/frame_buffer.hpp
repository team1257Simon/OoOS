#ifndef __FRAMEBUFFER
#define __FRAMEBUFFER
#include <util/multiarray.hpp>
namespace ooos
{
	template<typename FT, typename T> concept vec_draw_fn = std::assignable_from<typename multiarray<T, 2UZ>::reference, decltype(std::declval<FT>()(std::declval<vec2>()))>;
	template<std::integral T> struct fill_t {
		T value;
		constexpr T operator()(vec2 const&) const noexcept { return value; }
	};
	template<std::integral T> struct scaled_frame_buffer;
	template<std::integral T>
	struct linear_frame_buffer
	{
		typedef T pixel;
		typedef multiarray<pixel, 2UZ> array_type;
		typedef typename array_type::reference reference;
		typedef typename array_type::pointer pointer;
	private:
		array_type __fb;
		friend struct scaled_frame_buffer<T>;
		constexpr vec4 __scale_dims(vec2 const& sc) const noexcept { return __fb.dimensions().divide(sc).splice(sc); }
	public:
		constexpr linear_frame_buffer() noexcept = default;
		constexpr linear_frame_buffer(pointer fb, size_t width, size_t height) noexcept : __fb(fb, vec(width, height)) {}
		template<vec_draw_fn<T> DFT> constexpr void draw(vec4 const& area, DFT&& fn) noexcept;
		template<vec_draw_fn<T> DFT> constexpr void draw(vec2 const& start, vec2 const& end, DFT&& fn) noexcept { draw(start.splice(end), std::forward<DFT>(fn)); }
		template<vec_draw_fn<T> DFT> constexpr void draw(vec2 const& start, DFT&& fn) noexcept { draw(start, __fb.dimensions(), std::forward<DFT>(fn)); }
		template<vec_draw_fn<T> DFT> constexpr void draw(DFT&& fn) noexcept { draw(vec(0UZ, 0UZ), std::forward<DFT>(fn)); }
		constexpr void fill(vec4 const& area, pixel const& value) noexcept(std::is_nothrow_copy_constructible_v<pixel>) requires(std::copy_constructible<pixel>) { this->template draw<fill_t<pixel>>(area, std::move(fill_t(value))); }
		constexpr void fill(vec2 const& start, vec2 const& end, pixel const& value) noexcept(std::is_nothrow_copy_constructible_v<pixel>) requires(std::copy_constructible<pixel>) { fill(start.splice(end), value); }
		constexpr void fill(vec2 const& start, pixel const& value) noexcept(std::is_nothrow_copy_constructible_v<pixel>) requires(std::copy_constructible<pixel>) { fill(start, __fb.dimensions(), value); }
		constexpr void fill(pixel const& value) noexcept(std::is_nothrow_copy_constructible_v<pixel>) requires(std::copy_constructible<pixel>) { fill(vec(0UZ, 0UZ), value); }
	};
	template<std::integral T>
	struct scaled_frame_buffer
	{
		typedef T pixel;
		typedef multiarray<pixel, 4UZ> array_type;
		typedef typename array_type::reference reference;
		typedef typename array_type::pointer pointer;
	private:
		vec2 __slot_scale;
		array_type __fb;
		reference __sub(vec2 const& pos, vec2 const& px) noexcept { return __fb[pos[0], pos[1], px[0], px[1]]; }
	public:
		constexpr scaled_frame_buffer() noexcept = default;
		constexpr scaled_frame_buffer(linear_frame_buffer<T> const& fb, size_t s_wid, size_t s_ht) noexcept : __slot_scale(vec(s_wid, s_ht)), __fb(fb.__fb.data(), fb.__scale_dims(__slot_scale)) {}
		constexpr scaled_frame_buffer(pointer fb, size_t b_wid, size_t b_ht, size_t s_wid, size_t s_ht) noexcept : __slot_scale(vec(s_wid, s_ht)), __fb(fb, vec(b_wid / s_wid, b_ht / s_ht, s_wid, s_ht)) {}
		template<vec_draw_fn<T> DFT> constexpr void draw(vec2 const& pos, DFT&& fn) noexcept;
		constexpr void fill(vec2 const& pos, pixel const& value) noexcept { this->template draw<fill_t<pixel>>(pos, std::move(fill_t<pixel>(value))); }
	};
	template<std::integral T>
	template<vec_draw_fn<T> DFT>
	constexpr void linear_frame_buffer<T>::draw(vec4 const& area, DFT&& fn) noexcept
	{
		vec2 base 			= area.template sub<2UZ>();
		for(vec2 pos 		= base; pos[1] < area[3]; ++pos[1])
			for(pos[0]		= area[0]; pos[0] < area[2]; ++pos[0])
				__fb[pos]	= fn(pos - base);
	}
	template<std::integral T>
	template<vec_draw_fn<T> DFT>
	constexpr void scaled_frame_buffer<T>::draw(vec2 const& pos, DFT&& fn) noexcept
	{
		for(vec2 px				= vec(0UZ, 0UZ); px[1] < __slot_scale[1]; ++px[1])
			for(px[0]			= 0UZ; px[0] < __slot_scale[0]; ++px[0])
				__sub(pos, px)	= fn(px);
	}
}
#endif