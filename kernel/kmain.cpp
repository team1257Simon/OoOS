#include "libk_decls.h"
#include "direct_text_render.hpp"
#include "arch/idt_amd64.h"
#include "heap_allocator.hpp"
#include "string"
extern psf2_t* __startup_font;
direct_text_render tty;
extern "C"
{
    extern void gdt_setup();
    void kmain(framebuf_t* fb, mmap_t* mmap, pagefile* pg)
    {
        gdt_setup();
        idt_init();
        heap_allocator::init_instance(pg, mmap);
        new (&tty) direct_text_render{ fb, __startup_font, 0x00FFFFFF, 0 };
        tty.cls();
        tty.print_line("Hello world!");
        tty.print_line("FB: " + std::to_string(fb));
        while(1);
    }
}