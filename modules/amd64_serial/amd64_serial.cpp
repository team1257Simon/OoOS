#include "amd64_serial.hpp"
amd64_serial::config_type amd64_serial::__cfg = serial_config();
static bool serial_empty_transmit(word p) { line_status_byte b = inb(p); return b.transmitter_buffer_empty; }
static bool serial_have_input(word p) { line_status_byte b = inb(p); return b.data_ready; }
amd64_serial::size_type amd64_serial::avail() const { return __input_pos > in.cur ? static_cast<size_type>(__input_pos - in.cur) : 0UZ; }
ooos::generic_config_table& amd64_serial::get_config() { return __cfg.generic; }
amd64_serial::amd64_serial() = default;
void amd64_serial::__trim_old()
{
    size_type rem       = amd64_serial::avail();
    size_type old_cap   = in.capacity();
    pointer new_buf     = static_cast<pointer>(allocate_buffer(in.capacity(), alignof(value_type)));
    __builtin_memcpy(new_buf, in.cur, rem);
    destroy_buffer(in);
    in.set(new_buf, new_buf, new_buf + old_cap);
    __input_pos         = in.beg + rem;
}
static bool loopback_test(word port)
{
    word modem_port         = modem_ctl_port(port);
    modem_ctl_byte cur_ctl  = inb(modem_port);
    modem_ctl_byte prev_ctl = cur_ctl;
    cur_ctl.loopback_enable = true;
    cur_ctl.out1_enable     = true;
    cur_ctl.out2_enable     = true;
    cur_ctl.rts_enable      = true;
    outb(modem_port, cur_ctl);
    outb(port, 0xAE);
    bool result             = (inb(port) == 0xAEUC);
    outb(modem_port, prev_ctl);
    return result;
}
bool amd64_serial::initialize()
{
    create_buffer(out, 64);
    create_buffer(in, 16);
    __input_pos                     = in.beg;
    word port                       = ooos::get_element<0>(__cfg);
    trigger_level_t level           = static_cast<trigger_level_t>(ooos::get_element<2>(__cfg));
    word baud_div                   = ooos::get_element<3>(__cfg);
    word ier                        = ier_port(port);
    word sp                         = line_status_port(port);
    serial_ier init_ier             = inb(ier);
    line_ctl_byte mode(ooos::get_element<1>(__cfg));
    outb(ier, 0UC);
    word ctl                        = line_ctl_port(port);
    line_ctl_byte cur_ctl           = inb(ctl);
    cur_ctl.divisor_latch_access    = true;
    outb(ctl, cur_ctl);
    outb(port, baud_div.lo);
    outb(port + 1, baud_div.hi);
    cur_ctl.divisor_latch_access    = false;
    outb(ctl, cur_ctl);
    outb(ctl, mode);
    outb(iir_or_fifo_port(port), fifo_ctl_byte(true, true, true, false, level));
    if(__unlikely(!loopback_test(port))) return false;
    outb(modem_ctl_port(port), modem_ctl_byte(true, true, true, true, false));
    init_ier.receive_data           = true;
    outb(ier, init_ier);
    on_irq(4UC, [=, this]() -> void
    {
        do {
            if(!(__input_pos < in.fin))
            {
                if(in.size() && !ooos::get_element<4>(__cfg)) __trim_old();
                else
                {
                    size_t ocap = in.capacity();
                    resize_buffer(in, ocap * 2);
                    __input_pos = in.beg + ocap;
                }
            }
            __input_pos[0]  = inb(port);
            __input_pos++;
        } while(serial_have_input(sp));
    });
    device_id = register_device(this, COM);
    return true;
}
bool amd64_serial::overflow(size_type needed)
{
    size_t ocap = out.capacity();
    resize_buffer(out, needed < ocap ? ocap << 1 : ocap + needed);
    return true;
}
int amd64_serial::sync()
{
    size_t count    = 0UZ;
    word p          = ooos::get_element<0>(__cfg);
    word sp         = line_status_port(p);
    for(char c : out)
    {
        outb(p, static_cast<uint8_t>(c));
        count++;
        if(c == '\n' || count >= 16)
        {
            while(!serial_empty_transmit(sp)) 
                pause();
            count = 0UZ;
        }
    }
    __builtin_memset(out.beg, 0, out.capacity());
    out.revert();
    return 0;
}
typename amd64_serial::size_type amd64_serial::read(pointer dest, size_type n)
{
    size_type rem = amd64_serial::avail();
    size_type old = in.size();
    if(rem && old && ooos::get_element<4>(__cfg)) __trim_old();
    if(n > rem) n = rem;
    __builtin_memcpy(dest, in.cur, n);
    in.cur += n;
    return n;
}
void amd64_serial::finalize()
{
    if(in.beg)
        destroy_buffer(in);
    if(out.beg) 
        destroy_buffer(out);
    deregister_device(this);
}
EXPORT_MODULE(amd64_serial);