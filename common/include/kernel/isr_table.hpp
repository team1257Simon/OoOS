#ifndef __ISR_TABLE
#define __ISR_TABLE
#include <stdint.h>
#include <libk_decls.h>
#include <kernel_api.hpp>
#include <functional>
#include <vector>
typedef std::ext::nothrow_function<> irq_callback;
typedef std::ext::nothrow_function<u8, qword> interrupt_callback;
struct msi_info
{
	uintptr_t msg_addr_value;
	void* owner;
	uint16_t msg_data_value;
	ooos::isr_actor callback;
};
namespace interrupt_table
{
	__nointerrupts bool add_irq_handler(u8 idx, irq_callback&& handler) noexcept;
	__nointerrupts bool add_irq_handler(void* owner, u8 idx, ooos::isr_actor&& handler) noexcept;
	__nointerrupts void deregister_owner(void* owner) noexcept;
	__nointerrupts void add_interrupt_callback(interrupt_callback&& cb) noexcept;
	__nointerrupts msi_info* allocate_msi_vector(void* owner, ooos::isr_actor&& handler, qword current_msi_addr, word current_msi_data, msi_trigger_mode mode) noexcept;
	__nointerrupts msi_info* allocate_msi_vectors(void* owner, uint8_t count, qword current_msi_addr_field, word current_msi_data_field, msi_trigger_mode mode) noexcept;
}
#endif