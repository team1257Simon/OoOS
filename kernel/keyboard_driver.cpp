#include "keyboard_driver.hpp"
void keyboard_driver_base::__on_interrupt() { if(__skip_send()) return; kb_data dat = this->__get_last(__current_state); this->__current_state = dat.state; for(kb_listener l : __listeners) { l(dat); } }
void keyboard_driver_base::add_listener(kb_listener &&kl) { __listeners.push_back(kl); }
void keyboard_driver_base::initialize() { this->__on_init(); interrupt_table::add_irq_handler(this->__get_irq_index(), [&]() -> void { this->__on_interrupt(); }); }
bool keyboard_driver_base::__skip_send() { return false; }