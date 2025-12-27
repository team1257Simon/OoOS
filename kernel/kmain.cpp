#include <arch/apic.hpp>
#include <arch/cpu_time.hpp>
#include <arch/hpet_amd64.hpp>
#include <arch/idt_amd64.h>
#include <arch/keyboard.hpp>
#include <arch/pci_device_list.hpp>
#include <bits/icxxabi.h>
#include <fs/ext.hpp>
#include <fs/delegate_hda.hpp>
#include <fs/ramfs.hpp>
#include <fs/sysfs.hpp>
#include <net/protocol/arp.hpp>
#include <net/protocol/dhcp.hpp>
#include <net/netdev_module.hpp>
#include <sched/scheduler.hpp>
#include <sched/task_ctx.hpp>
#include <sched/task_list.hpp>
#include <sched/worker.hpp>
#include <sched/worker_list.hpp>
#include <util/circular_queue.hpp>
#include <util/multiarray.hpp>
#include <algorithm>
#include <device_registry.hpp>
#include <direct_text_render.hpp>
#include <elf64_exec.hpp>
#include <elf64_shared.hpp>
#include <frame_manager.hpp>
#include <kdebug.hpp>
#include <kernel_mm.hpp>
#include <kernel_api.hpp>
#include <map>
#include <module_loader.hpp>
#include <typeinfo>
#include <ow-crypt.h>
#include <prog_manager.hpp>
#include <rtc.h>
#include <shared_object_map.hpp>
#include <stdlib.h>
#include <users.hpp>
sysinfo_t* sysinfo;
static direct_text_render startup_tty;
static device_stream* com;
static ramfs testramfs;
static extfs test_extfs(94208UL);
volatile apic bsp_lapic(0U);
static bool direct_print_enable{};
static bool fx_enable{};
static char dbgbuf[19] = "0x";
static char dbg_serial_io[2]{};
static const char test_arg[] = "Hello world ";
static ooos::ps2_controller test_ps2{};
static ooos::ps2_keyboard* test_kb{};
static char kb_pos[sizeof(ooos::ps2_keyboard)]{};
static ooos::delegate_hda test_delegate;
static std::ext::delegate_ptr<sysfs> test_sysfs(nullptr);
static const char digits[]{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };
extern "C"
{
	extern uint64_t errinst;
	extern kframe_tag* __kernel_frame_tag;
	extern void* isr_table[];
	extern void* svinst;
	extern bool task_change_flag;
	extern unsigned char kernel_isr_stack_top;
	extern bool debug_stop_flag;
	extern void test_fault();
	attribute(section(".data.plocal")) task_t kproc{};
	int get_debug_char() { if(com && com->read(dbg_serial_io, 1UZ)) { return dbg_serial_io[0]; } return -1; }
	void put_debug_char(int ch) { dbg_serial_io[1] = static_cast<char>(ch); if(com) com->write(1UZ, dbg_serial_io + 1); }
}
void task_exec(elf64_program_descriptor const& prg, cstrvec&& args, cstrvec&& env, std::array<file_vnode*, 3>&& stdio_ptrs, addr_t exit_fn = nullptr, int64_t parent_pid = -1L, priority_val pv = priority_val::PVNORM, uint16_t quantum = 3);
filesystem* get_task_vfs() { if(!active_task_context()) return nullptr; return active_task_context()->get_vfs_ptr(); }
filesystem* create_task_vfs() { return std::addressof(test_extfs); /* TODO */ }
void kfx_save() { if(fx_enable) asm volatile("fxsave %0" : "=m"(kproc.fxsv) :: "memory"); }
void kfx_load() { if(fx_enable) asm volatile("fxrstor %0" :: "m"(kproc.fxsv) : "memory"); }
void xdirect_write(std::string const& str) { direct_write(str.c_str()); }
void xdirect_writeln(std::string const& str) { direct_writeln(str.c_str()); }
void xklog(std::string const& str) { klog(str.c_str()); }
static int __xdigits(uintptr_t num) { return num ? div_round_up((sizeof(uintptr_t) * CHAR_BIT) - __builtin_clzl(num), 4) : 1; }
static void __dbg_num(uintptr_t num, size_t lenmax) { if(!num) { direct_write("0"); return; } for(size_t i = lenmax + 1; i > 1; i--, num >>= 4) { dbgbuf[i] = digits[num & 0xF]; } dbgbuf[lenmax + 2] = 0; direct_write(dbgbuf); }
constexpr static bool has_ecode(byte idx) { return (idx > 0x09UC && idx < 0x0FUC) || idx == 0x11UC || idx == 0x15UC || idx == 0x1DUC || idx == 0x1EUC; }
static void delegate_ptr_cb_sysfs(void* ptr) { static_cast<sysfs*>(ptr)->sync(); }
static void descr_pt(partition_table const& pt)
{
	for(partition_table::const_iterator i = pt.begin(); i != pt.end(); i++)
	{
		if(i->type_guid.data_full[0] == 0 && i->type_guid.data_full[1] == 0) continue;
		direct_write("Partition at: ");
		xdirect_write(std::to_string(i->start_lba));
		direct_write(" to ");
		xdirect_writeln(std::to_string(i->end_lba));
	}
}
void task_exec(elf64_program_descriptor const& prg, cstrvec&& args, cstrvec&& env, std::array<file_vnode*, 3>&& stdio_ptrs, addr_t exit_fn, int64_t parent_pid, priority_val pv, uint16_t quantum)
{
	task_descriptor task
	{
		.program	= prg,
		.argv		= std::move(args),
		.parent_pid	= static_cast<spid_t>(parent_pid),
		.uid		= 0U,
		.gid		= 0U,
		.quantum	= quantum,
		.priority	= pv
	};
	task_ctx* ctx	= tl.create_user_task(std::move(task));
	ctx->env_vec	= std::move(env);
	ctx->init_task_state();
	ctx->set_stdio_ptrs(std::move(stdio_ptrs));
	if(exit_fn) ctx->start_task(exit_fn);
	else ctx->start_task();
	if(test_kb->has_listener(dynamic_cast<void*>(ctx->ctx_filesystem)))
	{
		ooos::keyboard_stdin* tie = test_kb->listener_for(dynamic_cast<void*>(ctx->ctx_filesystem)).target<ooos::keyboard_stdin>();
		if(tie)
			tie->activate(ctx->task_struct.task_ctl.task_pid);
		else direct_write("W: no keyboard tie");
	}
	else direct_write("W: no keyboard listener");
	user_entry(std::addressof(ctx->task_struct));
}
module_loader::iterator init_boot_module(boot_loaded_module& mod_desc)
{
	module_loader& loader = module_loader::get_instance();
	if(mod_desc.buffer && mod_desc.size) {
		xdirect_writeln("Loading module " + std::string(mod_desc.filename));
		return loader.add(mod_desc.buffer, mod_desc.size).first;
	}
	return loader.end();
}
ooos::abstract_module_base* get_boot_module(const char* filename)
{
	try
	{
		if(sysinfo->loaded_modules && sysinfo->loaded_modules->count)
		{
			for(size_t i = 0; i < sysinfo->loaded_modules->count; i++)
			{
				if(!__builtin_strcmp(filename, sysinfo->loaded_modules->descriptors[i].filename))
				{
					module_loader::iterator result = init_boot_module(sysinfo->loaded_modules->descriptors[i]);
					if(result != module_loader::get_instance().end())
						return result->second.get_module();
					break;
				}
			}
		}
	}
	catch(std::exception& e) { panic(e.what()); }
	return nullptr;
}
device_stream* load_com_module()
{
	ooos::abstract_module_base* mod = get_boot_module("X64_COM.KO");
	if(mod) return mod->as_device<char>();
	return nullptr;
}
ooos::block_io_provider_module* load_ahci_module()
{
	ooos::abstract_module_base* mod = get_boot_module("X64_AHCI.KO");
	if(mod) return mod->as_blockdev();
	return nullptr;
}
ooos::abstract_netdev_module* load_e1000e_module()
{
	ooos::abstract_module_base* mod	= get_boot_module("IE1000E.KO");
	if(mod) return dynamic_cast<ooos::abstract_netdev_module*>(mod);
	return nullptr;
}
void net_tests()
{
	if(ooos::abstract_netdev_module* test_dev = load_e1000e_module())
	{
		mac_t const& mac        = test_dev->get_mac_addr();
		xdirect_writeln("MAC: " + stringify(mac));
		protocol_ipv4& p_ip     = test_dev->add_protocol_handler<protocol_ipv4>(ethertype_ipv4);
		protocol_udp& p_udp     = p_ip.add_transport_handler<protocol_udp>(UDP);
		protocol_dhcp& p_dhcp   = p_udp.add_port<protocol_dhcp>(dhcp_client_port);
		p_dhcp.base->ip_resolver->check_presence("10.0.2.2"IPV4);
		p_dhcp.transition_state(ipv4_client_state::INIT);
		hpet.delay_us(2000UL);
		xdirect_writeln(("Got IP " + stringify(p_dhcp.ipconfig.leased_addr)) + (", subnet mask " + stringify(p_dhcp.ipconfig.subnet_mask)) + (", default gateway " + stringify(p_dhcp.ipconfig.primary_gateway)) + (", and DNS server " + stringify(p_dhcp.ipconfig.primary_dns_server)) + (" from DHCP server " + stringify(p_dhcp.ipconfig.dhcp_server_addr)) + ";");
		xdirect_write(("T1: " + std::to_string(p_dhcp.ipconfig.lease_renew_time)) + ("; T2: " + std::to_string(p_dhcp.ipconfig.lease_rebind_time)) + ("; total lease duration is " + std::to_string(p_dhcp.ipconfig.lease_duration)) + ". ");
		direct_writeln("Initiating timer of 2 seconds to test schedule-on-delay.");
		time_t ts				= hpet.count_usec();
		scheduler::defer_sec(2UL, [ts]() -> void {
			time_t result = hpet.count_usec() - ts;
			xdirect_writeln("[timed " + std::to_string(result) + " microseconds]");
		});
	}
	else panic("net device module failed to load");
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
	xdirect_write("initial map values: ");
	for(map_type::iterator i = m.begin(); i != m.end(); ++i)
	{
		xdirect_write(i->first);
		direct_write(": ");
		xdirect_write(std::to_string(i->second));
		direct_write("; ");
	}
	dwendl();
	m.erase("gyeep");
	direct_write("map values after erase: ");
	for(map_type::iterator i = m.begin(); i != m.end(); ++i)
	{
		xdirect_write(i->first);
		direct_write(": ");
		xdirect_write(std::to_string(i->second));
		direct_write("; ");
	}
	dwendl();
	m["dreep"] = 45;
	m.insert_or_assign("fweep", 37);
	direct_write("map values after reassign: ");
	for(map_type::iterator i = m.begin(); i != m.end(); ++i)
	{
		xdirect_write(i->first);
		direct_write(": ");
		xdirect_write(std::to_string(i->second));
		direct_write("; ");
	}
	dwendl();
	m.clear();
	m.insert(std::make_pair("meep", 21));
	m["gyeep"] = 63;
	m.insert_or_assign("bweep", 42);
	m["fweep"] = 84;
	m.insert_or_assign("dreep", 105);
	direct_write("map values after reset: ");
	for(map_type::iterator i = m.begin(); i != m.end(); ++i)
	{
		xdirect_write(i->first);
		direct_write(": ");
		xdirect_write(std::to_string(i->second));
		direct_write("; ");
	}
	dwendl();
}
// 3 x 5 x 4 x 2 array, rank 4
constexpr static int8_t test_array[]
{
	/*---------------------------------------------------------------------------------------*/
	0,		1,		2,		3,		8,		9,		10,		11,		16, 	17,		18,		19,
	/*-----------------------*	|	*-------------------------*	 |	*------------------------*/
	4,		5,		6,		7,		12,		13,		14,		15,		20,		21,		22,		23,
	/*---------------------------------------------------------------------------------------*/
	24,		25,		26,		27,		32,		33,		34,		35,		40,		41,		42,		43,
	/*-----------------------*	|	*-------------------------*	 |	*------------------------*/
	28,		29,		30,		31,		36,		37,		38,		39,		44,		45,		46,		47,
	/*---------------------------------------------------------------------------------------*/
	48,		49,		50,		51,		56,		57,		58,		59,		64,		65,		66,		67,
	/*-----------------------*	|	*-------------------------*	 |	*------------------------*/
	52,		53,		54,		55,		60,		61,		62,		63,		68,		69,		70,		71,
	/*---------------------------------------------------------------------------------------*/
	72,		73,		74,		75,		80,		81,		82,		83,		88,		89,		90,		91,
	/*-----------------------*	|	*-------------------------*	 |	*------------------------*/
	76,		77,		78,		79,		84,		85,		86,		87,		92,		93,		94,		95,
	/*---------------------------------------------------------------------------------------*/
	96,		97,		98,		99,		104,	105,	106,	107,	112,	113,	114,	115,
	/*-----------------------*	|	*-------------------------*	 |	*------------------------*/
	100,	101,	102,	103,	108,	109,	110,	111,	116,	117,	118,	119
	/*---------------------------------------------------------------------------------------*/
};
constexpr static ooos::scale_vector<4UZ> test_dimensions{ 3UZ, 5UZ, 4UZ, 2UZ };
constexpr static ooos::multiarray<const int8_t, 4UZ> test_multi(test_array, test_dimensions);
void str_tests()
{
	srand(sys_time(nullptr));
	constexpr int the_answer 			= linear_combination<int>()(std::array{ 3, 2, 4, 1 }, std::array{ 2, 5, 5, 6 });
	constexpr int8_t other_answer		= test_multi[ooos::vec(2UZ, 1UZ, 2UZ, 0UZ)];
	xdirect_write(std::to_string(the_answer) + " ");
	xdirect_write(std::to_string(other_answer) + " ");
	xdirect_write(std::to_string(sysinfo) + " ");
	xdirect_write(std::to_string(3.14159265358L) + " ");
	xdirect_write(std::to_string(rand()) + " ");
	xdirect_write(std::string(10UL, 'e') + " ");
	std::string test_str("I/like/to/eat/apples/and/bananas");
	for(std::string s : std::ext::split(test_str, "/")) xdirect_write(s + " ");
	std::vector<std::string> v{ "Dewey", "Cheatem", "and Howe" };
	xdirect_writeln(std::ext::join(v, ", "));
	xdirect_write("crc32c test: ");
	debug_print_num(crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(test_str.c_str()), test_str.size()), 8);
	dwendl();
	direct_write("dragon test: ");
	std::ext::dragon<std::string> sdragon{};
	debug_print_num(sdragon(test_str));
	direct_writeln(" (rawr)");
	std::string setting = create_hash_setting_string();
	xdirect_writeln("crypt setting: " + setting);
	std::string crypto = create_crypto_string("fleedle deedle", setting);
	xdirect_writeln("crypt of fleedle deedle: " + crypto);
}
int test_worker_1(const char* arg)
{
	direct_write(arg);
	direct_writeln("in worker 1");
	return 1;
}
int test_worker_2(const char* arg)
{
	direct_write(arg);
	direct_writeln("in worker 2");
	return 2;
}
void extfs_tests()
{
	try
	{
		test_extfs.initialize();
		test_extfs.open_directory("files");
		file_vnode* fn = test_extfs.open_file("files/memes.txt");
		fn->write("derple blerple\n", 15);
		test_extfs.close_file(fn);
		xdirect_writeln("Wrote files/memes.txt");
	}
	catch(std::exception& e) { panic(e.what()); }
}
void dyn_elf_tests()
{
	if(test_extfs.has_init()) try
	{
		shared_object_map& sm				= shared_object_map::get_globals();
		shared_object_map::iterator test_so	= shared_object_map::get_ldso_object(nullptr);
		kmm.enter_frame(sm.shared_frame);
		addr_t sym							= test_so->get_load_offset();
		xdirect_writeln("Dynamic Linker SO name: " + test_so->get_soname());
		sym									= test_so->entry_point();
		xdirect_write("Dynamic Linker Entry: " + std::to_string(sym.as()) + " (");
		xdirect_writeln(std::to_string(kmm.frame_translate(sym), std::ext::hex) + ")");
		sym									= test_so->resolve_by_name("dlopen").second;
		xdirect_write("Symbol dlopen: " + std::to_string(sym.as()) + " (");
		xdirect_writeln(std::to_string(kmm.frame_translate(sym), std::ext::hex) + ")");
		sym									= test_so->resolve_by_name("dlclose").second;
		xdirect_write("Symbol dlclose: " + std::to_string(sym.as()) + " (");
		xdirect_writeln(std::to_string(kmm.frame_translate(sym), std::ext::hex) + ")");
		kmm.exit_frame();
	}
	catch(std::exception& e) { panic(e.what()); }
}
void elf64_tests()
{
	std::string console("/dev/console");
	std::string keybd("/dev/kbd");
	if(test_extfs.has_init()) try
	{
		file_vnode* tst             = test_extfs.open_file("test.elf");
		elf64_executable* test_exec	= prog_manager::get_instance().add(tst);
		test_extfs.close_file(tst);
		if(test_exec)
		{
			file_vnode* k							= test_extfs.get_file_or_null(keybd);
			if(!k) k								= test_extfs.tie_char_device(keybd, ooos::make_interface(*test_kb));
			if(k)
			{
				file_vnode* c    						= test_extfs.get_file_or_null(console);
				if(!c) c        						= test_extfs.lndev(console, 1, com->get_device_id());
				elf64_program_descriptor const& desc 	= test_exec->describe();
				xdirect_writeln("Entry at " + std::to_string(desc.entry));
				task_exec(desc, std::move(std::vector<const char*>{ "test.elf" }), std::move(std::vector<const char*>{ nullptr }), std::move(std::array{ k, c, c }));
				prog_manager::get_instance().remove(test_exec);
			}
			else xdirect_writeln("Keyboard listener failed to initialize");
		}
		else xdirect_writeln("Executable failed to validate");
	}
	catch(std::exception& e) { panic(e.what()); }
}
volatile uint64_t end_read;
void fn() { end_read = hpet_amd64::count_usec(); }
void hpet_tests()
{
	if(hpet_amd64::init_instance())
	{
		uint64_t start_read = hpet_amd64::count_usec();
		hpet_amd64::delay_usec(1000UL, fn);
		while(!end_read) pause();
		xdirect_writeln("time split: " + std::to_string(end_read - start_read) + " microseconds");
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
constexpr static sysfs_file_paths sys_files
{
	.data_file		= std::string("sys/objects.dat"),
	.index_file		= std::string("sys/index.dat"),
	.extents_file	= std::string("sys/blocks.dat"),
	.directory_file	= std::string("sys/tags.dat")
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
	std::pair<test_map::value_handle, bool> added	= tm.add({ 0, { .data_full{ 0x12345UL, 0xABCDEUL } }, "abcdefg" });
	added.first->something = 43;
	std::pair<test_map::value_handle, bool> again	= tm.add({ 42, {}, "hijklmn" });
	again.first->something_else.data_full[0]		= 0xEDCBAUL;
	again.first->something_else.data_full[1]		= 0x54321UL;
}
void sysfs_tests()
{
	if(test_extfs.has_init()) try
	{
		std::ext::delegate_ptr<sysfs>::on_acquire_release(delegate_ptr_cb_sysfs, delegate_ptr_cb_sysfs);
		test_extfs.open_directory("sys");
		test_sysfs.emplace(std::addressof(test_extfs), sys_files);
		test_sysfs->init_blank(test_backup_filenames);
		direct_writeln("Initialized in directory sys");
		sysfs_node_test(*test_sysfs);
		uint32_t ino					= test_sysfs->find_node("some_test_data");
		if(ino)
		{
			sysfs_vnode& n				= test_sysfs->open(ino);
			test_map tm(n);
			test_map::value_handle hdl	= tm.find("abcdefg");
			xdirect_writeln("string abcdefg is associated with the number " + std::to_string(hdl->something));
		}
		else xdirect_writeln("[sysfs directory did not contain expected key]");
	}
	catch(std::exception& e) { panic(e.what()); test_sysfs.release(); }
}
void circular_queue_tests()
{
	ooos::circular_queue<char> test_queue{};
	char buffer[2]{ 0, 0 };
	for(char c = 'a'; c < 'h'; c++ /* he said the thing! */) test_queue.push(c);
	size_t l = test_queue.length();
	for(size_t i = 0; i < l - 3; i++)
	{
		buffer[0] = test_queue.pop();
		direct_write(buffer);
		if(i + 1 < l - 3)
			direct_write(", ");
		else direct_write("; ");
	}
	for(char c = 'a'; c < 'j'; c++ /* oh, that's why they call it that. */) test_queue.push(c);
	test_queue.bump(4UZ);
	l = test_queue.length();
	for(size_t i = 0; i < l; i++)
	{
		buffer[0] = test_queue.pop();
		direct_write(buffer);
		if(i + 1 < l)
			direct_write(", ");
	}
	dwendl();
}
void worker_tests()
{
	ooos::worker* volatile w1	= wl.create_worker(std::bind(test_worker_1, test_arg));
	ooos::worker* volatile w2	= wl.create_worker(std::bind(test_worker_2, test_arg));
	cli();
	int i;
	if(__unlikely(i = start_worker(w1)))
	{
		cli();
		xdirect_writeln("returned " + std::to_string(i));
		wl.destroy(w1);
		w1 = nullptr;
	}
	else if(__unlikely(i = start_worker(w2)))
	{
		cli();
		xdirect_writeln("returned " + std::to_string(i));
		wl.destroy(w2);
		w2 = nullptr;
	}
	else direct_writeln("started workers");
	sti();
	while(w1 || w2) { pause(); }
}
void uam_tests()
{
	if(test_sysfs)
	{
		try
		{
			if(user_accounts_manager::init_instance(*test_sysfs))
			{
				direct_write("initialized UAM; ");
				user_accounts_manager& inst				= *user_accounts_manager::get_instance();
				inst.create_user("test_user", "test_pass", "/bin/sh", "someone testy", "/home/test_user");
				user_handle result						= inst.get_user("test_user");
				result->capabilities.system_permissions	|= escalate_process;
				if(result->check_pw("test_pass")) xdirect_writeln("created account test_user with uid " + std::to_string(result->uid) + " and gid " + std::to_string(result->gid));
				else direct_writeln("account failed to populate correctly");
			}
			else direct_writeln("UAM init failed");
		}
		catch(std::exception& e) { panic(e.what()); }
	}
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
constexpr auto test_dbg_callback = [](byte idx, qword ecode) -> void
{
	if(get_gs_base<task_t>() != std::addressof(kproc)) return;
	if(idx < 0x20)
	{
		direct_write(codes[idx]);
		if(has_ecode(idx))
		{
			direct_write("(");
			__dbg_num(ecode, __xdigits(ecode));
			direct_write(")");
		}
		if(svinst && !errinst) { errinst = addr_t(svinst); }
		if(errinst) { direct_write(" at instruction "); __dbg_num(errinst, __xdigits(errinst)); }
		if(idx == 0x0EUC)
		{
			uint64_t fault_addr;
			asm volatile("movq %%cr2, %0" : "=a"(fault_addr) :: "memory");
			direct_write("; page fault address = ");
			__dbg_num(fault_addr, __xdigits(fault_addr));
		}
		if(idx != 0x01) debug_stop_flag = true;
		else dwendl();
	}
	else if(idx != 0xFF)
	{
		direct_write("Received interrupt ");
		__dbg_num(idx, 2);
		direct_write(" from software.");
	}
};
void run_tests()
{
	sch.start();
	test_kb 		= new(std::addressof(kb_pos)) ooos::ps2_keyboard(test_ps2);
	test_kb->add_listener(test_kb, [](ooos::keyboard_event e) -> void
	{
		wchar_t ch 	= e;
		if(!e.kv_release && ch < 127)
		{
			if(e.kv_vstate.ctrl()) direct_putch('^');
			if(e.kv_vstate.alt()) direct_putch('~');
			direct_putch(ch);
		}
	});
	// First test some of the specialized pseudo-stdlibc++ stuff, since a lot of the following code uses it
	direct_writeln("string test...");
	str_tests();
	direct_writeln("data structure tests...");
	map_tests();
	circular_queue_tests();
	// Some barebones drivers...the keyboard driver is kinda hard to have a static test for here so uh ye
	if(__unlikely(!(com = load_com_module()))) direct_writeln("failed to load serial driver");
	direct_writeln("ahci test...");
	descr_pt(test_delegate.get_partition_table());
	direct_writeln("hpet test...");
	hpet_tests();
	direct_writeln("net test...");
	net_tests();
	// Test the complicated stuff
	direct_writeln("extfs tests...");
	extfs_tests();
	if(test_extfs.has_init())
	{
		direct_writeln("sysfs tests...");
		sysfs_tests();
		uam_tests();
		shared_object_map::get_ldso_object(std::addressof(test_extfs));
		direct_writeln("SO loader tests...");
		dyn_elf_tests();
		direct_writeln("elf64 tests...");
		elf64_tests();
	}
	direct_writeln("worker tests...");
	worker_tests();
	direct_writeln("complete");
}
static void __serial_write(std::string const& msg)
{
	if(com)
	{
		std::string str = msg + "\n";
		com->write(str.size(), str.c_str());
		com->sync();
	}
}
extern "C"
{
	extern void _init();
	extern void gdt_setup();
	extern void do_syscall();
	extern void enable_fs_gs_insns();
	paging_table get_kernel_cr3() { return kproc.saved_regs.cr3; }
	void dwclear() { if(direct_print_enable) startup_tty.cls(); }
	void dwendl() { if(direct_print_enable) startup_tty.endl(); }
	void direct_putch(wchar_t ch) { if(direct_print_enable) startup_tty.putc(ch); }
	void direct_write(const char* str) { if(direct_print_enable) startup_tty.print_text(str); }
	void direct_writeln(const char* str) { if(direct_print_enable) startup_tty.print_line(str); }
	void debug_print_num(uintptr_t num, int lenmax) { int len = num ? div_round_up((sizeof(uint64_t) * CHAR_BIT) - __builtin_clzl(num), 4) : 1; __dbg_num(num, std::min(len, lenmax)); direct_write(" "); }
	void debug_print_addr(addr_t addr) { debug_print_num(addr.full); }
	[[noreturn]] void abort() { __serial_write("KERNEL ABORT"); direct_writeln("abort() called in kernel"); while(1); }
	void klog(const char* msg) noexcept {
		direct_writeln(msg);
		__serial_write("[KERNEL] " + std::string(msg));
	}
	void panic(const char* msg) noexcept
	{
		bool prt			= direct_print_enable;
		direct_print_enable	= true;
		direct_write("E: ");
		direct_writeln(msg);
		__serial_write("[KERNEL] E: " + std::string(msg));
		direct_print_enable	= prt;
	}
	bool kernel_setup() try
	{
		tss_init(std::addressof(kernel_isr_stack_top));
		enable_fs_gs_insns();
		set_kernel_gs_base(std::addressof(kproc));
		kproc.saved_regs.cr3	= get_cr3();
		// The code segments and data segment for userspace are computed at offsets of 16 and 8, respectively, of IA32_STAR bits 63-48
		init_syscall_msrs(addr_t(std::addressof(do_syscall)), 0x200UL, 0x08US, 0x10US);
		fadt_t* fadt			= nullptr;
		// FADT really just contains the century register; if we can't find it, just ignore and set the value based on the current century as of writing
		if(sysinfo->xsdt) fadt	= find_fadt();
		if(fadt) rtc::init_instance(fadt->century_register);
		else rtc::init_instance();
		// The startup "terminal" just directly renders text to the screen using a font that's stored as an object in the kernel's data segment.
		new(std::addressof(startup_tty)) direct_text_render(sysinfo);
		direct_print_enable		= true;
		dwclear();
		bsp_lapic.init();
		nmi_enable();
		sti();
		// The structure kproc will not contain all the normal data, but it shells the "next task" pointer for the scheduler if there is no task actually running.
		// It stores the state of the floating-point registers during ISRs, and its "next task" points at the calling process during a syscall.
		// If we ever attempt SMP, each processor will have its own one of these, but we'll burn that bridge when we get there. Er, cross it. Something.
		set_gs_base(std::addressof(kproc));
		asm volatile("fxsave %0" : "=m"(kproc.fxsv) :: "memory");
		array_zero(kproc.fxsv.xmm, sizeof(fx_state::xmm) / sizeof(int128_t));
		array_zero(kproc.fxsv.stmm, sizeof(fx_state::stmm) / sizeof(long double));
		fx_enable 				= true;
		scheduler::init_instance();
		hpet_amd64::init_instance();
		if(pci_device_list::init_instance())
		{
			ooos::init_api();
			ooos::block_io_provider_module* hda	= load_ahci_module();
			if(__unlikely(!hda)) panic("HDA load failed");
			else
			{
				test_delegate.initialize(*hda);
				direct_writeln("Initialized delegate HDA");
				test_extfs.tie_block_device(std::addressof(test_delegate));
				return true;
			}
		}
		else direct_writeln("PCI enum failed");
		return false;
	}
	catch(std::exception& e)
	{
		panic("unexpected error in setup:");
		panic(e.what());
		return false;
	}
	void kmain(sysinfo_t* si, mmap_t* mmap)
	{
		// The bootloader gives us the machine with all memory identity-mapped, interrupts off, and an unspecified but valid GDT.
		// The entry point function sets up the initial kernel stack, so grab that information for the kernel process tag.
		asm volatile("movq %%rsp, %0" : "=g"(kproc.saved_regs.rsp)	:: "memory");
		asm volatile("movq %%rbp, %0" : "=g"(kproc.saved_regs.rbp)	:: "memory");
		// The GDT is only used to set up the IDT (as well as enabling switching rings), but it's still a requirement because Intel wants back-compatibility.
		gdt_setup();
		// The IDT descriptors will essentially index a table of trampoline functions, each of which passes the interrupt vector number as an argument to isr_dispatch.
		// If the interrupt is one that pushes an error code to the stack, the trampoline will also properly pop that before returning.
		idt_init();
		nmi_disable();
		sysinfo 				= si;
		kproc.self 				= std::addressof(kproc);
		// This initializer is freestanding by necessity. It's called before _init because some global constructors invoke the heap allocator.
		kernel_memory_mgr::init_instance(mmap);
		// Because we are linking a barebones crti.o and crtn.o into the kernel, we can control the invocation of global constructors by calling _init.
		_init();
		// The code in libgcc that initializes the exception-handling frame will malloc a significant amount of memory for the frame data.
		// It normally waits until an exception is thrown, but because of the unusual layout of the eh_frame data in the kernel, this can cause problems.
		// By forcing it to do the initialization now, before any other large allocations occur, we can hopefully avoid those.
		force_eager_fde();
		try
		{
			// Most of the current kmain is tests...because ya know.
			if(kernel_setup()) run_tests();
			// The test tasks might trip after this point, so add a backstop to avoid any shenanigans
			while(1);
		}
		catch(std::exception& e)
		{
			// Any theoretical exceptions encountered in the test methods will propagate out to here. std::terminate essentially does the same thing as this, but the catch block also prints the exception's message.
			panic("unexpected error in tests:");
			panic(e.what());
			while(1);
		}
		__cxa_finalize(nullptr);
		__builtin_unreachable();
	}
}