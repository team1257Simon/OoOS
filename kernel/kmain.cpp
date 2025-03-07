#include "direct_text_render.hpp"
#include "arch/idt_amd64.h"
#include "arch/com_amd64.h"
#include "arch/kb_amd64.hpp"
#include "kernel_mm.hpp"
#include "kdebug.hpp"
#include "rtc.h"
#include "keyboard_driver.hpp"
#include "sched/task_ctx.hpp"
#include "sched/scheduler.hpp"
#include "sched/task_list.hpp"
#include "fs/hda_ahci.hpp"
#include "fs/ramfs.hpp"
#include "fs/ext.hpp"
#include "elf64_exec.hpp"
#include "bits/icxxabi.h"
#include "bits/dragon.hpp"
#include "stdlib.h"
#include "bits/stl_queue.hpp"
#include "algorithm"
#include "map"
extern psf2_t* __startup_font;
static direct_text_render startup_tty;
static serial_driver_amd64* com;
static sysinfo_t* sysinfo;
static ramfs testramfs;
static extfs test_extfs(94208UL);
static bool direct_print_enable{ false };
static bool fx_enable{ false };
static char dbgbuf[19]{ '0', 'x' };
const char* test_argv{ "Hello task world " };
std::atomic<bool> dbg_hold{ false };
extern uintptr_t saved_stack_ptr;
static const char digits[]{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
extern "C"
{
    extern uint64_t errinst;
    extern kframe_tag* __kernel_frame_tag;
    extern void* isr_table[];
    extern void* svinst;
    extern bool task_change_flag;
    extern unsigned char kernel_stack_base;
    extern unsigned char kernel_stack_top;
    extern unsigned char kernel_isr_stack_top;
    extern void test_fault();
    task_t kproc{};
}
filesystem* get_fs_instance() { task_ctx* task = current_active_task()->self; return task->get_vfs_ptr(); }
filesystem* create_task_vfs() { return &testramfs; /* TODO */ }
void kfx_save() { if(fx_enable) asm volatile("fxsave %0" : "=m"(kproc.fxsv) :: "memory"); }
void kfx_load() { if(fx_enable) asm volatile("fxrstor %0" :: "m"(kproc.fxsv) : "memory"); }
void xdirect_write(std::string const& str) { direct_write(str.c_str()); }
void xdirect_writeln(std::string const& str) { direct_writeln(str.c_str()); }
static int __xdigits(uintptr_t num) { return num ? div_roundup((sizeof(uint64_t) * CHAR_BIT) - __builtin_clzl(num), 4) : 1; }
static void __dbg_num(uintptr_t num, size_t lenmax) { if(!num) { direct_write("0"); return; } for(size_t i = lenmax + 1; i > 1; i--, num >>= 4) { dbgbuf[i] = digits[num & 0xF]; } dbgbuf[lenmax + 2] = 0; direct_write(dbgbuf); }
constexpr static bool has_ecode(byte idx) { return (idx > 0x09 && idx < 0x0F) || idx == 0x11 || idx == 0x15 || idx == 0x1D || idx == 0x1E; }
static void descr_pt(partition_table const& pt)
{
    for(partition_table::const_iterator i = pt.begin(); i != pt.end(); i++)
    {
        if(i->type_guid.data_full[0] == 0 && i->type_guid.data_full[1] == 0) continue;
        startup_tty.print_text("Partition at: ");
        startup_tty.print_text(std::to_string(i->start_lba));
        startup_tty.print_text(" to ");
        startup_tty.print_line(std::to_string(i->end_lba));
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
    startup_tty.print_text("initial map values: ");
    for(std::map<std::string, int>::iterator i = m.begin(); i != m.end(); ++i)
    {
        startup_tty.print_text(i->first);
        startup_tty.print_text(": ");
        startup_tty.print_text(std::to_string(i->second));
        startup_tty.print_text("; ");
    }
    startup_tty.endl();
    m.erase("gyeep");
    startup_tty.print_text("map values after erase: ");
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
    startup_tty.print_text("map values after reassign: ");
    for(std::map<std::string, int>::iterator i = m.begin(); i != m.end(); ++i)
    {
        startup_tty.print_text(i->first);
        startup_tty.print_text(": ");
        startup_tty.print_text(std::to_string(i->second));
        startup_tty.print_text("; ");
    }
    startup_tty.endl();
    m.clear();
    m.insert(std::make_pair("meep", 21));
    m["gyeep"] = 63;
    m.insert_or_assign("bweep", 42);
    m["fweep"] = 84;
    m.insert_or_assign("dreep", 105);
    startup_tty.print_text("map values after reset: ");
    for(std::map<std::string, int>::iterator i = m.begin(); i != m.end(); ++i)
    {
        startup_tty.print_text(i->first);
        startup_tty.print_text(": ");
        startup_tty.print_text(std::to_string(i->second));
        startup_tty.print_text("; ");
    }
    startup_tty.endl();
}
void str_tests()
{
    srand(syscall_time(nullptr));
    startup_tty.print_line(std::to_string(42));
    startup_tty.print_line(std::to_string(sysinfo));
    startup_tty.print_line(std::to_string(3.14159265358L));
    startup_tty.print_line(std::to_string(rand()));
    std::string test_str{ "I/like/to/eat/apples/and/bananas" };
    for(std::string s : std::ext::split(test_str, "/")) startup_tty.print_text(s + " ");
    startup_tty.endl();
    startup_tty.print_text("crc32c test: ");
    debug_print_num(crc32_calc(test_str.c_str(), 32), 8);
    startup_tty.endl();
    startup_tty.print_text("dragon test: ");
    std::ext::dragon<std::string> sdragon{};
    debug_print_num(sdragon(test_str));
    startup_tty.print_line(" (rawr)");
}
void vfs_tests()
{
    try 
    {
        // test folder creation
        testramfs.get_dir("test/files");
        file_node* n = testramfs.open_file("test/files/memes.txt");
        n->write("sweet dreams are made of memes\n", 31);
        testramfs.close_file(n);
        file_node* testout = testramfs.lndev("dev/com", com, 0, true);
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
int test_task_1(int argc, char** argv)
{
    direct_write(argv[0]);
    direct_writeln("in task 1");
    return 1;
}
int test_task_2(int argc, char** argv)
{
    direct_write(argv[0]);
    direct_writeln("in task 2");
    return 2;
}
void test_landing_pad()
{
    direct_writeln("Landed!");
    cli();
    task_ctx* ctx = get_gs_base<task_ctx>();
    long retv = ctx->exit_code;
    ctx->terminate();
    task_list::get().destroy_task(ctx->get_pid());
    startup_tty.print_line("returned " + std::to_string(retv));
    sti();
    while(1);
}
void task_tests()
{
    addr_t exit_test_fn{ &test_landing_pad };
    task_ctx* tt1 = task_list::get().create_system_task(&test_task_1, std::vector<const char*>{ test_argv }, S04, S04, priority_val::PVHIGH);
    task_ctx* tt2 = task_list::get().create_system_task(&test_task_2, std::vector<const char*>{ test_argv }, S04, S04);
    tt1->start_task(exit_test_fn);
    tt2->start_task(exit_test_fn);
    scheduler::get().start();
}
void extfs_tests()
{
    try
    {
        test_extfs.initialize();
        startup_tty.print_line("init complete");
        directory_node* dn = test_extfs.get_dir("files");
        startup_tty.print_text("dirnode addr: ");
        startup_tty.print_line(std::to_string(dn));
        file_node* fn = test_extfs.open_file("files/memes.txt");
        startup_tty.print_text("filenode addr: ");
        startup_tty.print_line(std::to_string(fn));
        fn->write("derple blerple", 14);
        test_extfs.close_file(fn);
    }
    catch(std::exception& e) { panic(e.what());}
}
void elf64_tests()
{
    if(test_extfs.has_init()) try
    {
        file_node* tst = test_extfs.open_file("test.elf");
        startup_tty.print_line("test.elf size: " + std::to_string(tst->size()));
        elf64_executable test_exec(tst);
        test_extfs.close_file(tst);
        if(test_exec.load())
        {
            elf64_program_descriptor const* desc = std::addressof(test_exec.describe());
            startup_tty.print_line("Entry at " + std::to_string(desc->entry));
            startup_tty.print_line("Stack at " + std::to_string(desc->prg_stack));
            task_ctx* task = task_list::get().create_user_task(*desc, std::vector<const char*>{ "TEST.ELF" });
            file_node* c = task->get_vfs_ptr()->lndev("com", com, 0);
            task->set_stdio_ptrs(c, c, c);
            task->start_task();
            user_entry(task->task_struct.self);
        }
        else startup_tty.print_line("Executable failed to validate");
    }
    catch(std::exception& e) { panic(e.what()); }
}
static const char* codes[] = 
{
    "#DE [Division by Zero]",
    "#DB [Debug Trap]",
    "NMI [Non-Maskable Interrupt]",
    "#BP [Breakpoint Trap]",
    "#OF [Overflow Error]",
    "#BR [Bounds Check Error]",
    "#UD [Invalid Opcode]",
    "#NM [No Device Available Error]",
    "#DF [Double Fault]",
    "Coprocessor Overrun (Deprecated)",
    "#TS [TSS Invalid]",
    "#NP [Non-Present Segment]",
    "#SS [Stack Segment Fault]",
    "#GP [General Protection Fault]",
    "#PF [Page Fault]",
    "[RESERVED INTERRUPT 0x0E]",
    "#MF [x87 Floating-Point Error]",
    "#AC [Alignment check]",
    "#MC [Machine Check Exception]",
    "#XM [SIMD Floating-Point Error]",
    "#VE [Virtualization Exception]",
    "#CP [Control Protection Exception]",
    "[RESERVED INTERRUPT 0x16]",
    "[RESERVED INTERRUPT 0x17]",
    "[RESERVED INTERRUPT 0x18]",
    "[RESERVED INTERRUPT 0x19]",
    "[RESERVED INTERRUPT 0x1A]",
    "[RESERVED INTERRUPT 0x1B (ELEVENTEEN)]",
    "#HV [Hypervisor Injection Exception]",
    "#VC [VMM Communication Exception]",
    "#SX [Security Exception]",
    "[RESERVED INTERRUPT 0x1F]"
};
void run_tests()
{
    // The current highlight of this OS (if you can call it that) is that I, an insane person, decided to make it possible to use lambdas for ISRs.
    interrupt_table::add_interrupt_callback(LAMBDA_ISR(byte idx, qword ecode)
    {
        if(idx < 0x20) 
        {
            startup_tty.print_text(codes[idx]);
            if(has_ecode(idx)) { startup_tty.print_text("("); __dbg_num(ecode, __xdigits(ecode)); startup_tty.print_text(")"); }
            if(errinst) { startup_tty.print_text(" at instruction "); __dbg_num(errinst, __xdigits(errinst)); }
            if(idx == 0x0E) 
            {
                uint64_t fault_addr;
                asm volatile("movq %%cr2, %0" : "=a"(fault_addr) :: "memory");
                startup_tty.print_text("; page fault address = ");
                __dbg_num(fault_addr, __xdigits(fault_addr));
            }
            addr_t target = errinst ? addr_t(errinst) : addr_t(svinst);
            uint8_t* bytes = target;
            bytes[0] = 0xEB;
            bytes[1] = 0xFE;
        }
        else
        {
            startup_tty.print_text("Received interrupt ");
            __dbg_num(idx, 2);
            startup_tty.print_line(" from software.");
        }
    });
    // First test some of the specialized pseudo-stdlibc++ stuff, since a lot of the following code uses it
    startup_tty.print_line("string test...");
    str_tests();
    startup_tty.print_line("map test...");
    map_tests();
    // Some barebones drivers...the keyboard driver is kinda hard to have a static test for here so uh ye
    startup_tty.print_line("serial test...");
    if(com) { com->sputn("Hello Serial!\n", 14); com->pubsync(); }
    startup_tty.print_line("ahci test...");
    if(ahci_hda::is_initialized()) descr_pt(ahci_hda::get_partition_table());
    // Test the complicated stuff
    startup_tty.print_line("vfs tests...");
    vfs_tests();   
    startup_tty.print_line("extfs tests...");
    extfs_tests();
    if(test_extfs.has_init()) { startup_tty.print_line("elf64 tests..."); elf64_tests(); }
    startup_tty.print_line("task tests...");
    task_tests(); 
    startup_tty.print_line("complete");
}
extern "C"
{
    extern void _init();
    extern void gdt_setup();
    extern void do_syscall();
    extern void enable_fs_gs_insns();
    paging_table kernel_cr3() { return kproc.saved_regs.cr3; }
    void direct_write(const char* str) { if(direct_print_enable) startup_tty.print_text(str); }
    void direct_writeln(const char* str) { if(direct_print_enable) startup_tty.print_line(str); }
    void debug_print_num(uintptr_t num, int lenmax) { int len = num ? div_roundup((sizeof(uint64_t) * CHAR_BIT) - __builtin_clzl(num), 4) : 1; __dbg_num(num, std::min(len, lenmax)); direct_write(" "); }
    void debug_print_addr(addr_t addr) { debug_print_num(addr.full); }
    [[noreturn]] void abort() { if(com) { com->sputn("KERNEL ABORT\n", 13); com->pubsync(); } startup_tty.print_line("abort() called in kernel"); while(1); }
    __isrcall void panic(const char* msg) noexcept { startup_tty.print_text("ERROR: "); startup_tty.print_line(msg); if(com) { com->sputn("[KPANIC] ", 9); com->sputn(msg, std::strlen(msg)); com->sputn("\n", 1); com->pubsync(); } }
    __isrcall void klog(const char* msg) noexcept { startup_tty.print_line(msg); if(com) { com->sputn("[KERNEL] ", 9); com->sputn(msg, std::strlen(msg)); com->sputn("\n", 1); com->pubsync(); } }
    void attribute(sysv_abi) kmain(sysinfo_t* si, mmap_t* mmap)
    {
        // Most of the current kmain is tests...because ya know.
        asm volatile("movq %0, %%rsp" :: "r"(&kernel_stack_top) : "memory");
        asm volatile("movq %0, %%rbp" :: "r"(&kernel_stack_base) : "memory");
        // Don't want to get interrupted during early initialization...
        cli();
        // The GDT is only used to set up the IDT (as well as enabling switching rings), so setting it up after the heap allocator is fine.
        gdt_setup();
        // The actual setup code for the IDT just fills the table with the same trampoline routine that calls the dispatcher for interrupt handlers.
        idt_init();
        nmi_disable();
        kproc.self = &kproc;
        // This initializer is freestanding by necessity. It's called before _init because some global constructors invoke the heap allocator (e.g. the serial driver).
        kernel_memory_mgr::init_instance(mmap); 
        // Because we are linking a barebones crti.o and crtn.o into the kernel, we can control the invocation of global constructors by calling _init. 
        _init();
        // Someone (aka the OSDev wiki) told me I need to do this in order to get exception handling to work properly, so here we are. It's imlemented in libgcc.
        __register_frame(&__ehframe);
        init_tss(&kernel_isr_stack_top);
        enable_fs_gs_insns();
        set_kernel_gs_base(&kproc);
        kproc.saved_regs.cr3 = get_cr3();
        kproc.saved_regs.rsp = &kernel_stack_top;
        kproc.saved_regs.rbp = &kernel_stack_base;
        // The code segments and data segment for userspace are computed at offsets of 16 and 8, respectively, of IA32_STAR bits 63-48
        init_syscall_msrs(addr_t{ &do_syscall }, 0UL, 0x08ui16, 0x10ui16);     
        sysinfo = si;
        fadt_t* fadt = nullptr;
        // FADT really just contains the century register; if we can't find it, just ignore and set the value based on the current century as of writing
        if(sysinfo->xsdt) fadt = find_fadt(sysinfo->xsdt);
        if(fadt) rtc_driver::init_instance(fadt->century_register);
        else rtc_driver::init_instance();
        // The startup "terminal" just directly renders text to the screen using a font that's stored in a data section linked in from libk.
        new (&startup_tty) direct_text_render{ si, __startup_font, 0x00FFFFFF, 0 };
        startup_tty.cls();
        // The base keyboard driver object abstracts out low-level initialization code that could theoretically change for different implementations of keyboards.
        keyboard_driver_base* kb = get_kb_driver();
        kb->initialize();
        kb->add_listener([&](kb_data d) -> void { if(!(d.event_code & KEY_UP)) dbg_hold = false; });
        if(serial_driver_amd64::init_instance()) com = serial_driver_amd64::get_instance();
        nmi_enable();
        sti();
        // The structure kproc will not contain all the normal data, but it shells the "next task" pointer for the scheduler if there is no task actually running
        set_gs_base(&kproc);
        asm volatile("fxsave %0" : "=m"(kproc.fxsv) :: "memory");
        __builtin_memset(kproc.fxsv.xmm, 0, sizeof(fx_state::xmm));
        for(int i = 0; i < 8; i++) kproc.fxsv.stmm[i] = 0.L;
        fx_enable = true;
        scheduler::init_instance();
        startup_tty.print_line(pci_device_list::init_instance(sysinfo->xsdt) ? (ahci_driver::init_instance(pci_device_list::get_instance()) ? (ahci_hda::init_instance() ? "AHCI HDA init success" : "HDA adapter init failed") : "AHCI init failed") : "PCI enum failed");
        direct_print_enable = true;
        try
        {
            // Any theoretical exceptions encountered in the test methods will propagate out to here. std::terminate essentially does the same thing as this, but the catch block also prints the exception's message.
            run_tests();
            // The test tasks might trip after this point, so add a backstop to avoid any shenanigans
            while(1);  
        } 
        catch(std::exception& e)
        {
            panic(e.what());
            panic("unexpected error in tests");
            abort();
            __builtin_unreachable();
        }
        __cxa_finalize(0);
        __builtin_unreachable();
    }
}
