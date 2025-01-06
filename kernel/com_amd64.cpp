#include "arch/com_amd64.h"
static serial_driver_amd64 __inst{};
static void com1_set_baud_divisor(word value)
{
    line_ctl_byte cur_ctl = inb(port_com1_line_ctl);
    cur_ctl.divisor_latch_access = true;
    outb(port_com1_line_ctl, cur_ctl);
    outb(port_com1, value.lo);
    outb(port_com1 + 1, value.hi);
    cur_ctl.divisor_latch_access = false;
    outb(port_com1_line_ctl, cur_ctl);
}
static bool com1_loopback_test()
{
    modem_ctl_byte cur_ctl = inb(port_com1_modem_ctl);
    modem_ctl_byte prev_ctl = cur_ctl;
    cur_ctl.loopback_enable = true;
    cur_ctl.out1_enable = true;
    cur_ctl.out2_enable = true;
    cur_ctl.rts_enable = true;
    outb(port_com1_modem_ctl, cur_ctl);
    outb(port_com1, 0xAE);
    bool result = (inb(port_com1) == 0xAE);
    outb(port_com1_modem_ctl, prev_ctl);
    return result;
}
static bool serial_have_input() { line_status_byte b = inb(port_com1_line_status); return b.data_ready; }
static bool serial_empty_transmit() { line_status_byte b = inb(port_com1_line_status); return b.transmitter_buffer_empty; }
serial_driver_amd64::serial_driver_amd64(size_t init_size) :__queue{ init_size }, __base{ init_size } {}
std::streamsize serial_driver_amd64::__sect_size() { return std::streamsize(16uL); }
std::streamsize serial_driver_amd64::__ddrem() { return this->__qrem(); }
int serial_driver_amd64::__ddwrite()
{
    char* ptr = this->__beg();
    while(ptr != this->__cur()) 
    {
        for(size_t i = 0; i < 16; i++, ++ptr) outb(port_com1, byte(*ptr));
        while(!serial_empty_transmit()) PAUSE;
    }
    return 0;
}
std::streamsize serial_driver_amd64::__ddread(std::streamsize cnt)
{
    std::streamsize result;
    for(result = 0; serial_have_input() && result < cnt; result++) { this->__push_elements(char(inb(port_com1))); }
    return result;
}
void serial_driver_amd64::__q_on_modify() { if(this->__qbeg()) this->setg(this->__qbeg(), this->__qcur(), this->__end()); }
bool serial_driver_amd64::init_instance(line_ctl_byte mode, trigger_level_t trigger_level, word baud_div)
{
    outb(port_com1_ier, 0ui8);
    com1_set_baud_divisor(baud_div);
    outb(port_com1_line_ctl, mode);
    outb(port_com1_fifo_ctl, fifo_ctl_byte{true, true, true, false, trigger_level});
    if(com1_loopback_test()) { outb(port_com1_modem_ctl, modem_ctl_byte{true, true, true, true, false}); return true; }
    else return false;
}