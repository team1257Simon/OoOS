#include "arch/apic.hpp"
#include "arch/com_amd64.h"
#include "arch/cpu_time.hpp"
#include "arch/hpet_amd64.hpp"
#include "arch/idt_amd64.h"
#include "arch/kb_amd64.hpp"
#include "arch/net/e1000e.hpp"
#include "bits/icxxabi.h"
#include "fs/ext.hpp"
#include "fs/hda_ahci.hpp"
#include "fs/ramfs.hpp"
#include "fs/sysfs.hpp"
#include "net/protocol/arp.hpp"
#include "net/protocol/dhcp.hpp"
#include "sched/scheduler.hpp"
#include "sched/task_ctx.hpp"
#include "sched/task_list.hpp"
#include "algorithm"
#include "device_registry.hpp"
#include "direct_text_render.hpp"
#include "elf64_exec.hpp"
#include "elf64_shared.hpp"
#include "elf64_kernel_object.hpp"
#include "frame_manager.hpp"
#include "kdebug.hpp"
#include "kernel_mm.hpp"
#include "kernel_api.hpp"
#include "keyboard_driver.hpp"
#include "map"
#include "typeinfo"
#include "ow-crypt.h"
#include "prog_manager.hpp"
#include "rtc.h"
#include "shared_object_map.hpp"
#include "stdlib.h"
sysinfo_t* sysinfo;
extern psf2_t* __startup_font;
static direct_text_render startup_tty;
static com_amd64* com;
static ramfs testramfs;
static extfs test_extfs(94208UL);
volatile apic bsp_lapic{ 0U };
static bool direct_print_enable{ false };
static bool fx_enable{ false };
static char dbgbuf[19]{ '0', 'x' };
static const char test_argv[]{ "Hello task world " };
static char test_e1000e_drv[sizeof(e1000e)]{};
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
    extern bool debug_stop_flag;
    extern void test_fault();
    task_t kproc{};
}
filesystem* get_fs_instance() { if(!current_active_task()) return nullptr; return active_task_context()->get_vfs_ptr(); }
filesystem* create_task_vfs() { return std::addressof(test_extfs); /* TODO */ }
void kfx_save() { if(fx_enable) asm volatile("fxsave %0" : "=m"(kproc.fxsv) :: "memory"); }
void kfx_load() { if(fx_enable) asm volatile("fxrstor %0" :: "m"(kproc.fxsv) : "memory"); }
void xdirect_write(std::string const& str) { direct_write(str.c_str()); }
void xdirect_writeln(std::string const& str) { direct_writeln(str.c_str()); }
void xklog(std::string const& str) { klog(str.c_str()); }
static int __xdigits(uintptr_t num) { return num ? div_round_up((sizeof(uintptr_t) * CHAR_BIT) - __builtin_clzl(num), 4) : 1; }
static void __dbg_num(uintptr_t num, size_t lenmax) { if(!num) { direct_write("0"); return; } for(size_t i = lenmax + 1; i > 1; i--, num >>= 4) { dbgbuf[i] = digits[num & 0xF]; } dbgbuf[lenmax + 2] = 0; direct_write(dbgbuf); }
constexpr static bool has_ecode(byte idx) { return (idx > 0x09UC && idx < 0x0FUC) || idx == 0x11UC || idx == 0x15UC || idx == 0x1DUC || idx == 0x1EUC; }
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
void net_tests()
{
    if(pci_config_space* net_pci = pci_device_list::get_instance()->find(2, 0))
    {
        e1000e* test_dev = new(test_e1000e_drv) e1000e(net_pci);
        if(!test_dev->initialize()) panic("init failed");
        else
        {
            mac_t const& mac = test_dev->get_mac_addr();
            startup_tty.print_line("MAC: " + stringify(mac));
            protocol_ipv4& p_ip     = test_dev->add_protocol_handler<protocol_ipv4>(ethertype_ipv4);
            protocol_udp& p_udp     = p_ip.add_transport_handler<protocol_udp>(UDP);
            protocol_dhcp& p_dhcp   = p_udp.add_port<protocol_dhcp>(dhcp_client_port);
            p_dhcp.base->ip_resolver->check_presence("10.0.2.2"IPV4);
            p_dhcp.transition_state(ipv4_client_state::INIT);
            hpet.delay_us(2000UL);
            startup_tty.print_line("Got IP " + stringify(p_dhcp.ipconfig.leased_addr) + ", subnet mask " + stringify(p_dhcp.ipconfig.subnet_mask) + ", default gateway " + stringify(p_dhcp.ipconfig.primary_gateway) + ", and DNS server " + stringify(p_dhcp.ipconfig.primary_dns_server) + " from DHCP server " + stringify(p_dhcp.ipconfig.dhcp_server_addr) + ";");
            startup_tty.print_line("T1: " + std::to_string(p_dhcp.ipconfig.lease_renew_time) +"; T2: " + std::to_string(p_dhcp.ipconfig.lease_rebind_time) + "; total lease duration is " + std::to_string(p_dhcp.ipconfig.lease_duration));
        }
    }
    else panic("net device not found on PCI bus");
}
void map_tests()
{
    using map_type = std::unordered_map<std::string, int>;
    map_type m{};
    m.insert(std::make_pair("meep", 21));
    m["gyeep"] = 63;
    m.insert_or_assign("bweep", 42);
    m["fweep"] = 84;
    m.insert_or_assign("dreep", 105);
    startup_tty.print_text("initial map values: ");
    for(map_type::iterator i = m.begin(); i != m.end(); ++i)
    {
        startup_tty.print_text(i->first);
        startup_tty.print_text(": ");
        startup_tty.print_text(std::to_string(i->second));
        startup_tty.print_text("; ");
    }
    startup_tty.endl();
    m.erase("gyeep");
    startup_tty.print_text("map values after erase: ");
    for(map_type::iterator i = m.begin(); i != m.end(); ++i)
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
    for(map_type::iterator i = m.begin(); i != m.end(); ++i)
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
    for(map_type::iterator i = m.begin(); i != m.end(); ++i)
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
    srand(sys_time(nullptr));
    startup_tty.print_text(std::to_string(42) + " ");
    startup_tty.print_text(std::to_string(sysinfo) + " ");
    startup_tty.print_text(std::to_string(3.14159265358L) + " ");
    startup_tty.print_text(std::to_string(rand()) + " ");
    startup_tty.print_text(std::string(10UL, 'e') + " ");
    std::string test_str{ "I/like/to/eat/apples/and/bananas" };
    for(std::string s : std::ext::split(test_str, "/")) startup_tty.print_text(s + " ");
    startup_tty.endl();
    startup_tty.print_text("crc32c test: ");
    debug_print_num(crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(test_str.c_str()), test_str.size()), 8);
    startup_tty.endl();
    startup_tty.print_text("dragon test: ");
    std::ext::dragon<std::string> sdragon{};
    debug_print_num(sdragon(test_str));
    startup_tty.print_line(" (rawr)");
    std::string setting = create_hash_setting_string();
    startup_tty.print_line("crypt setting: " + setting);
    std::string crypto = create_crypto_string("fleedle deedle", setting);
    startup_tty.print_line("crypt of fleedle deedle: " + crypto);
}
void vfs_tests()
{
    try 
    {
        // test folder creation
        testramfs.open_directory("test/files");
        file_node* n = testramfs.open_file("test/files/memes.txt");
        n->write("sweet dreams are made of memes\n", 31);
        testramfs.close_file(n);
        file_node* testout = testramfs.lndev("dev/console", 0, com->get_device_id());
        n = testramfs.open_file("test/files/memes.txt");
        char teststr[32](0);
        // test device and file inodes
        n->read(&teststr[0], 31);
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
        klog("I: made it to the catch block; the above error was intentional!");
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
    cli();
    task_ctx* ctx = get_gs_base<task_ctx>();
    long retv = ctx->exit_code;
    ctx->terminate();
    free(ctx->allocated_stack);
    free(ctx->tls);
    tl.destroy_task(ctx->get_pid());
    startup_tty.print_line("returned " + std::to_string(retv));
    sti();
    while(1);
}
void task_tests()
{
    addr_t exit_test_fn(std::addressof(test_landing_pad));
    task_ctx* tt1 = tl.create_system_task(&test_task_1, std::vector<const char*>{ test_argv }, S04, S04, priority_val::PVHIGH);
    task_ctx* tt2 = tl.create_system_task(&test_task_2, std::vector<const char*>{ test_argv }, S04, S04);
    tt1->start_task(exit_test_fn);
    tt2->start_task(exit_test_fn);
    sch.start();
}
void extfs_tests()
{
    if(hda_ahci::is_initialized()) try
    {
        test_extfs.initialize();
        test_extfs.open_directory("files");
        file_node* fn = test_extfs.open_file("files/memes.txt");
        fn->write("derple blerple\n", 15);
        test_extfs.close_file(fn);
        startup_tty.print_line("Wrote files/memes.txt");
    }
    catch(std::exception& e) { panic(e.what()); }
}
void dyn_elf_tests()
{
    if(test_extfs.has_init()) try
    {
        shared_object_map& sm = shared_object_map::get_globals();
        shared_object_map::iterator test_so = shared_object_map::get_ldso_object(nullptr);
        kmm.enter_frame(sm.shared_frame);
        addr_t sym = test_so->get_load_offset();
        startup_tty.print_line("Dynamic Linker SO name: " + test_so->get_soname());
        sym = test_so->entry_point();
        startup_tty.print_text("Dynamic Linker Entry: " + std::to_string(sym.as()) + " (");
        startup_tty.print_line(std::to_string(kmm.frame_translate(sym), std::ext::hex) + ")");
        sym = test_so->resolve_by_name("dlopen").second;
        startup_tty.print_text("Symbol dlopen: " + std::to_string(sym.as()) + " (");
        startup_tty.print_line(std::to_string(kmm.frame_translate(sym), std::ext::hex) + ")");
        sym = test_so->resolve_by_name("dlclose").second;
        startup_tty.print_text("Symbol dlclose: " + std::to_string(sym.as()) + " (");
        startup_tty.print_line(std::to_string(kmm.frame_translate(sym), std::ext::hex) + ")");
        kmm.exit_frame();
    }
    catch(std::exception& e) { panic(e.what()); }
}
void elf64_tests()
{
    if(test_extfs.has_init()) try
    {
        file_node* tst              = test_extfs.open_file("test.elf");
        elf64_executable* test_exec = prog_manager::get_instance().add(tst);
        test_extfs.close_file(tst);
        if(test_exec)
        {
            file_node* c    = test_extfs.get_file_or_null("/dev/console");
            if(!c) c        = test_extfs.lndev("/dev/console", 0, com->get_device_id());
            elf64_program_descriptor const& desc = test_exec->describe();
            startup_tty.print_line("Entry at " + std::to_string(desc.entry));
            sch.start();
            task_exec(desc, std::move(std::vector<const char*>{ "test.elf" }), std::move(std::vector<const char*>{ nullptr }), std::move(std::array{ c, c, c }));
            prog_manager::get_instance().remove(test_exec);
        }
        else startup_tty.print_line("Executable failed to validate");
        tst = test_extfs.open_file("sys/test_module.ko");
        elf64_kernel_object test_ko(tst);
        test_ko.load_module();
        test_extfs.close_file(tst);
    }
    catch(std::exception& e) { panic(e.what()); }
}
uint64_t end_read;
__isrcall void fn() { end_read = hpet_amd64::count_usec(); }
void hpet_tests()
{
    if(hpet_amd64::init_instance())
    {
        uint64_t start_read = hpet_amd64::count_usec();
        fence();
        hpet_amd64::delay_usec(1000UL, fn);
        fence();
        if(start_read > end_read) startup_tty.print_line("emulator too unstable; start read was " + std::to_string(start_read) + " microseconds"); 
        else startup_tty.print_line("time split: " + std::to_string(end_read - start_read));
    }
    else panic("hpet init failed");
}
constexpr static sysfs_backup_filenames test_backup_filenames
{
    .data_backup_file_name      { "objects.bak" },
    .index_backup_file_name     { "index.bak" },
    .extents_backup_file_name   { "blocks.bak" },
    .directory_backup_file_name { "tags.bak" }
};
struct test_sys_struct
{
    int something;
    guid_t something_else;
    char another_thing[8];
};
typedef decltype([](test_sys_struct const& tst) -> const char* { return tst.another_thing; }) test_extract;
typedef decltype([](const char* a, const char* b) -> bool { return std::strncmp(a, b, 8) == 0; }) test_pred;
typedef sysfs_hash_table<const char*, test_sys_struct, test_extract, std::hash<const char*>, test_pred> test_map;
void sysfs_node_test(sysfs& s)
{
    test_map tm(s, "some_test_data", sysfs_object_type::GENERAL_CONFIG);
    std::pair<test_map::value_handle, bool> added = tm.add({ 0, { .data_full{ 0x12345UL, 0xABCDEUL } }, "abcdefg" });
    added.first->something = 43;
    std::pair<test_map::value_handle, bool> again = tm.add({ 42, {}, "hijklmn" });
    again.first->something_else.data_full[0] = 0xEDCBAUL;
    again.first->something_else.data_full[1] = 0x54321UL;
}
void sysfs_tests()
{
    if(test_extfs.has_init()) try
    {
        test_extfs.open_directory("sys/sysfs");
        sysfs_file_ptrs sys_files
        {
            .data_file      { test_extfs.open_file("sys/sysfs/objects.dat") },
            .index_file     { test_extfs.open_file("sys/sysfs/index.dat") },
            .extents_file   { test_extfs.open_file("sys/sysfs/blocks.dat") },
            .directory_file { test_extfs.open_file("sys/sysfs/tags.dat") }
        };
        sysfs test_sysfs(sys_files);
        test_sysfs.init_blank(test_backup_filenames);
        startup_tty.print_line("Initialized in directory sys/sysfs");
        sysfs_node_test(test_sysfs);
        test_sysfs.sync();
        uint32_t ino = test_sysfs.find_node("some_test_data");
        if(ino)
        {
            sysfs_vnode& n = test_sysfs.open(ino);
            test_map tm(n);
            test_map::value_handle hdl = tm.find("abcdefg");
            startup_tty.print_line("string abcdefg is associated with the number " + std::to_string(hdl->something));
        }
        else startup_tty.print_line("[sysfs directory did not contain expected key]");
        test_extfs.close_file(sys_files.data_file);
        test_extfs.close_file(sys_files.index_file);
        test_extfs.close_file(sys_files.extents_file);
        test_extfs.close_file(sys_files.directory_file);
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
constexpr auto test_dbg_callback = [] __isrcall (byte idx, qword ecode) -> void
{
    if(get_gs_base<task_t>() != std::addressof(kproc)) return;
    if(idx < 0x20) 
    {
        startup_tty.print_text(codes[idx]);
        if(has_ecode(idx))
        {
            startup_tty.print_text("(");
            __dbg_num(ecode, __xdigits(ecode));
            startup_tty.print_text(")");
        }
        if(svinst && !errinst) { errinst = addr_t(svinst); }
        if(errinst){ startup_tty.print_text(" at instruction "); __dbg_num(errinst, __xdigits(errinst)); }
        if(idx == 0x0EUC) 
        {
            uint64_t fault_addr;
            asm volatile("movq %%cr2, %0" : "=a"(fault_addr) :: "memory");
            startup_tty.print_text("; page fault address = ");
            __dbg_num(fault_addr, __xdigits(fault_addr));
        }
        if(idx != 0x01) debug_stop_flag = true;
        else startup_tty.endl();
    }
    else if(idx != 0xFF)
    {
        startup_tty.print_text("Received interrupt ");
        __dbg_num(idx, 2);
        startup_tty.print_line(" from software.");
    }
};
void run_tests()
{
    interrupt_table::add_interrupt_callback(test_dbg_callback);
    // First test some of the specialized pseudo-stdlibc++ stuff, since a lot of the following code uses it
    startup_tty.print_line("string test...");
    str_tests();
    startup_tty.print_line("map test...");
    map_tests();
    // Some barebones drivers...the keyboard driver is kinda hard to have a static test for here so uh ye
    startup_tty.print_line("serial test...");
    if(com) { com->sputn("Hello Serial!\n", 14); com->pubsync(); }
    startup_tty.print_line("ahci test...");
    if(hda_ahci::is_initialized()) descr_pt(hda_ahci::get_instance()->get_partition_table());
    startup_tty.print_line("hpet test...");
    hpet_tests();
    startup_tty.print_line("net test...");
    net_tests();
    // Test the complicated stuff
    startup_tty.print_line("vfs tests...");
    vfs_tests();   
    startup_tty.print_line("extfs tests...");
    extfs_tests();
    if(test_extfs.has_init()) 
    {
        startup_tty.print_line("sysfs tests...");
        sysfs_tests();
        shared_object_map::get_ldso_object(std::addressof(test_extfs));
        startup_tty.print_line("SO loader tests...");
        dyn_elf_tests();
        startup_tty.print_line("elf64 tests..."); 
        elf64_tests(); 
    }
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
    paging_table get_kernel_cr3() { return kproc.saved_regs.cr3; }
    void direct_write(const char* str) { if(direct_print_enable) startup_tty.print_text(str); }
    void direct_writeln(const char* str) { if(direct_print_enable) startup_tty.print_line(str); }
    void debug_print_num(uintptr_t num, int lenmax) { int len = num ? div_round_up((sizeof(uint64_t) * CHAR_BIT) - __builtin_clzl(num), 4) : 1; __dbg_num(num, std::min(len, lenmax)); direct_write(" "); }
    void debug_print_addr(addr_t addr) { debug_print_num(addr.full); }
    [[noreturn]] void abort() { if(com) { com->sputn("KERNEL ABORT\n", 13); com->pubsync(); } startup_tty.print_line("abort() called in kernel"); while(1); }
    __isrcall void panic(const char* msg) noexcept { startup_tty.print_text("E: "); startup_tty.print_line(msg); if(com) { com->sputn("[KERNEL] E: ", 12); com->sputn(msg, std::strlen(msg)); com->sputn("\n", 1); com->pubsync(); } }
    __isrcall void klog(const char* msg) noexcept { startup_tty.print_line(msg); if(com) { com->sputn("[KERNEL] ", 9); com->sputn(msg, std::strlen(msg)); com->sputn("\n", 1); com->pubsync(); } }
    void attribute(sysv_abi) kmain(sysinfo_t* si, mmap_t* mmap)
    {
        // Most of the current kmain is tests...because ya know.
        asm volatile("movq %0, %%rsp" :: "r"(std::addressof(kernel_stack_top)) : "memory");
        asm volatile("movq %0, %%rbp" :: "r"(std::addressof(kernel_stack_base)) : "memory");
        // Don't want to get interrupted during early initialization...
        cli();
        // The GDT is only used to set up the IDT (as well as enabling switching rings), so setting it up after the heap allocator is fine.
        gdt_setup();
        // The actual setup code for the IDT just fills the table with the same trampoline routine that calls the dispatcher for interrupt handlers.
        idt_init();
        nmi_disable();
        sysinfo = si;
        kproc.self = std::addressof(kproc);
        // This initializer is freestanding by necessity. It's called before _init because some global constructors invoke the heap allocator (e.g. the serial driver).
        kernel_memory_mgr::init_instance(mmap); 
        // Because we are linking a barebones crti.o and crtn.o into the kernel, we can control the invocation of global constructors by calling _init. 
        _init();
        // Someone (aka the OSDev wiki) told me I need to do this in order to get exception handling to work properly, so here we are. It's imlemented in libgcc.
        __register_frame(std::addressof(__ehframe));
        init_tss(std::addressof(kernel_isr_stack_top));
        enable_fs_gs_insns();
        set_kernel_gs_base(std::addressof(kproc));
        kproc.saved_regs.cr3 = get_cr3();
        kproc.saved_regs.rsp = std::addressof(kernel_stack_top);
        kproc.saved_regs.rbp = std::addressof(kernel_stack_base);
        // The code segments and data segment for userspace are computed at offsets of 16 and 8, respectively, of IA32_STAR bits 63-48
        init_syscall_msrs(addr_t(std::addressof(do_syscall)), 0x200UL, 0x08US, 0x10US);     
        fadt_t* fadt = nullptr;
        // FADT really just contains the century register; if we can't find it, just ignore and set the value based on the current century as of writing
        if(sysinfo->xsdt) fadt = find_fadt();
        if(fadt) rtc::init_instance(fadt->century_register);
        else rtc::init_instance();
        // The startup "terminal" just directly renders text to the screen using a font that's stored in a data section linked in from libk.
        new(std::addressof(startup_tty)) direct_text_render(si, __startup_font, 0x00FFFFFFU, 0);
        startup_tty.cls();
        // The base keyboard driver object abstracts out low-level initialization code that could theoretically change for different implementations of keyboards.
        keyboard_driver* kb = get_kb_driver();
        kb->initialize();
        kb->add_listener([&](kb_data d) -> void { if(!(d.event_code & KEY_UP)) dbg_hold = false; });
        if(com_amd64::init_instance()) com = com_amd64::get_instance();
        direct_print_enable = true;
        bsp_lapic.init();
        nmi_enable();
        sti();
        // The structure kproc will not contain all the normal data, but it shells the "next task" pointer for the scheduler if there is no task actually running.
        // It stores the state of the floating-point registers during ISRs, and its "next task" points at the calling process during a syscall.
        // If we ever attempt SMP, each processor will have its own one of these, but we'll burn that bridge when we get there. Er, cross it. Something.
        set_gs_base(std::addressof(kproc));
        asm volatile("fxsave %0" : "=m"(kproc.fxsv) :: "memory");
        array_zero(kproc.fxsv.xmm, sizeof(fx_state::xmm) / sizeof(int128_t));
        for(int i = 0; i < 8; i++) kproc.fxsv.stmm[i] = 0.L;
        fx_enable = true;
        scheduler::init_instance();
        hpet_amd64::init_instance();
        if(pci_device_list::init_instance())
        {
            ooos_kernel_module::init_api();
            if(ahci::init_instance(pci_device_list::get_instance()))
            {
                if(hda_ahci::init_instance())
                    test_extfs.attach_block_device(hda_ahci::get_instance());
                else startup_tty.print_line("HDA adapter init failed");
            }
            else startup_tty.print_line("AHCI init failed");
        }
        else startup_tty.print_line("PCI enum failed");
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
        __cxa_finalize(nullptr);
        __builtin_unreachable();
    }
}
