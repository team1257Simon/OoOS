#include "direct_text_render.hpp"
#include "arch/idt_amd64.h"
#include "heap_allocator.hpp"
#include "rtc.h"
#include "bits/icxxabi.h"
#include "keyboard_driver.hpp"
#include "stdlib.h"
#include "bits/stdexcept.h"
#include "fs/data_buffer.hpp"
using namespace std;
bool can_print = false;
extern psf2_t* __startup_font;
extern "C" uint64_t errinst;
direct_text_render startup_tty;
extern uintptr_t translate_vaddr(vaddr_t addr);
constexpr static const char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' }; 
static char dbgbuf[19]{'0', 'x'};
void buffer_test()
{
    data_buffer<char> tb{};
    tb.sputn("Eleventeenology hath been studied", 33);
    startup_tty.print_line(tb.__str());
}
void debug_print_num(uintptr_t num, int lenmax = 16)
{
    for(size_t i = lenmax + 1; i > 1; i--)
    {
        dbgbuf[i] = digits[num & 0xF];
        num >>= 4;
    }
    dbgbuf[lenmax + 2] = 0;
    startup_tty.print_text(dbgbuf);
}
extern "C" void direct_write(const char* str) { startup_tty.print_text(str); } 
__isr_registers void debug_ecode(byte idx, qword ecode)
{
    if(ecode) 
    {
        startup_tty.print_text("INT# ");
        debug_print_num(idx, 2);
        startup_tty.print_text(", ECODE ");
        debug_print_num(ecode);
        startup_tty.print_text(", RIP@ ");
        debug_print_num(errinst);
    }
}
extern "C" void _init();
extern "C"
{
    [[noreturn]] void abort() { startup_tty.endl(); startup_tty.print_line("ABORT"); while(1) { asm volatile("hlt" ::: "memory"); } }
    void panic(const char* msg) { startup_tty.print_text("ERROR: "); startup_tty.print_line(msg); }
    extern void* isr_table[];
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
        keyboard_driver_base* kb = get_kb_driver();
        kb->initialize();
        nmi_enable();
        sti();
        try
        {
            interrupt_table::add_interrupt_callback(debug_ecode);
            can_print = true;
            srand(syscall_time(0));
            startup_tty.print_line("Hello world");
            startup_tty.print_line(std::to_string(42));
            startup_tty.print_line(std::to_string(sysinfo));
            startup_tty.print_line(std::to_string(3.14159265358L));
            startup_tty.print_line(std::to_string(rand()));
            buffer_test();
            while(1);  
        } 
        catch(std::exception& e)
        {
            panic(e.what());
            abort();
            __builtin_unreachable();
        }
        __cxa_finalize(0);
        __builtin_unreachable();
    }
}
