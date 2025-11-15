#ifndef __DIRECT_RENDER
#define __DIRECT_RENDER
#include <font.hpp>
#include <frame_buffer.hpp>
class direct_text_render
{
	psf2_t const* __font;
	size_t __cols;
	size_t __rows;
	ooos::scaled_frame_buffer<uint32_t> __fb;
	ooos::vec2 __cursor{};
	size_t __ident{};
	void __scur(int i);
	uint32_t __glyph_px(size_t glyph_idx, ooos::vec2 pt);
	size_t __bounds_check_idx(wchar_t c);
public:
	uint32_t foreground;
	uint32_t background;
	constexpr direct_text_render() noexcept = default;
	direct_text_render(sysinfo_t const* si) noexcept;
	void cls();
	void endl();
	void print_text(const char* text);
	void print_line(const char* text);
	void putc(wchar_t ch);
};
#endif