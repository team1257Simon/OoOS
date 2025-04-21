#include "elf64_exec.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
static std::allocator<program_segment_descriptor> sd_alloc{};
use_kmm(kmm);
elf64_program_descriptor const& elf64_executable::describe() const noexcept { return program_descriptor; }
void elf64_executable::frame_enter() { kmm.enter_frame(frame_tag); }
addr_t elf64_executable::segment_vaddr(size_t n) const { return addr_t(phdr(n).p_vaddr); }
elf64_executable::~elf64_executable() = default; // the resources allocated for the executable's segments are freed and returned to the kernel when the frame is destroyed
void elf64_executable::on_load_failed() { fm.destroy_frame(*frame_tag); frame_tag = nullptr; kmm.exit_frame(); }
void elf64_executable::set_frame(uframe_tag* ft) { ft->mapped_max = frame_tag->mapped_max; frame_tag = ft; program_descriptor.frame_ptr = ft; }
uframe_tag *elf64_executable::get_frame() const { return frame_tag; }
elf64_executable::elf64_executable(addr_t start, size_t size, size_t stack_sz, size_t tls_sz) :
    elf64_object        ( start, size ),
    stack_size          { stack_sz },
    tls_size            { tls_sz },
    frame_base          { nullptr },
    frame_extent        { nullptr },
    stack_base          { nullptr },
    tls_base            { nullptr },
    entry               ( ehdr().e_entry ),
    frame_tag           { nullptr },
    program_descriptor  {}
                        {}
elf64_executable::elf64_executable(file_node* n, size_t stack_sz, size_t tls_sz) : 
    elf64_object        { n },
    stack_size          { stack_sz },
    tls_size            { tls_sz },
    frame_base          { nullptr },
    frame_extent        { nullptr },
    stack_base          { nullptr },
    tls_base            { nullptr },
    entry               ( ehdr().e_entry ),
    frame_tag           { nullptr },
    program_descriptor  {}
                        {}
elf64_executable::elf64_executable(elf64_executable&& that) :
    elf64_object        ( std::move(that) ),
    stack_size          { that.stack_size },
    tls_size            { that.tls_size },
    frame_base          { std::move(that.frame_base) },
    frame_extent        { std::move(that.frame_extent) },
    stack_base          { std::move(that.stack_base) },
    tls_base            { std::move(that.tls_base)},
    entry               { std::move(that.entry) },
    frame_tag           { that.frame_tag },
    program_descriptor  { std::move(that.program_descriptor) }
    {
        that.stack_size = 0;
        that.tls_size = 0;
        that.frame_base = nullptr;
        that.frame_extent = nullptr;
        that.stack_base = nullptr;
        that.tls_base = nullptr;
        that.entry = nullptr;
        that.frame_tag = nullptr;
        new(std::addressof(that.program_descriptor)) elf64_program_descriptor
        {
            .frame_ptr = nullptr, 
            .prg_stack = nullptr, 
            .stack_size = 0,
            .prg_tls = nullptr, 
            .tls_size = 0, 
            .entry = nullptr,
            .ld_path = nullptr,
            .ld_path_count = 0,
            .object_handle = nullptr
        };
        program_descriptor.object_handle = this;
    }
elf64_executable::elf64_executable(elf64_executable const& that) :
    elf64_object        ( that ),
    stack_size          { that.stack_size },
    tls_size            { that.tls_size },
    frame_base          { that.frame_base },
    frame_extent        { that.frame_extent },
    stack_base          { that.stack_base },
    tls_base            { that.tls_base },
    entry               { that.entry },
    frame_tag           { that.frame_tag },
    program_descriptor  { that.program_descriptor }
    {
        program_descriptor.object_handle = this;
        // on_copy(std::addressof(fm.create_frame(frame_base, frame_extent)));
        // frame_tag->mapped_max = that.frame_tag->mapped_max;
    }
bool elf64_executable::xvalidate()
{
    if(ehdr().e_machine != EM_AMD64 || ehdr().e_ident[elf_ident_enc_idx] != ED_LSB) { panic("not an object for the correct machine"); return false; }
    if(ehdr().e_ident[elf_ident_class_idx] != EC_64 ) { panic("32-bit executables are not yet supported"); return false; }
    if(ehdr().e_type != ET_EXEC) { panic("object is not an executable"); return false; }
    if(!ehdr().e_phnum) { panic("no program headers present"); return false; }
    return true;
}
void elf64_executable::process_headers()
{
    elf64_object::process_headers();
    for(size_t n = 0; n < ehdr().e_phnum; n++)
    {
        elf64_phdr const& h = phdr(n);
        if(!is_load(h)) continue;
        if(!frame_base || frame_base > h.p_vaddr) frame_base = addr_t(h.p_vaddr);
        if(!frame_extent || h.p_vaddr + h.p_memsz > frame_extent) frame_extent = addr_t(h.p_vaddr + h.p_memsz);
    }
    frame_base = frame_base.page_aligned();
    frame_extent = frame_extent.next_page_aligned();
    stack_base = frame_extent;
    frame_extent += stack_size;
    tls_base = frame_extent;
    frame_extent += tls_size;
}
bool elf64_executable::load_segments()
{
    if((frame_tag = std::addressof(fm.create_frame(frame_base, frame_extent))))
    {
        frame_enter();
        size_t i = 0;
        for(size_t n = 0; n < ehdr().e_phnum; n++)
        {
            elf64_phdr const& h = phdr(n);
            if(!is_load(h) || !h.p_memsz) continue;
            addr_t addr = segment_vaddr(n);
            addr_t target = addr.trunc(h.p_align);
            size_t full_size = h.p_memsz + (addr - target);
            addr_t img_dat = segment_ptr(n);
            addr_t blk = kmm.allocate_user_block(full_size, target, h.p_align, is_write(h), is_exec(h));
            addr_t idmap(kmm.frame_translate(addr));
            kmm.exit_frame();
            if(!blk) { panic("could not allocate blocks for executable"); return false; }
            frame_tag->usr_blocks.emplace_back(blk, target, kernel_memory_mgr::aligned_size(target, full_size), h.p_align, is_write(h), is_exec(h));
            frame_enter();
            array_copy<uint8_t>(idmap, img_dat, h.p_filesz);
            if(h.p_memsz > h.p_filesz) { array_zero<uint8_t>(idmap.plus(h.p_filesz), static_cast<size_t>(h.p_memsz - h.p_filesz)); }
            new(std::addressof(segments[i++])) program_segment_descriptor
            { 
                .absolute_addr = idmap, 
                .virtual_addr  = addr, 
                .obj_offset    = static_cast<off_t>(h.p_offset),
                .size		   = h.p_memsz, 
                .seg_align     = h.p_align, 
                .perms         = static_cast<elf_segment_prot>(0b100 | (is_write(h) ? 0b010 : 0) | (is_exec(h) ? 0b001 : 0)) 
            };
        }
        addr_t stkblk = kmm.allocate_user_block(stack_size, stack_base, page_size, true, false);
        addr_t tlsblk = kmm.allocate_user_block(tls_size, tls_base, page_size, true, false);
        if(!stkblk || !tlsblk) { panic("could not allocate blocks for stack");  return false; }
        kmm.exit_frame();
        frame_tag->usr_blocks.emplace_back(stkblk, stack_base, stack_size, page_size, true, false);
        frame_tag->usr_blocks.emplace_back(tlsblk, tls_base, tls_size, page_size, true, false);
        new(std::addressof(program_descriptor)) elf64_program_descriptor
        { 
            .frame_ptr = frame_tag, 
            .prg_stack = stack_base, 
            .stack_size = stack_size,
            .prg_tls = tls_base, 
            .tls_size = tls_size,
            .entry = entry,
            .ld_path = nullptr,
            .ld_path_count = 0,
            .object_handle = this
        };
        return true;
    }
    else { panic("could not allocate frame"); return false; }
}