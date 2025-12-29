#include <gfx_image.hpp>
#include <stdlib.h>
namespace ooos
{
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
		int width{}, height{}, channels{};
		uint8_t* image	= __load(img_buffer, buffer_len, width, height, channels);
		if(!image) throw std::runtime_error(stbi_failure_reason());
		struct __simple_guard {
			void* ptr;
			inline ~__simple_guard() noexcept { free(ptr); }
		} g(image);
		return __convert_image_data(__to_span(image, width, height, channels), static_cast<size_t>(width), static_cast<size_t>(channels));
	}
	gfx_image::gfx_image(std::pair<std::vector<uint32_t>, vec2>&& p) noexcept : __vec(std::move(p.first)), __arr(__vec::data(), p.second), __view() {}
	gfx_image::gfx_image(const void* image_buffer, size_t buffer_len) : gfx_image(__load_image_data(image_buffer, buffer_len)) {}
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
}