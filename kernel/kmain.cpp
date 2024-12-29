#include "libk_decls.h"
#include "direct_text_render.hpp"
#include "arch/idt_amd64.h"
#include "heap_allocator.hpp"
#include "bits/icxxabi.h"
#include "string"
#include "vector"
extern psf2_t* __startup_font;
direct_text_render tty;
extern "C" void _init();
extern "C"
{
    extern void gdt_setup();
    void kmain(sysinfo_t* fb, mmap_t* mmap, pagefile* pg)
    {
        _init();
        gdt_setup();
        idt_init();
        heap_allocator::init_instance(pg, mmap);
        new (&tty) direct_text_render{ fb, __startup_font, 0x00FFFFFF, 0 };
        tty.cls();
        tty.print_line("Hello world!");
        tty.print_line("FB: " + std::to_string(fb));
        std::vector<mmap_entry> entries{mmap->entries, mmap->entries + mmap->num_entries};
        for(mmap_entry e : entries)
        {
            if(e.type == AVAILABLE || e.type == MMIO)  tty.print_line("ENTRY: addr " + std::to_string(reinterpret_cast<void*>(e.addr)) + " and length " + std::to_string(e.len));
        }
        while(1);
        __cxa_finalize(0);
    }
    __attribute__((noreturn)) void abort() { tty.endl(); tty.print_line("ABORT"); while(1) { asm volatile("hlt" ::: "memory"); } }
    void panic(const char* msg) { tty.print_text("ERROR: "); tty.print_line(msg); }
}
