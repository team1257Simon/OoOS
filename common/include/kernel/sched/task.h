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
    PVSYS = -1,
    PVLOW = 0,
    PVNORM = 1,
    PVHIGH = 2,
    PVEXTRA = 3
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
} __align(1) __pack fx_state;
typedef struct __reg_state
{
    register_t  rax;    // BASE+0x00
    register_t  rbx;    // BASE+0x08
    register_t  rcx;    // BASE+0x10
    register_t  rdx;    // BASE+0x18
    register_t  rdi;    // BASE+0x20
    register_t  rsi;    // BASE+0x28
    register_t  r8;     // BASE+0x30
    register_t  r9;     // BASE+0x38
    register_t  r10;    // BASE+0x40
    register_t  r11;    // BASE+0x48
    register_t  r12;    // BASE+0x50
    register_t  r13;    // BASE+0x58
    register_t  r14;    // BASE+0x60
    register_t  r15;    // BASE+0x68
    vaddr_t     rbp;    // BASE+0x70
    vaddr_t     rsp;    // BASE+0x78
    vaddr_t     rip;    // BASE+0x80
    register_t  rflags; // BASE+0x88
    uint16_t    ds;     // BASE+0x90
    uint16_t    es;     // BASE+0x92
    uint16_t    ss;     // BASE+0x94
    uint16_t    fs;     // BASE+0x96
    uint16_t    gs;     // BASE+0x98
    uint16_t    cs;     // BASE+0x9A
    vaddr_t     cr3;    // BASE+0x9C
                        // BASE+0xA4
} __pack __align(1) regstate_t;
typedef struct __task_control
{
    struct                      // BASE+0x00
    {
        bool block          : 1; // sleep or wait
        bool can_interrupt  : 1; // true if the wait can be interrupted
        bool sigxx          : 1; // signal flag to process (only one of these for now)
        bool sysrq          : 1; // signal flag from process (only one for now)
        bool                : 4; // Look at me, I get to be the one reserving bits this time!
    } __align(1) __pack;
    uint8_t sigcode;            // BASE+0x01; the error code associated with a signal if applicable
    priority_val prio_base;     // BASE+0x02; the base priority of the thread/process
    uint8_t skips;              // BASE+0x03; the number of times the task has been skipped for a higher-priority one. The system will escalate a lower-priority process at the front of its queue with enough skips.
    uint32_t wait_ticks_delta;  // BASE+0x04; for a sleeping task, how many ticks remain in the set time as an offset from the previous waiting task (or from zero if it is the first waiting process)
    int64_t parent_pid;         // BASE+0x08; a negative number indicates no parent
    uint64_t task_id;           // BASE+0x10; pid or thread-id; kernel itself is zero (i.e. a task with a parent pid of zero is a kernel task)
    uint64_t run_split;         // BASE+0x18; timer-split of when the task began its most recent timeslice; when it finishes, the delta to the current time is added to the run time counter
    uint64_t run_time;          // BASE+0x20
                                // BASE+0x28
} __pack tcb_t;
typedef struct __tls_store_t
{
    vaddr_t fs_baseptr;       // %fs:0x000; this will be a self-pointer
    vaddr_t gs_baseptr;       // %fs:0x008; pointer back to gs base
    size_t tl_data_size;      // %fs:0x010; size of the array following
    char tl_data[];           // %fs:0x018 - ???; will include thread-local variables like errno (eventually)
} __pack tls_t;
typedef struct __task_info
{
    vaddr_t self;                       // %gs:0x000; self-pointer
    vaddr_t frame_ptr;                  // %gs:0x008; this will be a pointer to a uframe_tag struct for a userspace task. The kernel has its own frame pointer of a different type
    regstate_t saved_regs;              // %gs:0x010 - %gs:0x0B4
    uint16_t quantum_val;               // %gs:0x0B4; base amount of time allocated per timeslice
    uint16_t quantum_rem;               // %gs:0x0B6; amount of time remaining in the current timeslice
    tcb_t task_ctl;                     // %gs:0x0B8
    fx_state fxsv;                      // %gs:0x0E0 - %gs:0x2E0
    size_t num_child_procs;             // %gs:0x2E0
    vaddr_t* child_procs;               // %gs:0x2E8; array of pointers to child process info structures (for things like process-tree termination)
    vaddr_t tls_block;                  // %gs:0x2F0; location for TLS 
    vaddr_t next;                       // %gs:0x2F8; updated when scheduling event fires.
    task_func_pointer exec_fn;          // %gs:0x300; called by wrapper entry function used if we can't get a direct C-style function pointer
} __pack task_t;
task_t* current_active_task();
[[noreturn]] void user_entry();
void init_pit();
int attribute(sysv_abi) task_wrapper_exec(int argc, char** argv);
#ifdef __cplusplus
}
constexpr unsigned int ticks_per_millis{ 596591 };
constexpr unsigned char early_trunc_thresh{ 6 };
constexpr word pit_divisor{ 2ui8 };
constexpr byte pit_mode{ 0x34ui8 };
constexpr word port_pit_data{ 0x40ui16 };
constexpr word port_pit_cmd{ 0x43ui16 };
#endif
#endif