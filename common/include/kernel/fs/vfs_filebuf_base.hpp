#ifndef __VFS_FILEBUF_BASE
#define __VFS_FILEBUF_BASE
#include "fs/data_buffer.hpp"
constexpr std::streamsize base_sector_size = 512;
template<std::char_type CT, std::char_traits_type<CT> TT = std::char_traits<CT>>
class vfs_filebuf_base : public data_buffer<CT, TT>
{
    using __buffer_base = data_buffer<CT, TT>;
public:
    using typename std::basic_streambuf<CT, TT>::char_type;
    using typename std::basic_streambuf<CT, TT>::traits_type;
    using typename std::basic_streambuf<CT, TT>::pos_type;
    using typename std::basic_streambuf<CT, TT>::off_type;
    using typename std::basic_streambuf<CT, TT>::int_type;
protected:
    using typename std::basic_streambuf<CT, TT>::__sb_type;
    using typename std::basic_streambuf<CT, TT>::__ptr_container;
    bool __dirty { false };
    virtual int __ddwrite() = 0; // Sync the put region to the file if this stream is output-buffered.
    virtual std::streamsize __ddread(std::streamsize) = 0; // Get more bytes from the file if there are any left.
    virtual std::streamsize __ddrem() = 0; // How many unread bytes are in the file's data sequence.
    virtual std::streamsize __sect_size() { return base_sector_size; }
    virtual std::streamsize showmanyc() override { return __ddrem(); }
    virtual void __on_modify() override { if(this->__beg()) { this->__fullsetp(this->__beg(), this->__cur(), this->__max()); __dirty = true; } }
    virtual int sync() override { __on_modify(); if(__dirty) { int result = __ddwrite(); __dirty = (result != 0); return result; } return 0; }
    virtual int_type underflow() override { std::streamsize n = std::min(__sect_size(), showmanyc()); if(n && __ddread(n)) { return traits_type::to_int_type(*this->gptr()); } return traits_type::eof(); }
public:
    vfs_filebuf_base(std::streamsize init_buffer_size = base_sector_size) : __buffer_base{ init_buffer_size } {}
    vfs_filebuf_base(vfs_filebuf_base&& that) : __buffer_base(std::forward<__buffer_base>(that)) {}
    vfs_filebuf_base& operator=(vfs_filebuf_base&& that) { this->__realloc_move(std::forward<__buffer_base>(that)); return *this; }
};
#endif