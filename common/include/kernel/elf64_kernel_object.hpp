#ifndef __ELF_KO
#define __ELF_KO
#include "elf64_dynamic.hpp"
#include "module.hpp"
class elf64_kernel_object : public elf64_dynamic_object
{
protected:
    addr_t load_base;
    std::align_val_t load_align;
    addr_t entry;
    ooos_kernel_module::abstract_module_base* module_object;
    virtual void process_headers() override;
    virtual addr_t translate_in_frame(addr_t addr) override;
    virtual void set_frame(uframe_tag* ft) override;
    virtual uframe_tag* get_frame() const override;
    virtual void frame_enter() override;
    virtual bool load_segments() override;
    virtual bool load_syms() override;
    virtual bool xvalidate() override;
    virtual void on_load_failed() override;
public:
    elf64_kernel_object(file_node* file);
    virtual ~elf64_kernel_object();
    ooos_kernel_module::abstract_module_base* load_module();
};
#endif