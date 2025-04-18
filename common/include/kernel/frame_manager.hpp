#ifndef __FRAME_MGR
#define __FRAME_MGR
#include "kernel/kernel_mm.hpp"
#include "set"
class frame_manager : std::set<uframe_tag>
{
    typedef std::set<uframe_tag> __base;
    static frame_manager __instance;
    constexpr frame_manager() = default;
public:
    using __base::iterator;
    using __base::const_iterator;
    uframe_tag& create_frame(addr_t start_base, addr_t start_extent);
    void destroy_frame(uframe_tag& ft);
    static frame_manager& get();
};
#define fm frame_manager::get()
#endif