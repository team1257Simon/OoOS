#include "sched/task_queue.hpp"
#include "kernel/libk_decls.h"
#include "arch/arch_amd64.h"
void init_pit() { outb(port_pit_cmd, pit_mode); outb(port_pit_data, pit_divisor.lo); outb(port_pit_data, pit_divisor.hi); }
void task_pl_queue::on_skipped() noexcept { if(!this->empty() && !this->at_end()) { this->next()->task_ctl.skips++; } }
bool task_pl_queue::skip_flag() noexcept { return !this->empty() && !this->at_end() && this->next()->task_ctl.skips > __skips_threshold; }
bool task_pl_queue::transfer_next(task_ptr_queue_base &to_whom) { return !this->empty() && !this->at_end() && this->transfer(to_whom) != 0; }
void task_wait_queue::tick_wait() noexcept { if(!this->empty() && !this->at_end()) { this->next()->task_ctl.wait_ticks_delta--; } }
unsigned int task_wait_queue::next_remaining_wait_ticks() const noexcept { return !this->empty() && !this->at_end() ? this->next()->task_ctl.wait_ticks_delta : 0u; }
bool task_wait_queue::interrupt_wait(const_iterator where) { if(!(where < this->end() && (*where)->task_ctl.can_interrupt)) return false; if(const_iterator subs = where + 1; subs < this->end()) (*subs)->task_ctl.wait_ticks_delta += (*where)->task_ctl.wait_ticks_delta; return this->erase(where) != 0; }
task_pl_queue &user_level_task_queues::operator[](priority_val pv) noexcept { return this->__base::operator[](__idx_by_prio(pv)); }
task_pl_queue const &user_level_task_queues::operator[](priority_val pv) const noexcept { return this->__base::operator[](__idx_by_prio(pv)); }