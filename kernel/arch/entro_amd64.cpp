#include "arch/entro_amd64.hpp"
entro_amd64::entro_amd64() : __base(max_accumulation) {}
void entro_amd64::on_modify_queue() { __trim_stale(); sync(); }
__isrcall void entro_amd64::accumulate()
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