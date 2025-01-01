#include "arch/kb_amd64.hpp"
keyboard_driver_amd64 __inst{};
constexpr static char sc_lower[] = { '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '"', '`', '\0', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '*', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', '\0', '\0', '\0', '\0', '\0' };
constexpr static char sc_shift[] = { '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\'', '~', '\0', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\r', '\0', '\0', '-', '\0', '5', '\0', '+', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
constexpr char case_diff = ('a' - 'A');
__isr_registers kb_data keyboard_driver_amd64::__get_next(kb_state current_state, bool extd)
{
    byte scan = kb_get();
    byte id = scan & ~KEY_UP;
    if(scan == KEYDN_SPECIAL) { __skip = true; return __get_next(current_state, true); }
    kb_data result{};
    result.k_code = id;
    if(extd || !current_state.numlk)
    {
        result.k_char = '\0';
        if((scan & KEY_UP) != 0)
        { 
            result.k_code = id & ~KEY_UP; 
            result.event_code = byte(KEYUP_SPECIAL); 
            if(id == K_LALT) { result.state.alt = false; return result; }
            else if(id == K_LCTRL) { result.state.ctrl = false; return result; }
        }
        else 
        { 
            result.k_code = id;
            result.event_code = byte(KEYDN_SPECIAL);
            if(id == K_INSERT) result.state.mode = write_mode(!current_state.mode); 
            else if(id == K_LALT) { result.state.alt = true; return result; }
            else if(id == K_LCTRL) { result.state.ctrl = true; return result; }
        }
        if(id <= 0x53 && id >= 0x47) return result;
    }
    bool is_up = (scan & KEY_UP) != 0;
    if(id == K_LSHIFT || id == K_RSHIFT) result.state.shift = !is_up;
    else if(id == K_LALT) result.state.alt = !is_up;
    else if(id == K_LCTRL) result.state.ctrl = !is_up;
    if(is_up)
    {
        result.event_code = byte(KEY_UP);
        result.k_char = '\0';
    }
    else
    {
        result.event_code = byte(KEY_DN);
        result.k_char = (current_state.shift ? sc_shift : sc_lower)[id];
        if(current_state.capslk) 
        {
            if(result.k_char >= 'a' && result.k_char <= 'z') result.k_char -= case_diff;
            else if(result.k_char >= 'A' && result.k_char <= 'Z') result.k_char += case_diff;
        }
        if(id == K_NUMLK) result.state.numlk = !current_state.numlk;
        else if(id == K_CAPS) result.state.capslk = !current_state.capslk;
    }
    
    return result;
}
__isr_registers kb_data keyboard_driver_amd64::__get_last(kb_state current_state) { return this->__get_next(current_state, false); }
__isr_registers bool keyboard_driver_amd64::__skip_send() { if(__skip) { __skip = false; return true; } return false; }
void keyboard_driver_amd64::__on_init() { kb_put(0xF4); kb_get(); irq_clear_mask<1>(); }
byte keyboard_driver_amd64::__get_irq_index() { return 1; }
keyboard_driver_base* get_kb_driver() { return static_cast<keyboard_driver_base*>(&__inst); }
