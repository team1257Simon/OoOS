#ifndef __ISR_TABLE
#define __ISR_TABLE
#include "stdint.h"
#include "functional"
#include "kernel/libk_decls.h"
#include "vector"
using irq_handler = std::function<void(uint8_t)>;
class irq_table
{
    spinlock_t __my_mutex{};
    std::vector<irq_handler> __handler_tables[16]{};
    constexpr std::size_t __how_many(uint8_t idx) const noexcept { return __handler_tables[idx].size(); }
public:
    constexpr irq_table() noexcept = default;
    constexpr void add_handler(uint8_t idx, irq_handler const& handler) { if(idx >= 16) return; while(acquire(&__my_mutex)); __handler_tables[idx].push_back(handler); release(&__my_mutex); }
    constexpr void add_multi(irq_handler const& handler, uint8_t min, uint8_t max_excl) { for(int i = min; i < max_excl && i < 16; i++) add_handler(i, handler); }
    constexpr void dispatch(uint8_t irq) const { if(irq < 16) for(std::size_t i = 0; i < __how_many(irq); i++) __handler_tables[irq][i](irq); }
    static irq_table& get_instance() noexcept;
};
#endif