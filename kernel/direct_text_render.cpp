#include "direct_text_render.hpp"
#include "kernel/libk_decls.h"
#include "string.h"
const char* hex {"0123456789ABCDEF"};

void direct_text_render::__advance() noexcept 
{
    if(__cursor_pos.x < __fb_col_cap()) __cursor_pos.x++;
    else endl();
}

void direct_text_render::__write_one(char c)
{
    if(c == '\n') endl();
    else if(c)
    {
        __render(c, __fb_ptr, __cursor_pos);
        __advance();
    }
}

void direct_text_render::cls()
{
    for(point p {0, 0}; p.y < __fb_row_cap(); p.y++) 
    {
        for(p.x = 0; p.x < __fb_col_cap(); p.x++)
        {
            __render.fill(__render.bg_color(), __fb_ptr, p);
        }
    }
}

void direct_text_render::endl()
{
    __cursor_pos.x = 0;
    if(__cursor_pos.y < __fb_row_cap()) 
    {
        __cursor_pos.y++;
    }
    else
    {
        __cursor_pos.y = 0;
    }
}

void direct_text_render::print_text(const char *text)
{
    size_t n = strnlen(text, __fb_col_cap() * __fb_row_cap());
    for(size_t i = 0; i < n; i++) 
    {
        __write_one(text[i]);
    }
}

static size_t log16(uint64_t number) 
{
    uint64_t m = 0;
    size_t result = 0;
    while(number & ~m)
    {
        result++;
        m = (m << 4) | 0xF;
    }
    return result;
}

static uint8_t digit16(uint64_t num, uint8_t i)
{
    size_t j = i * 4;
    uint64_t m = 0xF;
    return (num & (m << j)) >> j;
}

void direct_text_render::print_hex(uint64_t number)
{
    if(number == 0)
    {
        print_text("0x0");
    }
    else 
    {
        print_text("0x");
        size_t n = log16(number);
        for(size_t d = 1; d <= n; d++)
        {
            __write_one(hex[digit16(number, n - d)]);
        }  
    }  
}

void direct_text_render::print_addr(void *addr) { print_hex(reinterpret_cast<uint64_t>(addr)); }
