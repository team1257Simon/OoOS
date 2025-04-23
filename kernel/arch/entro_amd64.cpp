#include "arch/entro_amd64.hpp"
entro_amd64::entro_amd64() : __base(max_accumulation) {}
void entro_amd64::on_modify_queue() { this->__trim_stale(); this->sync(); }
__isrcall void entro_amd64::accumulate()
{
    if(this->__qsize() < max_accumulation)
    {
        if(this->reserve_at_least(this->__qsize() + accumulation_rate))
        {    
            this->__qsete(static_cast<uint8_t*>(__fill_entropy(this->__qcur(), accumulation_rate / sizeof(uint64_t))));
            this->__trim_stale();
            this->sync();
        }
    }
}