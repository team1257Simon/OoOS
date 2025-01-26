#ifndef __TASK
#define __TASK
#include "sys/types.h"
#ifdef __cplusplus
#include "functional"
typedef int (attribute(sysv_abi) task_closure)(int, char**);
typedef decltype(std::addressof(std::declval<task_closure>())) task_functor;
typedef std::function<task_closure>* task_func_pointer;
extern "C" 
{
#else
typedef int (attribute(sysv_abi) *task_functor)(int argc, char** argv);
typedef void* task_func_pointer;
#endif
struct uframe_tag;
typedef enum
#ifdef __cplusplus
class
#endif
__task_prio
#ifdef __cplusplus
: int8_t
#endif
{
    PVLOW = 0,
    PVNORM = 1,
    PVHIGH = 2,
    PVEXTRA = 3,
    PVSYS = 4
} priority_val;
typedef struct __fx_state
{
    uint16_t fcw;               // BASE+0x000
    uint16_t fsw;               // BASE+0x002
    uint8_t ftw;                // BASE+0x004
    uint8_t rsv0;               // BASE+0x005
    uint16_t fop;               // BASE+0x006
    uint64_t fip;               // BASE+0x008
    uint64_t fdp;               // BASE+0x010
    uint32_t mxcsr;             // BASE+0x018
    uint32_t mxcsr_mask;        // BASE+0x01B
    int128_t stmm[8];           // BASE+0x020
    int128_t xmm[16];           // BASE+0x0A0
    int128_t rsv1[3];           // BASE+0x1A0
    int128_t avail[3];          // BASE+0x1D0
                                // BASE+0x200
} __align(16) __pack fx_state;
typedef struct __reg_state
{
    register_t  rax;    // BASE+0x00=0x10
    register_t  rbx;    // BASE+0x08=0x18
    register_t  rcx;    // BASE+0x10=0x20
    register_t  rdx;    // BASE+0x18=0x28
    register_t  rdi;    // BASE+0x20=0x30
    register_t  rsi;    // BASE+0x28=0x38
    register_t  r8;     // BASE+0x30=0x40
    register_t  r9;     // BASE+0x38=0x48
    register_t  r10;    // BASE+0x40=0x50
    register_t  r11;    // BASE+0x48=0x58
    register_t  r12;    // BASE+0x50=0x60
    register_t  r13;    // BASE+0x58=0x68
    register_t  r14;    // BASE+0x60=0x70
    register_t  r15;    // BASE+0x68=0x78
    vaddr_t     rbp;    // BASE+0x70=0x80
    vaddr_t     rsp;    // BASE+0x78=0x88
    vaddr_t     rip;    // BASE+0x80=0x90
    register_t  rflags; // BASE+0x88=0x98
    uint16_t    ds;     // BASE+0x90=0xA0
    uint16_t    ss;     // BASE+0x92=0xA2
    uint16_t    cs;     // BASE+0x94=0xA4
    vaddr_t     cr3;    // BASE+0x96=0xA6
                        // BASE+0x9E=0xAE
} __pack __align(2) regstate_t;
typedef struct __task_control
{
    struct                      // BASE+0x00
    {
        bool block              : 1; // sleep or wait
        bool can_interrupt      : 1; // true if the wait can be interrupted
        bool notify_cterm       : 1; // true if the wait should be interrupted on a child process termination
        bool sigkill            : 1; // true if the process has stopped due to an abnormal termination (e.g. kill signal)
        priority_val prio_base  : 4; // the base priority of the thread/process
    } __pack __align(1);
    uint8_t skips;              // BASE+0x01; the number of times the task has been skipped for a higher-priority one. The system will escalate a lower-priority process at the front of its queue with enough skips.    
    uintptr_t signal_num;       // BASE+0x02; the sigval union (WIP)
    uint32_t wait_ticks_delta;  // BASE+0x0A; for a sleeping task, how many ticks remain in the set time as an offset from the previous waiting task (or from zero if it is the first waiting process)
    int64_t parent_pid;         // BASE+0x0E; a negative number indicates no parent
    uint64_t task_id;           // BASE+0x16; pid or thread-id; kernel itself is zero (i.e. a task with a parent pid of zero is a kernel task)
                                // BASE+0x1E
} __align(1) __pack tcb_t;
typedef struct __task_info
{
    vaddr_t self;                       // %gs:0x000; self-pointer
    vaddr_t frame_ptr;                  // %gs:0x008; this will be a pointer to a uframe_tag struct for a userspace task. The kernel has its own frame pointer of a different type
    regstate_t saved_regs;              // %gs:0x010 - %gs:0x0AE
    uint16_t quantum_val;               // %gs:0x0AE; base amount of time allocated per timeslice
    uint16_t quantum_rem;               // %gs:0x0B0; amount of time remaining in the current timeslice
    tcb_t task_ctl;                     // %gs:0x0B2 - %gs:0x0D0
    fx_state fxsv;                      // %gs:0x0D0 - %gs:0x2D0    
    uint64_t run_split;                 // %gs:0x2D0; timer-split of when the task began its most recent timeslice; when it finishes, the delta to the current time is added to the run time counter
    uint64_t run_time;                  // %gs:0x2D8; total runtime
    uint64_t sys_time;                  // %gs:0x2E0; approximate time in syscalls
    vaddr_t tls_block;                  // %gs:0x2E8; location for TLS 
    size_t num_child_procs;             // %gs:0x2F0; how many children
    vaddr_t* child_procs;               // %gs:0x2F8; array of pointers to child process info structures (for things like process-tree termination)
    vaddr_t next;                       // %gs:0x300; updated when scheduling event fires.
} __align(16) __pack task_t;
task_t* current_active_task();
[[noreturn]] void user_entry();
void init_pit();
void init_tss(vaddr_t k_rsp);
#ifdef __cplusplus
}
constexpr unsigned int sub_tick_ratio{ 157 };
constexpr unsigned int early_trunc_thresh{ 5 };
constexpr unsigned int cycle_max { 2280 };
constexpr word pit_divisor{ 760ui16 };
constexpr byte pit_mode{ 0x34ui8 };
constexpr word port_pit_data{ 0x40ui16 };
constexpr word port_pit_cmd{ 0x43ui16 };
#endif
#endif