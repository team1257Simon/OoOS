#include "direct_text_render.hpp"
#include "arch/idt_amd64.h"
#include "arch/com_amd64.h"
#include "heap_allocator.hpp"
#include "rtc.h"
#include "keyboard_driver.hpp"
#include "sched/task.h"
#include "fs/data_buffer.hpp"
#include "fs/hda_ahci.hpp"
#include "fs/ramfs.hpp"
#include "bits/icxxabi.h"
#include "stdlib.h"
#include "bits/stdexcept.h"
#include "bits/stl_queue.hpp"
#include "map"
#include "algorithm"
extern psf2_t* __startup_font;
extern "C" uint64_t errinst;
static std::atomic<uint64_t> t_ticks;
static direct_text_render startup_tty;
static bool direct_print_enable = false;
static serial_driver_amd64* com;
static sysinfo_t* sysinfo;    
ramfs testramfs;
static char dbgbuf[19]{'0', 'x'};
constexpr static const char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
static void __dbg_num(uintptr_t num, size_t lenmax) { for(size_t i = lenmax + 1; i > 1; i--, num >>= 4) { dbgbuf[i] = digits[num & 0xF]; } dbgbuf[lenmax + 2] = 0; direct_write(dbgbuf); }
void descr_pt(partition_table const& pt)
{
    for(partition_entry_t e : pt)
    {
        if(e.type_guid.data_full[0] == 0 && e.type_guid.data_full[1] == 0) continue;
        startup_tty.print_text("Partition at: ");
        startup_tty.print_text(std::to_string(e.start_lba));
        startup_tty.print_text(" to ");
        startup_tty.print_line(std::to_string(e.end_lba));
    }
}
void map_tests()
{
    std::map<std::string, int> m{};
    m.insert(std::make_pair("meep", 21));
    m["gyeep"] = 63;
    m.insert_or_assign("bweep", 42);
    m["fweep"] = 84;
    m.insert_or_assign("dreep", 105);
    startup_tty.print_line("initial map values: ");
    for(std::map<std::string, int>::iterator i = m.begin(); i != m.end(); ++i)
    {
        startup_tty.print_text(i->first);
        startup_tty.print_text(": ");
        startup_tty.print_text(std::to_string(i->second));
        startup_tty.print_text("; ");
    }
    startup_tty.endl();
    m.erase("gyeep");
    startup_tty.print_line("map values after erase: ");
    for(std::map<std::string, int>::iterator i = m.begin(); i != m.end(); ++i)
    {
        startup_tty.print_text(i->first);
        startup_tty.print_text(": ");
        startup_tty.print_text(std::to_string(i->second));
        startup_tty.print_text("; ");
    }
    startup_tty.endl();
    m["dreep"] = 45;
    m.insert_or_assign("fweep", 37);
    startup_tty.print_line("map values after reassign: ");
    for(std::map<std::string, int>::iterator i = m.begin(); i != m.end(); ++i)
    {
        startup_tty.print_text(i->first);
        startup_tty.print_text(": ");
        startup_tty.print_text(std::to_string(i->second));
        startup_tty.print_text("; ");
    }
    startup_tty.endl();
}
void serial_tests()
{
    if(com)
    {
        interrupt_table::add_irq_handler(4, LAMBDA_ISR() { size_t n = com->in_avail(); char buf[n]; com->sgetn(buf, n); direct_write("[ "); for(size_t i = 0; i < n; i++) { debug_print_num(buf[i], 2); } direct_write("]"); });
        com->sputn("Hello Serial!\n", 14);
        com->pubsync();
    }
}
void str_tests()
{
    srand(syscall_time(0));
    startup_tty.print_line(std::to_string(42));
    startup_tty.print_line(std::to_string(sysinfo));
    startup_tty.print_line(std::to_string(3.14159265358L));
    startup_tty.print_line(std::to_string(rand()));
    std::string test_str{ "I/like/to/eat/apples/and/bananas" };
    std::vector<std::string> test_vec = std::ext::split(test_str, "/");
    for(std::string s : test_vec) startup_tty.print_text(s + " ");
    startup_tty.endl();
}
void ahci_tests()
{
    uint64_t tk = t_ticks;
    while(tk < 10) { tk = t_ticks; BARRIER; }
    startup_tty.print_line(pci_device_list::init_instance(sysinfo->xsdt) ? (ahci_driver::init_instance(pci_device_list::get_instance()) ? (ahci_hda::init_instance() ? "AHCI HDA init success" : "HDA adapter init failed") : "AHCI init failed") : "PCI enum failed");
    if(ahci_hda::is_initialized()) descr_pt(ahci_hda::get_partition_table());
}
void vfs_tests()
{
    try 
    {
        // test folder creation
        testramfs.get_folder("test/files");
        file_inode* n = testramfs.open_file("test/files/memes.txt");
        n->write("sweet dreams are made of memes\n", 31);
        testramfs.close_file(n);
        file_inode* testout = testramfs.lndev("dev/com", com, true);
        n = testramfs.open_file("test/files/memes.txt");
        char teststr[32](0);
        // test device and file inodes
        n->read(teststr, 31);
        testout->write(teststr, 31);
        testout->fsync();
        testramfs.close_file(n);
        testramfs.close_file(testout);
        // test linking
        testramfs.link("test/files/memes.txt", "test/stuff/dreams.txt", true);
        n = testramfs.open_file("test/stuff/dreams.txt");
        char test2str[17](0);
        n->read(test2str, 16);
        direct_writeln(test2str);
        testramfs.close_file(n);
        testramfs.unlink("test/files", true, true);
        // test error condition(s)
        testramfs.open_file("test/files/memes.txt/dreams.txt");
    }
    catch(std::exception& e)
    {
        panic(e.what());
        startup_tty.print_line("NOTE: made it to the catch block; the above error was intentional!");
    }
}
void queue_tests()
{
    using namespace std;
    using namespace std::ext;
    try 
    {
        resettable_queue<string> q1{};
        resettable_queue<string> q2{};
        q1.emplace("blerple");
        q1.emplace("derple");
        q1.emplace("merple");
        q2.emplace("yerple");
        q2.emplace("zerple");
        q2.emplace("herple");
        startup_tty.print_line(q1.pop());
        startup_tty.print_line(q1.pop());
        string* ptr = q1.unpop();
        if(ptr) startup_tty.print_line(*ptr);
        else startup_tty.print_line("( :( )");
        q1.transfer(q2, 2);
        for(resettable_queue<string>::iterator i = q2.begin(); i != q2.end(); i++) startup_tty.print_line(*i);
        for(resettable_queue<string>::iterator i = q1.begin(); i != q1.end(); i++) startup_tty.print_line(*i);  
    }
    catch(std::exception& e) { panic(e.what()); }
}
void run_tests()
{
    interrupt_table::add_interrupt_callback(LAMBDA_ISR(byte idx, qword ecode)
    {
        if(idx < 0x20) 
        {
            startup_tty.print_text("INT# ");
            __dbg_num(idx, 2);
            if(ecode)
            {
                startup_tty.print_text(", ECODE ");
                __dbg_num(ecode, 8);
            }
            startup_tty.print_text(", RIP@ ");
            __dbg_num(errinst, 10);
            if(idx == 0x0E)
            {
                uint64_t fault_addr;
                asm volatile("movq %%cr2, %0" : "=a"(fault_addr) :: "memory");
                startup_tty.print_text("; fault addr = ");
                __dbg_num(fault_addr, 16);
            }
            while(1);
            __builtin_unreachable();
        }
    });
    interrupt_table::add_irq_handler(0, LAMBDA_ISR() { t_ticks++; });
    startup_tty.print_line("string test...");
    str_tests();
    startup_tty.print_line("serial test...");
    serial_tests();
    startup_tty.print_line("disk test...");
    ahci_tests();
    startup_tty.print_line("map test...");
    map_tests();
    startup_tty.print_line("vfs tests...");
    vfs_tests();
    startup_tty.print_line("queue tests...");
    queue_tests();
    startup_tty.print_line("complete");
}
filesystem* get_fs_instance() { return &testramfs; }
void xdirect_write(std::string const& str) { direct_write(str.c_str()); }
void xdirect_writeln(std::string const& str) { direct_writeln(str.c_str()); }
extern "C"
{
    task_t kproc{};
    extern void _init();
    extern void* isr_table[];
    extern void* svinst;
    extern void gdt_setup();
    void direct_write(const char* str) { if(direct_print_enable) startup_tty.print_text(str); }
    void direct_writeln(const char* str) { if(direct_print_enable) startup_tty.print_line(str); }
    void debug_print_num(uintptr_t num, int lenmax) { __dbg_num(num, lenmax); direct_write(" "); }
    [[noreturn]] void abort() { uint64_t *sp; asm volatile("movq %%rsp, %0" : "=r"(sp) :: "memory"); debug_print_num(*sp); startup_tty.endl(); startup_tty.print_line("ABORT"); if(com) { com->sputn("ABORT\n", 6); com->pubsync(); } while(1) { asm volatile("hlt" ::: "memory"); } }
    __isrcall void panic(const char* msg) noexcept { startup_tty.print_text("ERROR: "); startup_tty.print_line(msg); if(com) { com->sputn("[KPANIC] ", 9); com->sputn(msg, std::strlen(msg)); com->sputn("\n", 1); com->pubsync(); } }
    void kmain(sysinfo_t* si, mmap_t* mmap)
    {
        cli();
        nmi_disable();
        kproc.self = &kproc;
        // This initializer is freestanding by necessity. It's called before _init because some global constructors invoke the heap allocator (e.g. the serial driver).
        heap_allocator::init_instance(mmap); 
        // Because we are linking a barebones crti.o and crtn.o into the kernel, we can control the invocation of global constructors by calling _init. 
        _init();
        // Someone (aka the OSDev wiki) told me I need to do this in order to get exception handling to work properly, so here we are. It's imlemented in libgcc.
        __register_frame(&__ehframe);
        // The GDT is only used to set up the IDT (as well as enabling switching rings), so setting it up after the heap allocator is fine.
        gdt_setup();
        // The actual setup code for the IDT just fills the table with the same trampoline routine that calls the dispatcher for interrupt handlers.
        idt_init();
        // The wrgsbase instruction will be enabled before the lidt method returns, so we can do this now.
        set_kernel_gs_base(&kproc);
        sysinfo = si;
        fadt_t* fadt = nullptr;
        if(sysinfo->xsdt) fadt = find_fadt(sysinfo->xsdt);
        if(fadt) rtc_driver::init_instance(fadt->century_register);
        else rtc_driver::init_instance();
        // The startup "terminal" just directly renders text to the screen using a font that's stored in a data section linked in from libk.
        new (&startup_tty) direct_text_render{ si, __startup_font, 0x00FFFFFF, 0 };
        startup_tty.cls();
        // The base keyboard driver object abstracts out low-level initialization code that could theoretically change for different implementations of keyboards.
        keyboard_driver_base* kb = get_kb_driver();
        kb->initialize();
        if(serial_driver_amd64::init_instance()) com = serial_driver_amd64::get_instance();
        nmi_enable();
        sti();
        direct_print_enable = true;
        try
        {
            // Any theoretical exceptions encountered in the test methods will propagate out to here. std::terminate essentially does the same thing as this, but the catch block also prints the exception's message.
            run_tests();
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
