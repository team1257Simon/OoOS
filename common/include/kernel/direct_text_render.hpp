#ifndef __DIRECT_RENDER
#define __DIRECT_RENDER
#include "kernel/font.hpp"
#include "string"
class direct_text_render
{
    uint32_t* __fb_ptr{};
    font_render __render{};
    uint32_t __fb_wid{};
    uint32_t __fb_ht{};
    point __cursor_pos{ 0, 0 };
    constexpr uint32_t __fb_col_cap() const { return __fb_wid / __render.glyph_width(); }
    constexpr uint32_t __fb_row_cap() const { return __fb_ht / __render.glyph_height(); }
    void __advance() noexcept;
    void __write_one(char c);
public:
    constexpr direct_text_render() noexcept = default;
    constexpr direct_text_render(sysinfo_t* fb_info, psf2_t* font_data, uint32_t fg_color, uint32_t bg_color) noexcept :
        __fb_ptr{ fb_info->fb_ptr },
        __render{ font_data, fb_info->fb_pitch, fg_color, bg_color },
        __fb_wid{ fb_info->fb_width },
        __fb_ht{ fb_info->fb_height }
    {}
    constexpr void set_fg_color(uint32_t color) noexcept { __render.set_fg_color(color); }
    constexpr uint32_t get_fg_color() const noexcept { return __render.fg_color(); }
    constexpr void set_bg_color(uint32_t color) noexcept { __render.set_bg_color(color); }
    constexpr uint32_t get_bg_color() const noexcept { return __render.bg_color(); }
    void cls();
    void endl();
    void cr();
    void print_text(const char* text);
    void print_text(std::string const& text);
    void print_line(const char* text);
    void print_line(std::string const& text);
};
#endif