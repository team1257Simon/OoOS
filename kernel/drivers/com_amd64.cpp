#include "arch/com_amd64.h"
#include "device_registry.hpp"
#include "isr_table.hpp"
#include "string"
// https://raw.githubusercontent.com/marcv81/termcap/master/termcap.src under "linux" for these control codes
constexpr auto seq_backspace = "\033[D\033[P";
constexpr auto seq_del = "\033[P";
constexpr auto seq_ins = "\033[4h";
com_amd64 com_amd64::__instance{ 64 };
static bool serial_have_input() { line_status_byte b = inb(port_com1_line_status); return b.data_ready; }
static bool serial_empty_transmit() { line_status_byte b = inb(port_com1_line_status); return b.transmitter_buffer_empty; }
static constexpr void write_seq(const char* str) { for(const char* c = str; *c; c++) outb(port_com1, *c); }
std::basic_streambuf<char>* get_kstio_stream() { return com_amd64::get_instance(); }
com_amd64::com_amd64(size_t init_size) : __queue{ init_size }, __base{ init_size } {}
std::streamsize com_amd64::sector_size() { return std::streamsize(16UL); }
std::streamsize com_amd64::unread_size() { return __qrem(); }
void com_amd64::set_echo(bool mode) noexcept { __mode_echo = mode; }
com_amd64* com_amd64::get_instance() { return std::addressof(__instance); }
com_amd64::pos_type com_amd64::seekpos(pos_type pos, std::ios_base::openmode which) { return which.out ? pos_type(off_type(__cur() - __beg())) : pos_type(off_type(__qcur() - __qbeg())); }
com_amd64::pos_type com_amd64::seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which) { return which.out ? pos_type(off_type(__cur() - __beg())) : pos_type(off_type(__qcur() - __qbeg())); }
__isrcall void com_amd64::on_modify_queue() { ptrdiff_t n = gptr() - eback(); if(n > 0) __qsetn(static_cast<size_t>(n)); if(__qbeg()) setg(__qbeg(), __qcur(), __end()); }
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
void com_amd64::__do_echo()
{
    if(!__mode_echo) return;
    char* ptr = __qbeg() + __pos_echo;
    while(ptr < __end())
    {
        for(size_t i = 0; i < 16 && ptr < __end(); i++, __pos_echo++, ++ptr)
        {
            if(*ptr == '\b' || *ptr == 127) { write_seq(seq_backspace); i += 5; }
            else if(ptr + 3 < __end() && *ptr == '\033' && ptr[1] == '[' && ptr[2] == '2' && ptr[3] == '~') { write_seq(seq_ins); ptr += 3; __pos_echo += 3; i += 2; }
            else if(ptr + 3 < __end() && *ptr == '\033' && ptr[1] == '[' && ptr[2] == '3' && ptr[3] == '~') { write_seq(seq_del); ptr += 3; __pos_echo += 3; i += 3; }
            else if(*ptr == 13) outb(port_com1, '\n');
            else outb(port_com1, byte(*ptr));
            while(!serial_empty_transmit()) { pause(); }
        }
    }
}
int com_amd64::write_dev()
{
    char* ptr = __beg();
    while(ptr != __cur()) 
    { 
        for(size_t i = 0; i < 16 && ptr != __cur(); i++, ++ptr) 
        { 
            outb(port_com1, byte(*ptr)); 
            if(*ptr == '\n') 
            {
                // newlines can take a while to process; give the console time to sync up or we might lose characters 
                ptr++; 
                goto buff_wait; 
            } 
        }
    buff_wait:
        while(!serial_empty_transmit()) { pause(); }
    }
    __setc(__beg());
    return 0;
}
__isrcall std::streamsize com_amd64::read_dev(std::streamsize cnt)
{
    std::streamsize result;
    for(result = 0; serial_have_input() && (cnt == 0 || result < cnt); result++) __push_elements(inb(port_com1));
    if(result) __do_echo();
    __pos_echo -= __trim_stale();
    return result;
}
bool com_amd64::init_instance(line_ctl_byte mode, trigger_level_t trigger_level, word baud_div)
{
    __instance.__set_stale_op_threshold(6);
    serial_ier init_ier{ inb(port_com1_ier) };
    outb(port_com1_ier, 0ui8);
    com1_set_baud_divisor(baud_div);
    outb(port_com1_line_ctl, mode);
    outb(port_com1_fifo_ctl, fifo_ctl_byte{ true, true, true, false, trigger_level });
    if(com1_loopback_test()) 
    { 
        outb(port_com1_modem_ctl, modem_ctl_byte{ true, true, true, true, false });
        init_ier.receive_data = true;
        outb(port_com1_ier, init_ier);
        if(interrupt_table::add_irq_handler(4, std::move(LAMBDA_ISR() { __instance.read_dev(0); }))) irq_clear_mask<4ui8>();
        try { __instance.__dev_id = device_registry::get_instance().add(std::addressof(__instance), COM); } catch(std::exception& e) { panic(e.what()); return false; }
        return true; 
    }
    else return false;
}
extern "C"
{
    int get_debug_char() { return com_amd64::get_instance()->sgetc(); }
    void put_debug_char(int ch) { com_amd64::get_instance()->sputc(static_cast<char>(ch)); }
}