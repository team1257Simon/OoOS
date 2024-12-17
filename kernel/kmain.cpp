#include "kernel/libk_decls.h"
#include "kernel/direct_text_render.hpp"
extern psf2_t* __startup_font;
uint8_t tty_space[sizeof(direct_text_render)];
direct_text_render* tty = reinterpret_cast<direct_text_render*>(tty_space);

extern "C"
{
    void kmain(framebuf_t* fb, mmap_t* mmap, pagefile* pg)
    {
        new (tty) direct_text_render { fb, __startup_font, 0x00FFFFFF, 0 };
        tty->cls();
        tty->print_text("Hi there!");
        while(1);
    }
}