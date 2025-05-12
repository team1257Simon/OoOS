#include "entropy_source.hpp"
entropy_source::entropy_source() : __base(max_accumulation) {}
void entropy_source::on_modify_queue() { __trim_stale(); sync(); }
__isrcall void entropy_source::accumulate()
{
    if(__qsize() < max_accumulation)
    {
        size_t sz = __qsize() + accumulation_rate;
        size_t cap = __qcapacity();
        if(cap >= sz || __q_grow_buffer(sz - cap))
        {    
            __qsete(static_cast<uint8_t*>(__fill_entropy(__qcur(), accumulation_rate / sizeof(uint64_t))));
            __trim_stale();
            sync();
        }
    }
}