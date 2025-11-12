#include "font.hpp"
extern "C" const uint8_t __startup_font_data[]
{
	#embed "startup_font.psf"
};
psf2_t const* __startup_font = reinterpret_cast<psf2_t const*>(__startup_font_data);
void font_render::operator()(char c, uint32_t* fb, point const& pos) const { for(point glyph_px(0U, 0U); glyph_px.y < __my_font->height; glyph_px.y++) { for(glyph_px.x = 0; glyph_px.x < __my_font->width; glyph_px.x++) fb[__buffer_offset(pos, glyph_px)] = __glyph_pixel(c, glyph_px) ? __foreground : __background; fb[__buffer_offset(pos, point(__my_font->width, glyph_px.y))] = __background; } }
void font_render::fill(uint32_t color, uint32_t* fb, point const& pos) const { for(point px(0U, 0U); px.y < __my_font->height; px.y++) { for(px.x = 0; px.x < __my_font->width; px.x++) { fb[__buffer_offset(pos, px)] = color; } fb[__buffer_offset(pos, point(__my_font->width, px.y))] = color; } }