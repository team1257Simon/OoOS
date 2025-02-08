#include "sched/task_queue.hpp"
#include "kernel/libk_decls.h"
#include "arch/arch_amd64.h"
extern "C" tss system_tss;
extern "C" void init_pit() { outb(port_pit_cmd, pit_mode); outb(port_pit_data, pit_divisor.lo); outb(port_pit_data, pit_divisor.hi); }
extern "C" void init_tss(vaddr_t k_rsp) { system_tss.rsp[0] = k_rsp; system_tss.rsp[1] = k_rsp; system_tss.rsp[2] = k_rsp; for(int i = 0; i < 7; i++) { system_tss.ist[i] = k_rsp; } }
__isrcall void task_pl_queue::on_skipped() noexcept { if(!this->empty() && !this->at_end()) { this->next()->task_ctl.skips++; } }
__isrcall bool task_pl_queue::skip_flag() noexcept { return !this->empty() && !this->at_end() && this->next()->task_ctl.skips > __skips_threshold; }
__isrcall bool task_pl_queue::transfer_next(task_ptr_queue_base &to_whom) { return !this->empty() && !this->at_end() && this->transfer(to_whom) != 0; }
__isrcall void task_wait_queue::tick_wait() noexcept { if(!this->empty() && !this->at_end()) { this->next()->task_ctl.wait_ticks_delta--; } }
__isrcall unsigned int task_wait_queue::next_remaining_wait_ticks() const noexcept { return !this->empty() && !this->at_end() ? this->next()->task_ctl.wait_ticks_delta : 0u; }
bool task_wait_queue::interrupt_wait(const_iterator where) { if(!(where < this->end() && (*where)->task_ctl.can_interrupt)) return false; if(const_iterator subs = where + 1; subs < this->end()) (*subs)->task_ctl.wait_ticks_delta += (*where)->task_ctl.wait_ticks_delta; (*where)->task_ctl.block = false; return this->erase(where) != 0; }
__isrcall unsigned int task_wait_queue::cumulative_remaining_ticks() const noexcept { unsigned int result = 0; for(const_iterator i = current(); i != end(); i++) result += (*i)->task_ctl.wait_ticks_delta; return result; }
__isrcall task_pl_queue &prio_level_task_queues::operator[](priority_val pv) noexcept { return this->__base::operator[](__idx_by_prio(pv)); }
__isrcall task_pl_queue const &prio_level_task_queues::operator[](priority_val pv) const noexcept { return this->__base::operator[](__idx_by_prio(pv)); }