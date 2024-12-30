#include "libk_decls.h"
#include "direct_text_render.hpp"
#include "arch/idt_amd64.h"
#include "heap_allocator.hpp"
#include "rtc.h"
#include "bits/icxxabi.h"
#include "isr_table.hpp"
using namespace std;
extern psf2_t* __startup_font;
direct_text_render startup_tty;
extern "C" void _init();
extern "C"
{
    extern void gdt_setup();
    void kmain(sysinfo_t* sysinfo, mmap_t* mmap, pagefile* pg)
    {
        _init();
        cli();
        nmi_disable();
        gdt_setup();
        idt_init();
        heap_allocator::init_instance(pg, mmap);
        fadt_t* fadt = nullptr;
        if(sysinfo->xsdt) fadt = find_fadt(sysinfo->xsdt);
        if(fadt) rtc_driver::init_instance(fadt->century_register);
        else rtc_driver::init_instance();
        new (&startup_tty) direct_text_render{ sysinfo, __startup_font, 0x00FFFFFF, 0 };
        startup_tty.cls();
        startup_tty.print_line("Hello world!");
        interrupt_table::add_irq_handler(0, [&]() -> void { startup_tty.cr(); startup_tty.print_text(std::to_string(rtc_driver::get_instance().get_timestamp())); });
        nmi_enable();
        sti();
        while(1);
        __cxa_finalize(0);
    }
    __attribute__((noreturn)) void abort() { startup_tty.endl(); startup_tty.print_line("ABORT"); while(1) { asm volatile("hlt" ::: "memory"); } }
    void panic(const char* msg) { startup_tty.print_text("ERROR: "); startup_tty.print_line(msg); }
}
