#ifndef __FONT_H
#define __FONT_H
#include "kernel/kernel_defs.h"
#ifdef __cplusplus
extern "C" 
{
#endif
typedef struct __point
{
	uint32_t x;
	uint32_t y;
} point;
typedef struct __psf2_t
{
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bpg;
    uint32_t height;
    uint32_t width;
    uint8_t glyph_data[];
} __pack psf2_t;
#ifdef __cplusplus
}
class font_render 
{
	psf2_t* __my_font{};
	uint32_t __foreground{};
	uint32_t __background{};
	uint32_t __pitch{};
	constexpr size_t __buffer_start_offset(point const& start) const { return (start.x * __my_font->width) * 4 + (start.y * __my_font->height) * __pitch; }
	constexpr size_t __buffer_offset(point const& start, point const& sub) const { return ((__buffer_start_offset(start) + sub.y * __pitch) / 4) + sub.x; }
	constexpr char __ensure_bounds(char c) const { return (static_cast<unsigned char>(c) > __my_font->numglyph || c < 1) ? ' ' : c; }
	constexpr size_t __glyph_index(char c) const { return __my_font->bpg * __ensure_bounds(c); }
	constexpr uint8_t __glyph_byte(char c, point const& offs) const { return __my_font->glyph_data[__glyph_index(c) + (offs.x / 8) + offs.y]; }
	constexpr bool __glyph_pixel(char c, point const& where) const { return (__glyph_byte(c, where) & (0b10000000 >> (where.x % 8))) != 0; }
public:
	constexpr font_render(psf2_t* font_data, uint32_t pitch, uint32_t fg_color, uint32_t bg_color) noexcept : 
		__my_font		{ font_data }, 
		__foreground	{ fg_color }, 
		__background	{ bg_color }, 
		__pitch			{ pitch } 
						{}
	constexpr font_render() noexcept = default;
	constexpr uint32_t glyph_width() const { return __my_font->width; }
	constexpr uint32_t glyph_height() const { return __my_font->height; }
	constexpr void set_fg_color(uint32_t value) noexcept { __foreground = value; }
	constexpr void set_bg_color(uint32_t value) noexcept { __background = value; }
	constexpr void set_fb_pitch(uint32_t value) noexcept { __pitch = value; }
	constexpr uint32_t fg_color() const noexcept { return __foreground; }
	constexpr uint32_t bg_color() const noexcept { return __background; }
	constexpr uint32_t fb_pitch() const noexcept { return __pitch; }
	__isrcall void operator()(char c, uint32_t* fb, point const& pos) const;
	__isrcall void fill(uint32_t color, uint32_t* fb, point const& pos) const;
};
#endif
#endif