#include "libk_decls.h"
#include "direct_text_render.hpp"
#include "arch/idt_amd64.h"
#include "heap_allocator.hpp"
#include "bits/icxxabi.h"
using namespace std;
extern psf2_t* __startup_font;
direct_text_render startup_tty;
extern "C" void _init();
extern "C"
{
    extern void gdt_setup();
    void kmain(sysinfo_t* fb, mmap_t* mmap, pagefile* pg)
    {
        _init();
        cli();
        nmi_disable();
        gdt_setup();
        idt_init();
        heap_allocator::init_instance(pg, mmap);
        new (&startup_tty) direct_text_render{ fb, __startup_font, 0x00FFFFFF, 0 };
        nmi_enable();
        sti();
        startup_tty.cls();
        startup_tty.print_line("Hello world!");
        while(1);
        __cxa_finalize(0);
    }
    __attribute__((noreturn)) void abort() { startup_tty.endl(); startup_tty.print_line("ABORT"); while(1) { asm volatile("hlt" ::: "memory"); } }
    void panic(const char* msg) { startup_tty.print_text("ERROR: "); startup_tty.print_line(msg); }
}
