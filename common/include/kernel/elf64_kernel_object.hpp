#ifndef __ELF_KO
#define __ELF_KO
#include <elf64_dynamic.hpp>
#include <module.hpp>
class elf64_kernel_object : public elf64_dynamic_object
{
protected:
	addr_t load_base{};
	std::align_val_t load_align{};
	addr_t entry{};
	ooos::abstract_module_base* module_object{};
	virtual void process_headers() override;
	virtual addr_t translate_in_frame(addr_t addr) override;
	virtual void set_frame(uframe_tag* ft) override;
	virtual uframe_tag* get_frame() const override;
	virtual void frame_enter() override;
	virtual bool load_segments() override;
	virtual bool load_syms() override;
	virtual bool xvalidate() override;
	virtual void on_load_failed() override;
	virtual bool is_position_relocated() const noexcept override;
public:
	constexpr ooos::abstract_module_base* get_module() noexcept { return module_object; }
	elf64_kernel_object(file_vnode* file);
	elf64_kernel_object(addr_t start, size_t size);
	elf64_kernel_object(elf64_kernel_object&& that);
	elf64_kernel_object(elf64_kernel_object const&) = delete;
	virtual ~elf64_kernel_object();
	virtual addr_t resolve(uint64_t offs) const override;
	ooos::abstract_module_base* load_module();
	void unload_pre_init();		// called if the module needs to be unloaded before the initialize() function is invoked on it
};
#endif