#ifndef __EXECUTABLE
#define __EXECUTABLE
#include "kernel/elf64_object.hpp"
#include "kernel/kernel_mm.hpp"
class elf64_executable : public virtual elf64_object
{
protected:
	size_t stack_size;
	addr_t frame_base;
	addr_t frame_extent;
	addr_t stack_base;
	addr_t entry;
	uframe_tag* frame_tag;
	elf64_program_descriptor program_descriptor;
	virtual void process_headers() override;
	virtual bool load_segments() override;
	virtual bool xvalidate() override;
	virtual void on_load_failed() override;
	virtual addr_t segment_vaddr(size_t n) const;
	virtual void frame_enter() override;
	virtual void set_frame(uframe_tag* ft) override;
	virtual uframe_tag* get_frame() const override;
public:
	virtual ~elf64_executable();
	elf64_executable(file_vnode* n, size_t stack_sz = S04);
	elf64_executable(addr_t start, size_t size, size_t stack_sz = S04);
	elf64_executable(elf64_executable const& that);
	elf64_executable(elf64_executable&& that);
	elf64_program_descriptor const& describe() const noexcept;
};
#endif