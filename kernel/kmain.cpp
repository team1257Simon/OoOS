#include "libk_decls.h"
#include "direct_text_render.hpp"
#include "arch/idt_amd64.h"
#include "heap_allocator.hpp"
#include "memory"
#include "functional"
#include "string.h"
#include "vector"
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
        tty->print_hex(*reinterpret_cast<uint32_t*>(is));
        tty->endl();
        alloc.deallocate(is, 4);
        std::allocator<int> blloc{}; 
        int* js = blloc.allocate(2);
        js[1] = 0xC001;
        js[0] = 0xC0DE;
        std::function<size_t(const char*)> fstrlen{strlen};
        tty->print_hex(fstrlen("stuff"));
        tty->endl();
        std::function<void(int, int)> test2 { [&](int a, int b) -> void { tty->print_hex(a + b); tty->endl(); } };
        test2(5, 5);
        test2(10, 8);
        std::vector<int> v{};
        v.reserve(6);
        v.push_back(0x100);
        v.push_back(0x200);
        v.insert(v.begin() + 1, js, js+2);
        tty->print_text("Size: ");
        tty->print_hex(v.size());
        tty->endl();
        tty->print_text("Cap: ");
        tty->print_hex(v.capacity());
        tty->endl();
        for(int i : v)
        {
            tty->print_hex(i);
            tty->endl();
        }
        v.pop_back();
        tty->print_text("Popped\n");
        for(int i : v)
        {
            tty->print_hex(i);
            tty->endl();
        }
        v.push_back(0x200);
        v.push_back(0x300);
        tty->print_text("Pushed\n");
        v.erase(v.begin(), v.begin() + 2);
        tty->print_text("Erased\n");
        for(int i : v)
        {
            tty->print_hex(i);
            tty->endl();
        }
        std::vector<int> u{js, js + 2};
        blloc.deallocate(js, 2);
        for(int i : u)
        {
            tty->print_hex(i);
            tty->endl();
        }
        while(1);
    }
}