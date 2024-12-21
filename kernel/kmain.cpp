#include "kernel/libk_decls.h"
#include "kernel/direct_text_render.hpp"
#include "kernel/idt_amd64.h"
#include "kernel/heap_allocator.hpp"
#include "memory"
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
        std::allocator<uint8_t> alloc{};
        uint8_t* is = alloc.allocate(4);
        is[0] = 0xDE;
        is[1] = 0xC0;
        is[2] = 0x01;
        is[3] = 0xC0;
        tty->print_hex(*reinterpret_cast<uint64_t*>(is));
        alloc.deallocate(is, 4);
        while(1);
    }
}