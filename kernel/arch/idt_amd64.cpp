#include <libk_decls.h>
#include <arch/idt_amd64.h>
#include <arch/apic.hpp>
#include <util/bitmap.hpp>
#include <unordered_map>
#include <isr_table.hpp>
#include <kernel_mm.hpp>
#include <array>
#include <string>
#include <stdlib.h>
std::array<std::unordered_map<void*, ooos::isr_actor>, 16UZ> __managed_handlers{};
std::array<std::vector<irq_callback>, 16UZ> __handler_tables{};
std::vector<interrupt_callback> __registered_callbacks{};
std::array<msi_info, 256UZ - 48UZ> __msi_handlers{};
static uint64_t msi_vec_alloc_bitmap[4] { 0xFFFFFFFFFFFFUL, 0UL, 0UL, 0UL };
extern volatile apic bsp_lapic;
struct __pack msi_addr_reg
{
	bool				: 2;
	bool dst_mode		: 1;
	bool redir_hint		: 1;
	bool				: 8;
	uint8_t dst_id		: 8;
	uint16_t hi_bits	: 12;
};
enum class msi_delivery_mode : uint8_t
{
	FIXED	= 0UC,
	LOWEST	= 1UC,
	SMI		= 2UC,
	NMI		= 4UC,
	INIT	= 5UC,
	EXTERN	= 7UC,
};
struct __pack msi_data_reg
{
	uint8_t vec;
	msi_delivery_mode delivery_mode	: 3;
	bool							: 3;
	msi_trigger_mode trigger_mode	: 2;
};
namespace interrupt_table
{
    spinlock_t __itable_mutex;
	uint8_t irq_range			= 0x30UC;
    void __lock() { lock(std::addressof(__itable_mutex)); }
    void __unlock() { release(std::addressof(__itable_mutex)); }
	void deregister_owner(void* owner) noexcept
	{
		if(owner)
		{
			__lock();
			for(std::unordered_map<void*, ooos::isr_actor>& m : __managed_handlers) m.erase(owner);
			for(msi_info& msi : __msi_handlers)
			{
				if(msi.owner == owner)
				{
					uint8_t vec		= dword(msi.msg_data_value).lo.lo;
					msi.callback	= nullptr;
					msi.owner		= nullptr;
					bitmap_clear_bit(msi_vec_alloc_bitmap, vec);
				}
			}
			__unlock();
		}
	}
	bool add_irq_handler(void* owner, u8 idx, ooos::isr_actor&& handler) noexcept
	{
		if(__unlikely(!handler)) { klog("[ISR] W: bad callback functor"); }
		else try
		{
			if(idx < 16UC && owner)
			{
				__lock();
				bool result = __managed_handlers[idx].emplace(owner, std::forward<ooos::isr_actor>(handler)).second;
				__unlock();
				return result;
			}
			return false;
		}
		catch(...) {
			panic("out of memory");
			abort();
		}
		return false;
	}
	bool add_irq_handler(u8 idx, irq_callback&& handler) noexcept
	{
		if(__unlikely(!handler)) { klog("[ISR] W: bad callback functor"); }
		else try
		{
			if(idx < 16UC)
			{
				__lock();
				__handler_tables[idx].push_back(std::move(handler));
				__unlock();
				return __handler_tables[idx].size() == 1;
			}
			return false;
		}
		catch(...) {
			panic("out of memory");
			abort();
		}
		return false;
	}
	void add_interrupt_callback(interrupt_callback&& cb) noexcept
	{
		if(__unlikely(!cb)) { klog("[ISR] W: bad callback functor"); }
		else try { __registered_callbacks.push_back(std::move(cb)); }
		catch(...) {
			panic("out of memory");
			abort();
		}
	}
	msi_info* allocate_msi_vector(void* owner, ooos::isr_actor&& handler, qword current_msi_addr_field, word current_msi_data_field, msi_trigger_mode mode) noexcept
	{
		off_t ivec			= bitmap_scan_single_zero(msi_vec_alloc_bitmap, 4UZ);
		if(__unlikely(ivec < 0Z)) return nullptr;
		__lock();
		bitmap_set_bit(msi_vec_alloc_bitmap, ivec);
		uint8_t vec			= static_cast<uint8_t>(ivec);
		msi_addr_reg addr	= std::bit_cast<msi_addr_reg>(current_msi_addr_field.lo);
		msi_data_reg data	= std::bit_cast<msi_data_reg>(current_msi_data_field);
		addr.hi_bits		= 0xFFEUS;
		data.delivery_mode	= msi_delivery_mode::FIXED;
		data.trigger_mode	= mode;
		data.vec			= vec;
		msi_info* result	= new(std::addressof(__msi_handlers[vec - 0x30UC])) msi_info
		{
			.msg_addr_value	= qword(std::bit_cast<uint32_t>(addr), 0U),
			.owner			= owner,
			.msg_data_value	= std::bit_cast<uint16_t>(data),
			.callback		= std::move(handler)
		};
		__unlock();
		return result;
	}
	msi_info* allocate_msi_vectors(void* owner, uint8_t count, qword current_msi_addr_field, word current_msi_data_field, msi_trigger_mode mode) noexcept
	{
		off_t ivec			= bitmap_scan_chain_zeroes(msi_vec_alloc_bitmap, 4UZ, count);
		if(__unlikely(ivec < 0Z)) return nullptr;
		__lock();
		bitmap_set_chain_bits(msi_vec_alloc_bitmap, ivec, count);
		msi_addr_reg addr	= std::bit_cast<msi_addr_reg>(current_msi_addr_field.lo);
		msi_data_reg data	= std::bit_cast<msi_data_reg>(current_msi_data_field);
		addr.hi_bits		= 0xFFEUS;
		data.delivery_mode	= msi_delivery_mode::FIXED;
		data.trigger_mode	= mode;
		uint8_t vec			= static_cast<uint8_t>(ivec);
		uint8_t pos			= vec - 0x30UC;
		for(uint8_t i		= 0UC; i < count; i++)
		{
			data.vec		= vec + i;
			new(std::addressof(__msi_handlers[pos + i])) msi_info
			{
				.msg_addr_value	= qword(std::bit_cast<uint32_t>(addr), 0U),
				.owner			= owner,
				.msg_data_value	= std::bit_cast<uint16_t>(data)
			};
		}
		__unlock();
		return std::addressof(__msi_handlers[pos]);
	}
}
inline void pic_eoi(u8 irq)
{
    if(bsp_lapic.valid()) return bsp_lapic.eoi();
    if(irq > 7) outb(command_pic2, sig_pic_eoi);
    outb(command_pic1, sig_pic_eoi);
}
extern "C"
{
    extern uint64_t ecode;
    extern void* const isr_table[256];
    extern idt_entry_t idt_table[256];
    extern void no_waiting_op();
    extern void (*callback_8)();
    extern volatile bool delay_flag;
    struct {
        uint16_t size;
        void* idt_ptr;
    } __pack idt_descriptor{};
    extern void idt_register();
    constexpr static void idt_set_descriptor(uint8_t vector, addr_t isr);
    void exception_handler(int exception_number, void* exception_address) { idt_set_descriptor(static_cast<uint8_t>(exception_number), exception_address); }
    constexpr static void idt_set_descriptor(uint8_t vector, addr_t isr)
    {
        new(std::addressof(idt_table[vector])) idt_entry_t
        {
            .isr_low	{ static_cast<uint16_t>(isr.full & 0xFFFFUS) },
            .kernel_cs	{ 0x8US },
            .ist		{ static_cast<uint8_t>((vector < 0x30UC) ? 1 : 0) },
            .attributes	{ 0xEEUC },
            .isr_mid	{ static_cast<uint16_t>((isr.full >> 16) & 0xFFFFUS) },
            .isr_high	{ static_cast<uint32_t>((isr.full >> 32) & 0xFFFFFFFFU) },
			.reserved	{ 0U }
        };
    }
    void isr_dispatch(uint8_t idx)
    {
        bool is_err			= (idx == 0x08UC || (idx > 0x09UC && idx < 0x0FUC) || idx == 0x11UC || idx == 0x15UC || idx == 0x1DUC || idx == 0x1EUC);
        if(idx > 0x19UC && idx < 0x30UC)
        {
            u8 irq		= static_cast<uint8_t>(idx - 0x20UC);
            kernel_memory_mgr::suspend_user_frame();
            kfx_save();
        	if(irq == 8UC)
			{
				(*callback_8)();
				delay_flag	= false;
				callback_8	= no_waiting_op;
			}
            for(std::pair<void* const, ooos::isr_actor>& p : __managed_handlers[irq]) p.second();
            for(irq_callback const& h : __handler_tables[irq]) h();
            pic_eoi(irq);
            kfx_load();
            kernel_memory_mgr::resume_user_frame();
        }
		else if(idx >= 0x30UC && __msi_handlers[idx - 0x30UC].callback)
		{
            kernel_memory_mgr::suspend_user_frame();
            kfx_save();
			__msi_handlers[idx - 0x30UC].callback();
            kfx_load();
            kernel_memory_mgr::resume_user_frame();
		}
        else
        {
            kernel_memory_mgr::suspend_user_frame();
            kfx_save();
            for(interrupt_callback const& c : __registered_callbacks) { if(c) c(idx, is_err ? ecode : 0); }
            kfx_load();
            kernel_memory_mgr::resume_user_frame();
        }
    }
    void idt_init()
    {
        pic_remap<0x20UC, 0x28UC>();
        for(int i = 0; i < 256; i++) idt_set_descriptor(i, isr_table[i]);
        idt_descriptor.size		= 4095US;
        idt_descriptor.idt_ptr	= idt_table;
        idt_register();
    }
}