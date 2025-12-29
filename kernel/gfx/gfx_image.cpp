#include <gfx_image.hpp>
#include <stb_image_resize2.h>
#include <stdlib.h>
namespace ooos
{
	static inline uint8_t* __load(addr_t img_buffer, size_t buffer_len, int& width, int& height, int& channels) noexcept;
	stb_image_data::stb_image_data() noexcept = default;
	stb_image_data::~stb_image_data() noexcept = default;
	stb_image_data::stb_image_data(stb_image_data&&) noexcept = default;
	stb_image_data::stb_image_data(stb_image_data const& that) : stb_image_data(that.__channels, that.__dimensions, array_dup(that.__data_ptr.get(), that.size_bytes())) {}
	stb_image_data& stb_image_data::operator=(stb_image_data&&) noexcept = default;
	stb_image_data& stb_image_data::operator=(stb_image_data const& that) { return (*this = std::move(stb_image_data(that))); }
	stb_image_data::stb_image_data(const void* image_buffer, size_t buffer_len, n2vec<int> const& dims) : stb_image_data(std::move(stb_image_data(image_buffer, buffer_len).scale(dims))) {}
	stb_image_data::stb_image_data(int ch, n2vec<int> const& dm, uint8_t* dp) noexcept : __channels(ch), __dimensions(dm), __data_ptr(dp) {}
	stb_image_data::stb_image_data(const void* image_buffer, size_t buffer_len) : __channels(), __dimensions(), __data_ptr(__load(image_buffer, buffer_len, __dimensions[0], __dimensions[1], __channels)) {}
	size_t stb_image_data::size_bytes() const noexcept { return static_cast<size_t>(__dimensions.volume() * __channels); }
	size_t stb_image_data::channels() const noexcept { return static_cast<size_t>(__channels); }
	vec2 stb_image_data::dimensions() const noexcept { return vec(static_cast<size_t>(__dimensions[0]), static_cast<size_t>(__dimensions[1])); }
	std::span<uint8_t> stb_image_data::get_view() const noexcept { return std::span<uint8_t>(__data_ptr.get(), size_bytes()); }
	stb_image_data::operator bool() const noexcept { return __data_ptr; }
	static inline std::span<uint8_t> __to_span(uint8_t* image, int width, int height, int channels) { return std::span<uint8_t>(image, static_cast<size_t>((vec(width, height)).volume() * channels)); }
	static inline uint8_t* __load(addr_t img_buffer, size_t buffer_len, int& width, int& height, int& channels) noexcept
	{
		int* x	= std::addressof(width);
		int* y	= std::addressof(height);
		int* c	= std::addressof(channels);
		return stbi_load_from_memory(img_buffer, static_cast<int>(buffer_len), x, y, c, 0);
	}
	static inline std::pair<std::vector<uint32_t>, vec2> __convert_image_data(std::span<uint8_t>&& image, size_t width, size_t channels)
	{
		std::pair<std::vector<uint32_t>, vec2> result{};
		result.second[0]					= width;
		std::vector<uint32_t>& v			= result.first;
		for(std::span<uint8_t>::iterator i	= image.begin(); i < image.end(); i += channels)
			v.push_back(dword(channels > 2UZ ? word(i[2], i[1]) : word(i[0], i[0]), word(i[0], channels == 4UZ ? i[3] : channels == 2UZ ? i[1] : 0xFFUC)));
		result.second[1]					= v.size() / width;
		return result;
	}
	static inline std::pair<std::vector<uint32_t>, vec2> __load_image_data(addr_t img_buffer, size_t buffer_len)
	{
		stb_image_data data(img_buffer, buffer_len);
		size_t width	= data.dimensions()[0];
		return __convert_image_data(data.get_view(), width, data.channels());
	}
	static inline std::pair<std::vector<uint32_t>, vec2> __load_image_data_scaled(addr_t img_buffer, size_t buffer_len, vec2 const& sdims)
	{
		n2vec<int> idims	= xvec<int>(sdims);
		stb_image_data data(img_buffer, buffer_len, idims);
		size_t width	= data.dimensions()[0];
		return __convert_image_data(data.get_view(), width, data.channels());
	}
	gfx_image::gfx_image(std::pair<std::vector<uint32_t>, vec2>&& p) noexcept : __vec(std::move(p.first)), __arr(__vec::data(), p.second), __view() {}
	gfx_image::gfx_image(const void* image_buffer, size_t buffer_len) : gfx_image(__load_image_data(image_buffer, buffer_len)) {}
	gfx_image::gfx_image(const void* image_buffer, size_t buffer_len, vec2 const& dims) : gfx_image(__load_image_data_scaled(image_buffer, buffer_len, dims)) {}
	gfx_image::~gfx_image()									= default;
	gfx_image::gfx_image(gfx_image const&)					= default;
	gfx_image::gfx_image(gfx_image&&) noexcept				= default;
	gfx_image& gfx_image::operator=(gfx_image const&)		= default;
	gfx_image& gfx_image::operator=(gfx_image&&) noexcept	= default;
	gfx_image::iterator gfx_image::begin() noexcept { return __arr::begin(); }
	gfx_image::iterator gfx_image::end() noexcept { return __arr::end(); }
	gfx_image::reference gfx_image::operator[](vec2 const& pos) noexcept { return (*static_cast<__arr*>(this))[pos]; }
	gfx_image::const_reference gfx_image::operator[](vec2 const& pos) const noexcept { return (*static_cast<__arr const*>(this))[pos]; }
	gfx_image::pointer gfx_image::operator[](size_type pos) noexcept { return (*static_cast<__arr*>(this))[pos]; }
	gfx_image::const_pointer gfx_image::operator[](size_type pos) const noexcept { return (*static_cast<__arr const*>(this))[pos]; }
	gfx_image::size_type gfx_image::size_bytes() const noexcept { return __arr::size_bytes(); }
	gfx_image::reference gfx_image::at(vec2 const& pos) { return __arr::at(pos); }
	gfx_image::const_reference gfx_image::at(vec2 const& pos) const { return __arr::at(pos); }
	gfx_image::pointer gfx_image::at(size_type pos) { return __arr::at(pos); }
	gfx_image::const_pointer gfx_image::at(size_type pos) const { return __arr::at(pos); }
	void gfx_image::display(linear_frame_buffer<uint32_t>& fb, vec2 const& pos) { fb.draw(pos, dimensions() + pos, std::bind_front(__idx, this)); }
	stb_image_data& stb_image_data::scale(n2vec<int> const& target_dims)
	{
		if(__data_ptr)
		{
			uint8_t* result	= stbir_resize_uint8_linear(__data_ptr.get(), __dimensions[0], __dimensions[1], 0, nullptr, target_dims[0], target_dims[1], 0, static_cast<stbir_pixel_layout>(__channels));
			if(!result) throw std::bad_alloc();
			__data_ptr.reset(result);
			__dimensions	= target_dims;
		}
		return *this;
	}
}