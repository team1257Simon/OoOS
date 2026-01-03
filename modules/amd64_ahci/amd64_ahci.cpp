#include "amd64_ahci.hpp"
amd64_ahci::config_type amd64_ahci::__cfg	= ahci_config();
static const char* last_msg_str				= nullptr;
constexpr size_t cl_offs_base				= 32UZ * sizeof(hba_cmd_header);
constexpr size_t fis_offs_base				= 32UZ * cl_offs_base;
constexpr size_t table_offs_base			= fis_offs_base + 32UZ * sizeof(hba_fis);
template<ooos::condition_callable FT> static inline bool await_completion(time_t max_spin, FT&& ft);
static inline ahci_device check_type(hba_port const& p);
static inline size_t __cap_size(size_t a, size_t b) { return a < b ? a : b; }
static inline size_t __at_least(size_t s, size_t minsz) { return s > minsz ? s : minsz; }
static inline bool __port_data_busy(hba_port* port) { barrier(); dword i = port->task_file; barrier(); return i.lo.lo[0] /* busy */ || i.lo.lo[3]; /* drq */ }
static inline bool __port_cmd_busy(hba_port* port, unsigned slot) { barrier(); dword i = port->cmd_issue; barrier(); dword j = port->i_state; barrier(); return !j.lo.lo[5] /* processed */ && i[slot]; }
static inline bool __port_ack_stop(hba_port* port) { barrier(); dword i = port->cmd; barrier(); return !i.lo.hi[7] /* cr */ && !i.lo.hi[6]; /* fr */ }
static inline bool __port_ack_reset(hba_port* port) { barrier(); dword i = dword(port->s_status); barrier(); return (i.lo.lo & 0x0F) == port_present; }
static inline bool __port_can_start(hba_port* port) { barrier(); dword i = port->cmd; barrier(); return !i.lo.hi[7]; /* cr */ }
static inline bool __not_data_busy(hba_port* port) { return !__port_data_busy(port); }
static inline bool __not_cmd_busy(hba_port* port, unsigned slot) { return !__port_cmd_busy(port, slot); }
constexpr static size_t prdt_cap(size_t region_size) { return static_cast<size_t>((region_size - 32 * (sizeof(hba_fis) + sizeof(hba_cmd_header))) / (32 * sizeof(hba_prdt_entry))); }
size_t ahci_port::block_size() const noexcept { return 512UZ; }
amd64_ahci::amd64_ahci() noexcept = default;
ahci_port::ahci_port(amd64_ahci& m, hba_port& p, uint32_t i, ahci_device dt) : __module(m), __port(p), __device_type(dt), __idx(i) {}
ooos::generic_config_table& amd64_ahci::get_config() { return __cfg.generic; }
amd64_ahci::uidx_t amd64_ahci::count() const noexcept { return __device_count; }
size_t ahci_port::io_count(utticket task_ticket) const { return __port.command_list[task_ticket].prd_count; }
bool ahci_port::io_complete(utticket task_ticket) const { return !io_busy() && !__port_cmd_busy(std::addressof(__port), task_ticket); }
size_t ahci_port::command_table_size() const noexcept { return sizeof(hba_cmd_table) + __prdt_count * sizeof(hba_prdt_entry); }
size_t ahci_port::bytes_per_prdt_entry() const noexcept { return __at_least(__cap_size(ooos::get_element<5>(amd64_ahci::__cfg), static_cast<size_t>((1UZ << 22) - 1)), 4096UZ); }
size_t ahci_port::sectors_per_prdt_entry() const noexcept { return bytes_per_prdt_entry() / block_size(); }
size_t ahci_port::max_operation_blocks() const noexcept { return __prdt_count * sectors_per_prdt_entry(); }
bool amd64_ahci::__handoff_done() noexcept { return (dword(__abar->bios_os_handoff)[bos_bit] || dword(__abar->bios_os_handoff)[bb_bit]); }
ooos::abstract_block_device const& amd64_ahci::operator[](uidx_t idx) const
{
	if(__unlikely(idx > 32U) || !__ports[idx])
		return __empty_slot;
	return *__ports[idx];
}
ooos::abstract_block_device& amd64_ahci::operator[](uidx_t idx)
{
	if(__unlikely(idx > 32U) || !__ports[idx])
		return __empty_slot;
	return *__ports[idx];
}
ooos::blockdev_type amd64_ahci::type_at_index(uidx_t idx) const noexcept
{
	if(__unlikely(idx > 32U))
		return ooos::BDT_NONE;
	return __ports[idx]->device_type();
}
amd64_ahci::sidx_t amd64_ahci::index_of(ooos::blockdev_type dev_type) const noexcept
{
	for(sidx_t i = 0; i < 32; i++)
		if(__ports[i])
			if(__ports[i]->device_type() == dev_type)
				return i;
	return -1;
}
bool amd64_ahci::initialize()
{
	__ahci_pci_space = find_pci_device(dev_class_storage_controller, subclass_sata_controller);
	if(!__ahci_pci_space) {
		log("E: no PCI config space found for AHCI");
		return false;
	}
	bar_desc bar	= compute_bar_info(*__ahci_pci_space, 5);
	__abar			= static_cast<hba_mem*>(map_dma(bar.base_value, bar.base_size, bar.is_prefetchable));
	__dma_size      = __at_least(__cap_size(ooos::get_element<0>(__cfg), 2097152UZ), 65536UZ);
	__dma_block		= allocate_dma(__dma_size, bar.is_prefetchable);
	if(!__dma_block) {
		log("E: failed to allocate DMA");
		return false;
	}
	barrier();
	__ahci_pci_space->command.bus_master			= true;
	__ahci_pci_space->command.memory_space			= true;
	__ahci_pci_space->command.interrupt_disable		= false;
	barrier();
	__abar->ghc										|= hba_enable_ahci;
	barrier();
	time_t max_spin									= ooos::get_element<1>(__cfg);
	if(__abar->bios_os_handoff_sup)
	{
		__abar->bios_os_handoff						|= BIT(oos_bit);
		barrier();
		if(__unlikely(!await_completion(max_spin, std::bind(&amd64_ahci::__handoff_done, this)))) {
			log("E: BIOS handoff error");
			return false;
		}
	}
	barrier();
	for(int i = 0; i < 32; i++)
	{
		if(!dword(__abar->p_implemented)[i]) continue;
		hba_port& port	= __abar->ports[i];
		ahci_device d	= check_type(port);
		if(d == none) continue;
		__ports[i]		= new(__port_struct_data[__device_count++]) ahci_port(*this, port, i, d);
		if(__unlikely(!__ports[i]->init_port(__dma_block))) {
			logf("W: skipping port %i due to error in initialization", i);
			__device_count--;
		}
		barrier();
	}
	if(__unlikely(!__device_count)) {
		log("E: No AHCI devices");
		return false;
	}
	__irq_init();
	return true;
}
void amd64_ahci::finalize()
{
	if(last_msg_str)
		release_buffer(addr_t(last_msg_str), alignof(char));
	if(__dma_block)
		release_dma(__dma_block, __dma_size);
}
void amd64_ahci::__handle_irq(dword s)
{
	for(int i = 0; i < 32; i++)
	{
		if(__ports[i])
		{
			uint32_t s1					= __abar->ports[i].i_state;
			barrier();
			__abar->ports[i].i_state	= s1;
			barrier();
			if(s[i] && (s1 & hba_error)) __ports[i]->soft_reset();
		}
		barrier();
	}
}
void amd64_ahci::__irq_init()
{
	for(int i = 0; i < 32; i++)
	{
		if(__ports[i])
		{
			barrier();
			__abar->i_status			|= BIT(i);
			barrier();
			uint32_t s1					= __abar->ports[i].i_state;
			barrier();
			__abar->ports[i].i_state	= s1;
			barrier();
		}
	}
	try
	{
		on_irq(__ahci_pci_space->header_0x0.interrupt_line, [this] -> void
		{
			uint32_t s			= __abar->i_status;
			barrier();
			__abar->i_status	= s;
			if(s) __handle_irq(s);
		});
	}
	catch(...) { raise_error("no memory"); __builtin_unreachable(); }
}
bool ahci_port::init_port(addr_t region_base)
{
	__prdt_count				= __at_least(__cap_size(prdt_cap(ooos::get_element<0>(amd64_ahci::__cfg)), ooos::get_element<4>(amd64_ahci::__cfg)), 4UZ);
	size_t prdt_size			= __prdt_count * sizeof(hba_prdt_entry);
	ooos::eh_exit_guard guard(std::addressof(__module));
	if(__unlikely(setjmp(__module.ctx_jmp()))) {
		__module.logf("E (on port %u): %s", __idx, __module.ctx_msg());
		return false;
	}
	soft_reset();
	stop_port();
	addr_t cmdlist_base			= region_base.plus(__idx * cl_offs_base);
	addr_t fis_base				= region_base.plus(fis_offs_base + __idx * sizeof(hba_fis));
	__port.command_list			= cmdlist_base;
	__builtin_memset(cmdlist_base, 0, sizeof(hba_cmd_header) * 32);
	barrier();
	__port.fis_receive			= fis_base;
	__builtin_memset(fis_base, 0, sizeof(hba_fis));
	barrier();
	for(int i = 0; i < 32; i++)
	{
		addr_t table_base						= region_base.plus(table_offs_base + __idx * prdt_size + i * command_table_size());
		__port.command_list[i].command_table	= table_base;
		barrier();
		__builtin_memset(table_base, 0, prdt_size);
	}
	start_port();
	return true;
}
ooos::blockdev_type ahci_port::device_type() const noexcept
{
	switch(__device_type)
	{
		case sata:  return ooos::BDT_HDD;
		case atapi: return ooos::BDT_CDD;
		default:    return ooos::BDT_NONE;
	}
}
void ahci_port::soft_reset()
{
	time_t max_spin = ooos::get_element<1>(amd64_ahci::__cfg);
	if(__unlikely(!await_completion(max_spin, [this]() -> bool { return __port.cmd_issue == 0; }))) {
		__module.raise_error("hung port", 1);
		__builtin_unreachable();
	}
	barrier();
	__port.cmd	&= ~hba_cmd_start;
	barrier();
	await_completion(max_spin, [this]() -> bool { return (__port.cmd & hba_command_cr) == 0; });
	barrier();
	__port.cmd	|= hba_cmd_start;
	int s1		= -1, s2 = -1;
	barrier();
	dword slots	= __port.s_active | __port.cmd_issue;
	barrier();
	for(int i = 0; i < 32 && s1 < 0 && s2 < 0; i++) { if(!slots[i]) { if(s1 < 0) s1 = i; else { s2 = i; break; } } }
	if(__unlikely(s1 < 0 && s2 < 0)) {
		__module.raise_error("no slots", 2);
		__builtin_unreachable();
	}
	barrier();
	hba_cmd_header* c1	= new(std::addressof(__port.command_list[s1])) hba_cmd_header
	{
		.cmd_fis_len	= static_cast<uint8_t>(sizeof(fis_reg_h2d) / sizeof(uint32_t)),
		.reset			= true,
		.cl_busy		= true,
		.command_table	= __port.command_list[s1].command_table
	};
	barrier();
	__builtin_memset(c1->command_table, 0, command_table_size());
	barrier();
	new(static_cast<void*>(c1->command_table->cmd_fis)) fis_reg_h2d
	{
		.type		= reg_h2d,
		.ctype		= false,
		.control	= soft_reset_bit,
	};
	if(!await_completion(max_spin, std::bind(__not_data_busy, std::addressof(__port)))) return hard_reset();
	barrier();
	__port.s_active		|= BIT(s1);
	barrier();
	__port.cmd_issue	= BIT(s1);
	barrier();
	unsigned u1 		= static_cast<unsigned>(s1);
	unsigned u2			= static_cast<unsigned>(s2);
	if(await_completion(max_spin, std::bind(__not_cmd_busy, std::addressof(__port), u1)) && !(__port.i_state & hba_error))
	{
		hba_cmd_header* c2	= new(std::addressof(__port.command_list[s2])) hba_cmd_header
		{
			.cmd_fis_len	= static_cast<uint8_t>(sizeof(fis_reg_h2d) / sizeof(uint32_t)),
			.reset			= false,
			.cl_busy		= false,
			.command_table	= __port.command_list[s2].command_table
		};
		barrier();
		__builtin_memset(c2->command_table, 0, sizeof(hba_cmd_table));
		barrier();
		reinterpret_cast<fis_reg_h2d*>(c2->command_table->cmd_fis)->type = reg_h2d;
		barrier();
		__port.s_active		|= BIT(s2);
		barrier();
		__port.cmd_issue	|= BIT(s2);
		barrier();
		if(__unlikely(!await_completion(max_spin, std::bind(__not_cmd_busy, std::addressof(__port), u2)) || (__port.i_state & hba_error))) hard_reset();
	}
	else hard_reset();
}
void ahci_port::hard_reset()
{
	time_t max_spin		= ooos::get_element<1>(amd64_ahci::__cfg);
	stop_port();
	await_completion(max_spin, std::bind(__port_can_start, std::addressof(__port)));
	if(!(dword(__port.cmd).lo.lo[0] /* SUD */) && __module.__abar->sud_supported) { barrier(); __port.cmd |= hba_command_spin_up_disk; }
	barrier();
	__port.s_control	|= hba_control_det;
	barrier();
	for(unsigned spin	= 0; spin < max_spin; barrier(), spin++);
	barrier();
	__port.s_control	&= ~hba_control_det;
	barrier();
	if(__unlikely(!await_completion(max_spin, std::bind(__port_ack_reset, std::addressof(__port))))) __module.raise_error("hung port", 3);
	barrier();
	uint32_t bits		= __port.s_err;
	barrier();
	__port.s_err		|= bits;
	barrier();
	start_port();
}
void ahci_port::start_port()
{
	time_t max_spin		= ooos::get_element<2>(amd64_ahci::__cfg);
	if(__unlikely(!await_completion(max_spin, std::bind(__port_can_start, std::addressof(__port))))) __module.raise_error("hung port", 4);
	barrier();
	__port.cmd			|= hba_command_fre;
	barrier();
	__port.cmd			|= hba_cmd_start;
}
void ahci_port::stop_port()
{
	time_t max_spin		= ooos::get_element<2>(amd64_ahci::__cfg);
	barrier();
	__port.cmd			&= ~hba_cmd_start;
	barrier();
	__port.cmd			&= ~hba_command_fre;
	barrier();
	if(__unlikely(!await_completion(max_spin, std::bind(__port_ack_stop, std::addressof(__port))))) __module.raise_error("hung port", 5);
}
ahci_port::stticket ahci_port::__find_cmd_slot()
{
	barrier();
	uint32_t slots		= (__port.s_active | __port.cmd_issue);
	barrier();
	dword dwslots(slots);
	for(stticket i = 0; i < 32; i++) if(!(dwslots[i])) return i;
	return -1;
}
void ahci_port::__cmd_issue(utticket slot)
{
	time_t max_spin			= ooos::get_element<1>(amd64_ahci::__cfg);
	time_t max_long_spin	= ooos::get_element<2>(amd64_ahci::__cfg);
	if(__unlikely(!await_completion(max_spin, std::bind(__not_data_busy, std::addressof(__port))))) __module.raise_error("hung port", 6);
	barrier();
	__port.cmd_issue		|= BIT(slot);
	barrier();
	dword st;
	const char* msg;
	int status				= 0;
	for(time_t i = 0; i < max_long_spin && __port_data_busy(std::addressof(__port)); i++)
	{
		barrier();
		st					= __port.i_state;
		if(__unlikely(st.hi.hi & i_state_hi_byte_error))
		{
			if(last_msg_str) __module.release_buffer(addr_t(last_msg_str), alignof(char));
			if(__module.asprintf(std::addressof(msg), "port number %i i/o error", __idx)) last_msg_str = msg;
			else msg		= "AHCI i/o error";
			status			= 7;
			break;
		}
		barrier();
	}
	if(__unlikely(status != 0 || __port_data_busy(std::addressof(__port)))) __module.raise_error(msg ? msg : "AHCI hardware error", status ? status : 8);
}
void ahci_port::__build_h2d_io_fis(uintptr_t dest_start, size_t sector_count, addr_t buffer, ata_command action, hba_cmd_header& cmd)
{
	__builtin_memset(cmd.command_table, 0, command_table_size());
	size_t bpe_max					= bytes_per_prdt_entry();
	bool enable_ioc					= !ooos::get_element<3>(amd64_ahci::__cfg);
	qword addr						= buffer.full;
	size_t final_count				= (sector_count % (bpe_max / block_size())) * block_size();
	barrier();
	for(uint16_t i = 0; i < cmd.prdt_length; i++, addr += bpe_max, barrier())
	{
		new(std::addressof(cmd.command_table->prdt_entries[i])) hba_prdt_entry
		{
			.data_base					= addr.lo,
			.data_base_hi				= addr.hi,
			.byte_count					= static_cast<uint32_t>((i == cmd.prdt_length - 1 ? final_count : bpe_max) - 1),
			.interrupt_on_completion	= enable_ioc,
		};
	}
	qword start(dest_start);
	new(static_cast<void*>(cmd.command_table->cmd_fis)) fis_reg_h2d
	{
		.type		= reg_h2d,
		.ctype		= true,
		.command	= action,
		.lba0		= start.lo.lo.lo,
		.lba1		= start.lo.lo.hi,
		.lba2		= start.lo.hi.lo,
		.device		= 0x40UC,
		.lba3		= start.lo.hi.hi,
		.lba4		= start.hi.lo.lo,
		.lba5		= start.hi.lo.hi,
		.count		= dword(static_cast<uint32_t>(sector_count)).lo,
	};
}
bool ahci_port::io_busy() const
{
	barrier();
	uint32_t c		= __port.cmd;
	barrier();
	return __port_data_busy(std::addressof(__port)) || (c & hba_command_cr) == 0;
}
ahci_port::stticket ahci_port::read(void* dest, uintptr_t src_start, size_t sector_count)
{
	if(__unlikely(io_busy())) return -1;
	stticket slot = __find_cmd_slot();
	if(__unlikely(slot < 0)) return slot;
	barrier();
	hba_cmd_header* cmd	= new(std::addressof(__port.command_list[slot])) hba_cmd_header
	{
		.cmd_fis_len	= static_cast<uint8_t>(sizeof(fis_reg_h2d) / sizeof(uint32_t)),
		.atapi			= false,
		.w_direction	= false, // d2h write
		.prdt_length	= static_cast<uint16_t>(__cap_size(div_round_up(sector_count, sectors_per_prdt_entry()), __prdt_count)),
		.command_table	= __port.command_list[slot].command_table
	};
	barrier();
	__build_h2d_io_fis(src_start, sector_count, dest, ata_command::read_dma_ext, *cmd);
	utticket uslot	= static_cast<utticket>(slot);
	__cmd_issue(uslot);
	await_completion(ooos::get_element<1>(amd64_ahci::__cfg), std::bind(&ahci_port::io_complete, this, uslot));
	return slot;
}
ahci_port::stticket ahci_port::write(uintptr_t dest_start, const void* src, size_t sector_count)
{
	if(__unlikely(io_busy())) return -1;
	stticket slot		= __find_cmd_slot();
	if(__unlikely(slot < 0)) return slot;
	barrier();
	hba_cmd_header* cmd	= new(std::addressof(__port.command_list[slot])) hba_cmd_header
	{
		.cmd_fis_len	= static_cast<uint8_t>(sizeof(fis_reg_h2d) / sizeof(uint32_t)),
		.atapi			= false,
		.w_direction	= true, // h2d write
		.prdt_length	= static_cast<uint16_t>(__cap_size(div_round_up(sector_count, sectors_per_prdt_entry()), __prdt_count)),
		.command_table	= __port.command_list[slot].command_table
	};
	barrier();
	__build_h2d_io_fis(dest_start, sector_count, src, ata_command::write_dma_ext, *cmd);
	utticket uslot		= static_cast<utticket>(slot);
	__cmd_issue(uslot);
	await_completion(ooos::get_element<1>(amd64_ahci::__cfg), std::bind(&ahci_port::io_complete, this, uslot));
	return slot;
}
void ahci_port::identify(identify_data* out)
{
	stticket slot		= __find_cmd_slot();
	if(__unlikely(slot < 0)) {
		__module.raise_error("no slots", 9);
		__builtin_unreachable();
	}
	qword addr			= reinterpret_cast<uintptr_t>(out);
	barrier();
	hba_cmd_header* cmd	= new(std::addressof(__port.command_list[slot])) hba_cmd_header
	{
		.cmd_fis_len	= 5UC,
		.w_direction	= false,
		.prdt_length	= 1US,
		.command_table	= new(__port.command_list[slot].command_table) hba_cmd_table{}
	};
	barrier();
	new(std::addressof(__port.command_list[slot].command_table->prdt_entries[0])) hba_prdt_entry
	{
		.data_base					= addr.lo,
		.data_base_hi				= addr.hi,
		.byte_count					= 511U,
		.interrupt_on_completion	= !ooos::get_element<3>(amd64_ahci::__cfg),
	};
	barrier();
	new(static_cast<void*>(cmd->command_table->cmd_fis)) fis_reg_h2d
	{
		.type		= reg_h2d,
		.ctype		= true,
		.command	= ata_command::identify,
		.device		= 0UC
	};
	utticket uslot	= static_cast<utticket>(slot);
	__cmd_issue(uslot);
	if(!await_completion(ooos::get_element<1>(amd64_ahci::__cfg), std::bind(&ahci_port::io_complete, this, uslot))) {
		__module.raise_error("hung port", 10);
		__builtin_unreachable();
	}
}
template<ooos::condition_callable FT>
static inline bool await_completion(time_t max_spin, FT&& ft)
{
	for(time_t spin = 0UZ; spin < max_spin; spin++)
		if(ft())
			return true;
	return ft();
}
static inline ahci_device check_type(hba_port const& p)
{
	barrier();
	if((dword(p.s_status).lo.lo & 0x0FUC) != port_present || (dword(p.s_status).lo.hi & 0x0FUC) != port_active) return none;
	barrier();
	switch(p.sig)
	{
		case sig_atapi: return atapi;
		case sig_semb:  return semb;
		case sig_pmul:  return pmul;
		case sig_sata:  return sata;
		default:        return none;
	}
}
EXPORT_MODULE(amd64_ahci);