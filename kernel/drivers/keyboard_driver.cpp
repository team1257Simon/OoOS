#include "keyboard_driver.hpp"
void keyboard_driver::add_listener(kb_listener&& kl) { __listeners.push_back(std::move(kl)); }
void keyboard_driver::clear_listeners() { __listeners.clear(); }
void keyboard_driver::initialize() { __on_init(); interrupt_table::add_irq_handler(__get_irq_index(), std::move([this]() -> void { if(__skip_send()) return; kb_data dat = __get_last(__current_state); __current_state = dat.state; for(kb_listener const& l : __listeners) { l(dat); } })); }
bool keyboard_driver::__skip_send() { return false; }