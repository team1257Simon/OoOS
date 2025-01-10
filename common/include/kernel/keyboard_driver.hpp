#ifndef __KEYBD_DRIVER
#define __KEYBD_DRIVER
#include "kernel/isr_table.hpp"
enum write_mode : bool
{
    INSERT = false,
    OVERWRITE = true
};
struct kb_state
{
    bool shift      : 1 { false  };
    bool ctrl       : 1 { false  };
    bool alt        : 1 { false  };
    bool fn         : 1 { false  };
    bool numlk      : 1 { false  };
    bool capslk     : 1 { false  };
    write_mode mode : 2 { INSERT };
    constexpr kb_state& operator=(kb_state const& that) 
    {
        shift = that.shift;
        ctrl = that.ctrl;
        alt = that.alt;
        fn = that.fn;
        numlk = that.numlk;
        capslk = that.capslk;
        return *this;
    }
} __pack;
struct kb_data
{
    kb_state    state;
    uint8_t     event_code;
    uint8_t     k_code;
    char        k_char;
} __pack;
typedef std::function<void(kb_data)> kb_listener;
class keyboard_driver_base
{
    std::vector<kb_listener> __listeners{};
    kb_state __current_state{};
protected:
    [[gnu::target("general-regs-only")]] virtual kb_data __get_last(kb_state current_state) = 0;
    virtual void __on_init() = 0;
    virtual byte __get_irq_index() = 0;
    [[gnu::target("general-regs-only")]] virtual bool __skip_send();
public:
    virtual void add_listener(kb_listener&& kl) final;
    virtual void clear_listeners() final;
    virtual void initialize() final;
};
keyboard_driver_base* get_kb_driver();
#endif