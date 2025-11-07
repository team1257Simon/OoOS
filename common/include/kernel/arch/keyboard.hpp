#ifndef __KEYBD
#define __KEYBD
#include <util/circular_queue.hpp>
namespace ooos
{
	typedef circular_queue<uint8_t> byte_queue;
    enum class keyboard_scanset : uint8_t
    {
        SC_SET1 = 0x43UC,
        SC_SET2 = 0x41UC,
        SC_SET3 = 0x3FUC
    };
    enum class keycode : wchar_t
    {
        KC_MAX_CHAR     = 0x10FFFFU,        // Anything greater than this is a special key, an escape byte, or an unknown scan
        KC_F1           = 0x110001U,
        KC_F2           = 0x110002U,
        KC_F3           = 0x110003U,
        KC_F4           = 0x110004U,
        KC_F5           = 0x110005U,
        KC_F6           = 0x110006U,
        KC_F7           = 0x110007U,
        KC_F8           = 0x110008U,
        KC_F9           = 0x110009U,
        KC_F10          = 0x11000AU,
        KC_F11          = 0x11000BU,
        KC_F12          = 0x11000CU,
        KC_LCTRL        = 0x120001U,        // Left Ctrl
        KC_LSHFT        = 0x120002U,        // Left Shift
        KC_LALT         = 0x120003U,        // Left Alt
        KC_RCTRL        = 0x120004U,        // Right Ctrl
        KC_RSHFT        = 0x120005U,        // Right Shift
        KC_RALT         = 0x120006U,        // Right Alt
        KC_CAPS         = 0x120007U,        // Caps Lock
        KC_NUM          = 0x120008U,        // Num Lock
        KC_SCRLL        = 0x120009U,        // Scroll Lock
        KC_INS          = 0x12000AU,        // Insert
        KC_LGUI         = 0x130001U,        // Left Windows/GUI key
        KC_RGUI         = 0x130002U,        // Right Windows/GUI key
        KC_APPS         = 0x130003U,        // "Apps" key
        KC_PAUSE        = 0x130004U,        // Pause
        KC_PRTSC        = 0x130005U,        // Print Screen
        KC_END          = 0x140001U,
        KC_HOME         = 0x140002U,
        KC_PG_UP        = 0x140003U,        // Page Up
        KC_PG_DN        = 0x140004U,        // Page Down
        KC_LEFT         = 0x140005U,
        KC_RIGHT        = 0x140006U,
        KC_UP           = 0x140007U,
        KC_DOWN         = 0x140008U,
        KC_POWER        = 0x150001U,
        KC_SLEEP        = 0x150002U,
        KC_WAKE         = 0x150003U,
        KC_M1           = 0x160001U,        // Multimedia key 1 (Previous track)
        KC_M2           = 0x160002U,        // Multimedia key 2 (Next track)
        KC_M3           = 0x160003U,        // Multimedia key 3 (Mute)
        KC_M4           = 0x160004U,        // Multimedia key 4 (Calculator)
        KC_M5           = 0x160005U,        // Multimedia key 5 (Play)
        KC_M6           = 0x160006U,        // Multimedia key 6 (Stop)
		KC_M7           = 0x160007U,        // Multimedia key 7 (Volume down)
        KC_M8           = 0x160008U,        // Multimedia key 8 (Volume up)
        KC_M9           = 0x160009U,        // Multimedia key 9 (WWW home)
        KC_M10          = 0x16000AU,        // Multimedia key 10 (WWW search)
		KC_M11          = 0x16000BU,        // Multimedia key 11 (WWW favorites)
        KC_M12          = 0x16000CU,        // Multimedia key 12 (WWW refresh)
        KC_M13          = 0x16000DU,        // Multimedia key 13 (WWW stop)
        KC_M14          = 0x16000EU,        // Multimedia key 14 (WWW forward)
		KC_M15          = 0x16000FU,        // Multimedia key 15 (WWW back)
        KC_M16          = 0x160010U,        // Multimedia key 16 (My Computer)
        KC_M17          = 0x160011U,        // Multimedia key 17 (Email)
        KC_M18          = 0x160012U,        // Multimedia key 18 (Media Select)
        KC_BRB          = 0x1700F0U,        // Placeholder for a break code escape byte in scancode tables (generally 0xF0)
        KC_EXB          = 0x1700E0U,        // Placeholder for a general escape byte in scancode tables (generally 0xE0)
        KC_SEQB         = 0x1700E1U,        // Placeholder for an atomic sequence (i.e. no separate break sequence) escape byte in scancode tables (generally 0xE1)
        KC_SEQ_UNKNOWN  = 0x1E0000U,        // Placeholder for an unrecognized scan sequence; the first 2 scancodes received will be a little-endian word in the low 16 bits
        KC_UNKNOWN      = 0x1F0000U,        // Placeholder for an unrecognized single-byte scan; the scancode will be in the lowest 8 bits
        KC_EXT_UNKNOWN  = 0x1FE000U,        // Placeholder for an unrecognized escaped scan; the unrecognized byte (i.e. after the escape byte) will be in the lowest 8 bits
        // Note: Multimedia keys are not present on all keyboards. I am also uncertain as to the mappings' consistency, hence the use of generic numbers.
        // The mappings given are the ones mentioned on OSDev Wiki.
    };
	enum class typematic_repeat_rate : uint8_t { RR_MIN = 0x00UC, RR_DEFAULT = 0x14UC, RR_MAX = 0x1FUC };
	constexpr typematic_repeat_rate operator+(typematic_repeat_rate a, uint8_t b) noexcept { return static_cast<typematic_repeat_rate>(static_cast<uint8_t>(a) + b < 0x1FUC ? static_cast<uint8_t>(a) + b : 0x1FUC); }
	constexpr typematic_repeat_rate operator-(typematic_repeat_rate a, uint8_t b) noexcept { return static_cast<typematic_repeat_rate>(b < static_cast<uint8_t>(a) ? static_cast<uint8_t>(a) - b : 0UC); }
	constexpr typematic_repeat_rate& operator+=(typematic_repeat_rate& a, uint8_t b) noexcept { return (a = a + b); }
	constexpr typematic_repeat_rate& operator-=(typematic_repeat_rate& a, uint8_t b) noexcept { return (a = a - b); }
	enum class typematic_delay : uint8_t { TD_250 = 0b00UC, TD_500 = 0b01UC, TD_750 = 0b10UC, TD_1000 = 0b11UC };
	enum class keyboard_id_bytes : uint16_t
	{
		KI_MF2_A 	= __be16(0xABUC, 0x83UC),
		KI_MF2_B	= __be16(0xABUC, 0xC1UC),
		KI_SHORT_KB	= __be16(0xABUC, 0x84UC),
		KI_NCD_N97	= __be16(0xABUC, 0x85UC),
		KI_STANDARD	= __be16(0xABUC, 0x86UC),
		KI_JP_G		= __be16(0xABUC, 0x90UC),
		KI_JP_P		= __be16(0xABUC, 0x91UC),
		KI_JP_A		= __be16(0xABUC, 0x92UC),
		KI_NCD_SUN	= __be16(0xACUC, 0xA1UC),
	};
	struct __pack typematic_byte
	{
		typematic_repeat_rate repeat_rate 	: 	5 	= typematic_repeat_rate::RR_DEFAULT;
		typematic_delay delay_millis		:	2	= typematic_delay::TD_500;
		bool								:	1;
	};
	// Controller state variables
	struct __pack keyboard_cstate
	{
		keyboard_scanset 	selected_set;
		typematic_byte		current_typematic_byte;
		uint8_t				id_bytes[2];
	};
	// State-vector for the basic toggle-states of the keyboard.
    struct __pack keyboard_vstate
    {
        bool left_ctrl   : 1     = false;
        bool right_ctrl  : 1     = false;
        bool left_alt    : 1     = false;
        bool right_alt   : 1     = false;
        bool left_shift  : 1     = false;
        bool right_shift : 1     = false;
        bool caps_lock   : 1     = false;
        bool num_lock    : 1     = false;
    };
    struct __pack keyboard_event
    {
        // The unicode codepoint of the character, if any, or the keycode for a special key (arrow keys, shift, caps lock, Fn keys, etc.).
        // The value of a special key will be out of range for unicode, so any such value indicates a special key.
        wchar_t kv_code     : 21;
        // If this bit is set, this is a break (key-up) event; if it is clear, it is a make (key-down) event.
        bool kv_release     : 1;
        // If this bit is set, the key is a num-pad numeral or symbol (the character code will remain the same).
        bool kv_numpad      : 1;
        // If this bit is set, the key generated multiple scancodes (other than escape bytes; e.g. BREAK or PAUSE in sets 1 and 2).
        bool kv_multiscan   : 1;
        // The state of the keyboard BEFORE the event — for example, if the key is right shift and the event is a key up, the right shift bit will still be set.
        // In prototype packets (the ones in the decode tables), the value will instead be an XOR mask to compute the state following the event.
        keyboard_vstate kv_vstate;
    };
    class keyboard_scan_decoder
    {
        keyboard_event const* __scans;
        keyboard_event const* __escaped_scans;
        uint8_t const* __pause_sequence;
        size_t __pause_sequence_length;
        keyboard_event __decode_one(uint8_t scan, byte_queue& rem) const;
        keyboard_event __decode_one_escaped(uint8_t scan, byte_queue& rem) const;
        keyboard_event __decode_one_seq(uint8_t seq_bookend, byte_queue& rem) const;
    public:
        keyboard_vstate current_state;
        keyboard_scan_decoder(keyboard_scanset ss) noexcept;
        keyboard_scan_decoder() noexcept;
        void set_scanset(keyboard_scanset ss) noexcept;
        keyboard_event decode(byte_queue& scan_bytes);
    };
	namespace __internal
	{
		template<typename T> concept __cmd_result_accept = std::is_invocable_v<T, uint8_t, uint8_t>;
		template<__cmd_result_accept FT> void __invoke_c(FT&& f, uint8_t a, uint8_t b) noexcept(std::is_nothrow_invocable_v<FT, uint8_t, uint8_t>) { __forward<FT>(f)(a, b); }
		struct __cmd_callback_base
		{
			template<typename R, typename C, typename ... Args> using member_fn = R (C::*)(Args...);
			template<typename GT> constexpr static bool not_empty(GT* gt) noexcept { return gt != nullptr; }
			template<typename R, typename C, typename ... Args> constexpr static bool not_empty(member_fn<R, C, Args...> mf) noexcept { return mf != nullptr; }
			template<boolable T> constexpr static bool not_empty(T t) noexcept { return t ? true : false; }
			template<typename T> constexpr static bool not_empty(T t) noexcept { return true; }
			template<__cmd_result_accept FT>
			class __cb_manager
			{
				constexpr static size_t __max_size  = sizeof(actors);
            	constexpr static size_t __max_align = alignof(actors);
				template<typename GT> constexpr static void __create_wrapper(actor_store& dst, GT&& ftor, std::true_type) { new(dst.access()) FT(__forward<GT>(ftor)); }
				template<typename GT> constexpr static void __create_wrapper(actor_store& dst, GT&& ftor, std::false_type) { dst.set_ptr(new FT(__forward<FT>(ftor))); }
				constexpr static void __destroy_wrapper(actor_store& target, std::true_type) { target.template cast<FT>().~FT(); }
				constexpr static void __destroy_wrapper(actor_store& target, std::false_type) { ::operator delete(target.template cast<FT*>()); }
				using __is_locally_storable = std::bool_constant<std::is_trivially_copyable_v<FT> && (sizeof(FT) <= __max_size && alignof(FT) <= __max_align && __max_align % alignof(FT) == 0)>;
            	constexpr static bool __is_local_store = __is_locally_storable::value;
			public:
				constexpr static FT* get_ptr(actor_store const& src) { if constexpr(__is_local_store) { return __addressof(const_cast<FT&>(src.template cast<FT>())); } else { return src.template cast<FT*>(); } }
				template<typename GT> constexpr static void init_cb(actor_store& dst, GT&& src) {  __create_wrapper(dst, __forward<GT>(src), __is_locally_storable());  }
				constexpr static void destroy_cb(actor_store& dst) { __delete_wrapper(dst, __is_locally_storable()); }
				static void invoke_fn(actor_store& fn, uint8_t a, uint8_t b) { __invoke_c(__forward<FT>(*get_ptr()), a, b); }
				static void action(actor_store& dst, actor_store const& src, mgr_op op)
				{
					switch(op)
					{
					case get_pointer:
						dst.template cast<FT*>() = get_ptr(src);
						break;
					case clone:
						init_cb(dst, *const_cast<FT const*>(get_ptr(src)));
						break;
					case destroy:
						destroy_cb(dst);
						break;
					case get_type_info:
						dst.set_ptr(&typeid(FT));
						break;
					}
				}
			};
			using manager_type = void(*)(actor_store&, actor_store const&, mgr_op);
			using invoker_type = void(*)(actor_store&, uint8_t, uint8_t);
			actor_store __my_callback;
			manager_type __my_manager;
			invoker_type __my_invoke;
			constexpr bool __empty() const noexcept { return !__my_manager; }
			constexpr ~__cmd_callback_base() noexcept { if(__my_manager) { __my_manager(__my_callback, __my_callback, destroy); } }
			constexpr void __swap_with(__cmd_callback_base& that) noexcept { __swap(this->__my_callback, that.__my_callback); __swap(this->__my_invoke, that.__my_invoke); __swap(this->__my_manager, that.__my_manager); }
		};
	}
	struct keyboard_cmd_callback;
	namespace __internal { template<typename T> concept __wrappable_cb = __cmd_result_accept<T> && !std::is_same_v<keyboard_cmd_callback, T>; }
	struct keyboard_cmd_callback : private __internal::__cmd_callback_base
	{
		constexpr keyboard_cmd_callback() noexcept = default;
		constexpr ~keyboard_cmd_callback() noexcept = default;
		constexpr operator bool() const noexcept { return !__empty(); }
		template<__internal::__wrappable_cb FT>
		constexpr keyboard_cmd_callback(FT&& ft)
		{
			typedef __cb_manager<std::decay_t<FT>> __mgr;
			if(not_empty(ft))
			{
				__mgr::init_actor(__my_callback, __forward<FT>(ft));
				__my_manager    = &__mgr::action;
                __my_invoke     = &__mgr::invoke_fn;
			}
		}
		constexpr keyboard_cmd_callback(keyboard_cmd_callback const& that) : __internal::__cmd_callback_base()
		{
			if(static_cast<bool>(that))
			{
				that.__my_manager(this->__my_callback, that.__my_callback, clone);
				this->__my_manager  = that.__my_manager;
                this->__my_invoke   = that.__my_invoke;
			}
		}
		constexpr keyboard_cmd_callback(keyboard_cmd_callback&& that) : __internal::__cmd_callback_base()
		{
			if(static_cast<bool>(that))
			{
				this->__my_callback    = that.__my_callback;
                this->__my_manager  = that.__my_manager;
                this->__my_invoke   = that.__my_invoke;
				that.__my_manager   = nullptr;
                that.__my_invoke    = nullptr;
			}
		}
		constexpr void swap(keyboard_cmd_callback& that) noexcept { this->__swap_with(that); }
		constexpr keyboard_cmd_callback& operator=(keyboard_cmd_callback const& that) { keyboard_cmd_callback(that).swap(*this); return *this; }
		constexpr keyboard_cmd_callback& operator=(keyboard_cmd_callback&& that) { keyboard_cmd_callback(__move(that)).swap(*this); return *this; }
		constexpr void operator()(uint8_t a, uint8_t b) { if(!__empty()) { __my_invoke(__my_callback, a, b); } }
		constexpr std::type_info const& target_type() const noexcept { if(__my_manager) { actor_store tmp_store; __my_manager(tmp_store, __my_callback, get_type_info); if(std::type_info const* result = tmp_store.cast<std::type_info const*>()) return *result; } return typeid(nullptr); }
	};
	struct keyboard_command
	{
		bool has_sub_cmd;
		uint8_t cmd_byte;
		uint8_t sub_byte;
		uint8_t max_response_bytes;
		keyboard_cmd_callback callback;
	};
	
}
#endif