#include "amd64_serial.hpp"
using namespace ooos_kernel_module;
module_config<word, line_ctl_byte, trigger_level_t, word> amd64_serial::__cfg = create_config
(
    parameter("port", parameter_type<word>, port_com1),
    parameter("mode", parameter_type<line_ctl_byte>, S8N1),
    parameter("trigger_level", parameter_type<trigger_level_t>, T4BYTE),
    parameter("baud_div", parameter_type<word>, 12US)
);
static bool serial_empty_transmit(word p) { line_status_byte b = inb(p); return b.transmitter_buffer_empty; }
generic_config_table& amd64_serial::get_config() { return __cfg.generic; }
amd64_serial::amd64_serial() = default;
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
    bool result = (inb(port) == 0xAEUC);
    outb(modem_port, prev_ctl);
    return result;
}
bool amd64_serial::initialize()
{
    this->create_buffer(out, 64);
    this->create_buffer(in, 64);
    word port               = get_element<0>(__cfg);
    line_ctl_byte mode      = get_element<1>(__cfg);
    trigger_level_t level   = get_element<2>(__cfg);
    word baud_div           = get_element<3>(__cfg);
    word ier                = ier_port(port);
    serial_ier init_ier     = inb(ier);
    outb(ier, 0UC);
    word ctl                = line_ctl_port(port);
    line_ctl_byte cur_ctl   = inb(ctl);
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
    init_ier.receive_data = true;
    outb(ier, init_ier);
    word sp = line_status_port(port);
    struct { word s; constexpr bool operator()() const noexcept { return static_cast<line_status_byte>(inb(s)).data_ready; }; } have_input(sp);
    this->on_irq(4UC, [&]() -> void
    {
        while(have_input())
        {
            if(!(__input_pos < in.fin))
            {
                size_t ocap = in.capacity();
                this->resize_buffer(in, ocap * 2);
                __input_pos = in.beg + ocap;
            }
            *(in.fin++) = inb(port);
        }
    });
    return true;
}
bool amd64_serial::overflow(size_type needed)
{
    size_t ocap = out.capacity();
    this->resize_buffer(out, needed < ocap ? ocap << 1 : ocap + needed);
    return true;
}
int amd64_serial::sync()
{
    size_t count = 0UZ;
    word p = get_element<0>(__cfg);
    for(char c : out)
    {
        outb(p, static_cast<uint8_t>(c));
        count++;
        if(c == '\n' || count >= 16)
        {
            while(!serial_empty_transmit(p)) 
                pause();
            count = 0UZ;
        }
    }
    __builtin_memset(out.beg, 0, out.capacity());
    out.revert();
    return 0;
}
amd64_serial::size_type amd64_serial::read(char* dest, size_type n)
{
    if(in.cur > in.beg && in.fin > in.cur)
    {
        size_t target = in.remaining();
        char* nbuf = static_cast<char*>(this->allocate_buffer(static_cast<size_t>(target), alignof(char)));
        __builtin_memcpy(nbuf, in.beg, target);
        this->destroy_buffer(in);
        in.set(nbuf, nbuf, nbuf + target);
    }
    return io_module_base<char>::read(dest, n);
}
void amd64_serial::finalize()
{
    if(in.beg)
        this->destroy_buffer(in);
    if(out.beg) 
        this->destroy_buffer(out);
}
EXPORT_MODULE(amd64_serial)