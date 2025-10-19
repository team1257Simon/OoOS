#ifndef __KB_AMD64
#define __KB_AMD64
#include "kernel/keyboard_driver.hpp"
#include "kernel/arch/arch_amd64.h"
// All scan codes for release are the same as the corresponding key down but with the high bit set
enum scancode_set1 : uint8_t
{
    K_ESC = 0x01,
    K_1 = 0x02, K_2 = 0x03, K_3 = 0x04, K_4 = 0x05, K_5 = 0x06, K_6 = 0x07, K_7 = 0x08, K_8 = 0x09, K_9 = 0x0A, K_0 = 0x0B,
    K_MINUS = 0x0C, K_EQUALS = 0x0D, K_BACKSP = 0x0E,
    K_TAB = 0x0F,
    K_Q = 0x10, K_W = 0x11, K_E = 0x12, K_R = 0x13, K_T = 0x14, K_Y = 0x15, K_U = 0x16, K_I = 0x17, K_O = 0x18, K_P = 0x19,
    K_LBRACE = 0x1A, K_RBRACE = 0x1B,
    K_ENTER = 0x1C,
    K_LCTRL = 0x1D,
    K_A = 0x1E, K_S = 0x1F, K_D = 0x20, K_F = 0x21, K_G = 0x22, K_H = 0x23, K_J = 0x24, K_K = 0x25, K_L = 0x26, K_SEMICOLON = 0x27,
    K_APOS = 0x28, K_TICK = 0x29,
    K_LSHIFT = 0x2A,
    K_BACKSLASH = 0x2B,
    K_Z = 0x2C, K_X = 0x2D, K_C = 0x2E, K_V = 0x2F, K_B = 0x30, K_N = 0x31, K_M = 0x32, K_COMMA = 0x33, K_PERIOD = 0x34, K_SLASH = 0x35,
    K_RSHIFT = 0x36,
    K_AST = 0x37,
    K_LALT = 0x38,
    K_SPACE = 0x39,
    K_CAPS = 0x3A,
    K_F1 = 0x3B, K_F2 = 0x3C, K_F3 = 0x3D, K_F4 = 0x3E, K_F5 = 0x3F, K_F6 = 0x40, K_F7 = 0x41, K_F8 = 0x42, K_F9 = 0x43, K_F10 = 0x44,
    K_NUMLK = 0x45, K_SCRLK = 0x46,
    K_PAD1 = 0x4F, K_PAD2 = 0x50, K_PAD3 = 0x51, K_PAD4 = 0x4B, K_PAD5 = 0x4C, 
    K_PAD6 = 0x4D, K_PAD7 = 0x47, K_PAD8 = 0x48, K_PAD9 = 0x49, K_PAD0 = 0x52,
    K_PADMINUS = 0x4A, K_PADPLUS = 0x4E, K_PADPERIOD = 0x53, K_F11 = 0x57, K_F12 = 0x58,
    SPECIAL_PREFIX = 0xE0
};
enum ext_code_set1 : uint8_t
{
    K_MUTE = 0x20, 
    K_CAL = 0x21,
    K_PLAY = 0x22,
    K_STOP = 0x23,
    K_VOLDN = 0x2E,
    K_VOLUP = 0x30,
    K_HOME = 0x47,
    K_UP = 0x48,
    K_PAGEUP = 0x49,
    K_LEFT =  0x4B,
    K_RIGHT = 0x4D,
    K_END = 0x4F,
    K_DOWN = 0x50,
    K_PAGEDOWN = 0x51,
    K_INSERT = 0x52,
    K_DEL = 0x53
};
enum event_type : uint8_t
{
    KEY_DN = 0x00,
    KEY_UP = 0x80,
    KEYDN_SPECIAL = 0xE0,
    KEYUP_SPECIAL = 0xF0
};
class keyboard_driver_amd64 : public keyboard_driver
{
    bool __skip = false;
    kb_data __get_next(kb_state current_state, bool extd);
protected:
	virtual kb_data __get_last(kb_state current_state) override;
    virtual void __on_init() override;
    virtual byte __get_irq_index() override;
    virtual bool __skip_send() override;
};
#endif