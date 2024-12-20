#include "kernel/libk_decls.h"
#include "kernel/direct_text_render.hpp"
#include "kernel/idt_amd64.h"
#include "kernel/heap_allocator.hpp"
extern psf2_t* __startup_font;
direct_text_render* tty;
extern "C"
{
    extern void gdt_setup();
    void kmain(framebuf_t* fb, mmap_t* mmap, pagefile* pg)
    {
        gdt_setup();
        idt_init();
        heap_allocator::init_instance(pg, mmap);
        tty = new direct_text_render { fb, __startup_font, 0x00FFFFFF, 0 };
        tty->cls();
        tty->print_text("Hi there!");
        tty->endl();
        while(1);
    }
}