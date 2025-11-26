#include <direct_text_render.hpp>
#include <string.h>
#include <functional>
size_t direct_text_render::__bounds_check_idx(wchar_t c) {
	if(__unlikely(static_cast<uint32_t>(c) > __font->numglyph || c < 1)) c = ' ';
	return static_cast<size_t>(c * __font->bpg);
}
uint32_t direct_text_render::__glyph_px(size_t glyph_idx, ooos::vec2 pt)
{
	if(__unlikely(pt[0]	>= __font->width || pt[1] >= __font->height || glyph_idx >= __font->numglyph * __font->bpg)) return background;
	uint8_t glyph_byte	= __font->glyph_data[glyph_idx + (pt[0] / 8) + pt[1]];
	return (glyph_byte & (0x80UC >> (pt[0] % 8))) ? foreground : background;
}
void direct_text_render::__scur(int i)
{
	size_t target	= (__cursor[0] + i + __cols) % __cols;
	if(target < __cursor[0]) __cursor[1] = (__cursor[1] + 1) % __rows;
	__cursor[0]		= target;
}
direct_text_render::direct_text_render(sysinfo_t const* si) noexcept :
	__font		{ std::addressof(__startup_font_data) },
	__cols		{ si->fb_width / __font->width },
	__rows		{ si->fb_height / __font->height },
	__fb		{ si->fb_ptr, si->fb_width, si->fb_height, __font->width, __font->height },
	foreground	{ 0x00FFFFFFU },
	background	{ 0x00000000U }
				{}
void direct_text_render::cls()
{
	for(ooos::vec2 pos	= ooos::vec(0UZ, 0UZ); pos[1] < __rows; ++pos[1])
		for(pos[0]		= 0UZ; pos[0] < __cols; ++pos[0])
			__fb.fill(pos, background);
}
void direct_text_render::endl()
{
	__cursor[1]++;
	__cursor[1]	%= __rows;
	__cursor[0]	= 0UZ;
}
void direct_text_render::print_text(const char* str)
{
	for(size_t i = 0; str[i]; i++)
		putc(str[i]);
}
void direct_text_render::print_line(const char* str)
{
	for(size_t i = 0; str[i]; i++)
		putc(str[i]);
	endl();
}
void direct_text_render::putc(wchar_t c)
{
	switch(c)
	{
	case L'\n':
		endl();
		return;
	case L'\r':
		__ident		= 0UZ;
		__cursor[0]	= 0UZ;
		return;
	case L'\t':
		if(!__cursor[0] && __ident < __cols)
			__ident	+= 4UZ;
		__scur(4);
		return;
	case L'\b':
		__scur(-1);
		__fb.fill(__cursor, background);
		break;
	// more as needed
	default:
		__fb.draw(__cursor, std::bind_front(&direct_text_render::__glyph_px, this, __bounds_check_idx(c)));
		break;
	}
	__scur(1);
}