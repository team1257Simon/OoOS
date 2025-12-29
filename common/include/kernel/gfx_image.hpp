#ifndef __GFX_IMG
#define __GFX_IMG
#include <stb_image.h>
#include <frame_buffer.hpp>
#include <bits/ranges_util.hpp>
#include <memory>
namespace ooos
{
	class stb_image_data
	{
		int __channels;
		n2vec<int> __dimensions;
		std::unique_ptr<uint8_t[]> __data_ptr;
		stb_image_data(int ch, n2vec<int> const& dm, uint8_t* dp) noexcept;
	public:
		stb_image_data() noexcept;
		~stb_image_data() noexcept;
		stb_image_data(const void* image_buffer, size_t buffer_len);
		stb_image_data(const void* image_buffer, size_t buffer_len, n2vec<int> const& dims);
		stb_image_data(stb_image_data&&) noexcept;
		stb_image_data(stb_image_data const& that);
		stb_image_data& operator=(stb_image_data&&) noexcept;
		stb_image_data& operator=(stb_image_data const& that);
		stb_image_data& scale(n2vec<int> const& target_dims);
		size_t size_bytes() const noexcept;
		size_t channels() const noexcept;
		vec2 dimensions() const noexcept;
		std::span<uint8_t> get_view() const noexcept;
		operator bool() const noexcept;
	};
	class gfx_image : std::vector<uint32_t>, multiarray<uint32_t, 2UZ>, public std::ranges::view_interface<gfx_image>
	{
		typedef std::vector<uint32_t> __vec;
		typedef multiarray<uint32_t, 2UZ> __arr;
		typedef std::ranges::view_interface<gfx_image> __view;
		gfx_image(std::pair<std::vector<uint32_t>, vec2>&& p) noexcept;
		typedef __arr::reference& (gfx_image::*__idx_ftor)(vec2 const&);
	public:
		gfx_image(const void* image_buffer, size_t buffer_len);
		gfx_image(const void* image_buffer, size_t buffer_len, vec2 const& target_dims);
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
		void display(linear_frame_buffer<uint32_t>& fb, vec2 const& pos = vec(0UZ, 0UZ));
	private:
		constexpr static reference(gfx_image::*__idx)(vec2 const&) = &gfx_image::operator[];
	};
}
#endif