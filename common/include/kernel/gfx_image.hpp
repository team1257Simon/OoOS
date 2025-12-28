#ifndef __GFX_IMG
#define __GFX_IMG
#include <stb_image.h>
#include <util/multiarray.hpp>
#include <bits/ranges_util.hpp>
namespace ooos
{
	class gfx_image : std::vector<uint32_t>, multiarray<uint32_t, 2UZ>, public std::ranges::view_interface<gfx_image>
	{
		typedef std::vector<uint32_t> __vec;
		typedef multiarray<uint32_t, 2UZ> __arr;
		typedef std::ranges::view_interface<gfx_image> __view;
		gfx_image(std::pair<std::vector<uint32_t>, vec2>&& p) noexcept;
	public:
		gfx_image(const void* image_buffer, size_t buffer_len);
		~gfx_image();
		gfx_image(gfx_image const&);
		gfx_image(gfx_image&&) noexcept;
		gfx_image& operator=(gfx_image const&);
		gfx_image& operator=(gfx_image&&) noexcept;
		using typename __arr::value_type;
		using typename __arr::iterator;
		using typename __arr::reverse_iterator;
		using typename __arr::pointer;
		using typename __arr::const_pointer;
		using typename __arr::reference;
		using typename __arr::const_reference;
		using typename __arr::size_type;
		using typename __arr::difference_type;
		using __arr::dimensions;
		using __arr::scales;
		iterator begin() noexcept;
		iterator end() noexcept;
		reference operator[](vec2 const& pos) noexcept;
		const_reference operator[](vec2 const& pos) const noexcept;
		pointer operator[](size_type pos) noexcept;
		const_pointer operator[](size_type pos) const noexcept;
		size_type size_bytes() const noexcept;
		reference at(vec2 const& pos);
		const_reference at(vec2 const& pos) const;
		pointer at(size_type pos);
		const_pointer at(size_type pos) const;
	};
}
#endif