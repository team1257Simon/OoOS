#ifndef __TASK
#define __TASK
#include "sys/types.h"
#if !(defined(__KERNEL__) || defined(__LIBK__))
typedef int32_t spid_t;
typedef void* addr_t;
typedef unsigned long qword;
typedef bool spinlock_t;
#define CXX_INI(...)
#define __pack __attribute__((packed))
#define __align(n) __attribute__((aligned(n)))
#define attribute(...) __attribute__((__VA_ARGS__))
#elif defined(__cplusplus)
#include "kernel/libk_decls.h"
typedef qword sigset_t;
#else
typedef unsigned long sigset_t;
typedef bool spinlock_t;
#endif
#ifdef __cplusplus
constexpr int num_signals = 64;
extern "C" 
{
#else
#define num_signals 64
#endif
typedef void (*signal_handler)(int);
typedef enum
#ifdef __cplusplus
class
#endif
__task_prio
#ifdef __cplusplus
: int8_t
#endif
{
    PVLOW   = 0,
    PVNORM  = 1,
    PVHIGH  = 2,
    PVEXTRA = 3,
    PVSYS   = 4
} priority_val;
typedef enum __sm_action
{
    SIG_BLOCK   = 0,
    SIG_UNBLOCK = 1,
    SIG_SETMASK = 2
} sigprocmask_action;
extern int8_t exception_signals[32];
typedef struct __fx_state
{
    uint16_t    fcw;
    uint16_t    fsw;
    uint8_t     ftw;
    uint8_t     rsv0;
    uint16_t    fop;
    uint64_t    fip;
    uint64_t    fdp;
    uint32_t    mxcsr;
    uint32_t    mxcsr_mask;
    long double stmm[8];
    int128_t    xmm[16];
    int128_t    rsv1[3];
    int128_t    avail[3];
} attribute(packed, aligned(16)) fx_state;
typedef struct __reg_state
{
    register_t  rax;
    register_t  rbx;
    register_t  rcx;
    register_t  rdx;
    register_t  rdi;
    register_t  rsi;
    register_t  r8;
    register_t  r9;
    register_t  r10;
    register_t  r11;
    register_t  r12;
    register_t  r13;
    register_t  r14;
    register_t  r15;
    addr_t      rbp;
    addr_t      rsp;
    addr_t      rip;
    register_t  rflags;
    uint16_t    ds;
    uint16_t    ss;
    uint16_t    cs;
    addr_t      cr3;
} attribute(packed, aligned(2)) regstate_t;
typedef struct __task_signal_info
{
    sigset_t        blocked_signals;
    sigset_t        pending_signals;
    fx_state        sigret_fxsave;
    signal_handler  signal_handlers[num_signals];
    regstate_t      sigret_frame;
    spinlock_t      sigmask_lock;
    int             active_signal;
} attribute(packed, aligned(16)) task_signal_info_t;
typedef struct __task_control
{
    volatile struct 
    {
        bool            block           : 1;        // true if the task is in a sleeping or waiting state
        bool            can_interrupt   : 1;        // true if the wait can be interrupted
        bool            should_notify   : 1;        // true if the wait should be interrupted on a child process termination
        bool            killed          : 1;        // true if the process has stopped due to an abnormal termination (e.g. kill signal)
        priority_val    prio_base       : 4;        // the base priority of the thread/process
    } attribute(packed, aligned(1));
    struct
    {   
        uint8_t             skips;                      // the number of times the task has been skipped for a higher-priority one. The system will escalate a lower-priority process at the front of its queue with enough skips.    
        task_signal_info_t* signal_info;                // points to the signal info struct for the process (handled in the larger, encompassing c++ task_ctx structure)
        uint32_t            wait_ticks_delta;           // for a sleeping task, how many ticks remain in the set time as an offset from the previous waiting task (or from zero if it is the first waiting process)
        spid_t              parent_pid;                 // a negative number indicates no parent
        pid_t               task_id;                    // pid or thread-id; kernel itself is zero (i.e. a task with a parent pid of zero is a kernel task)
        uid_t               task_uid;                   // WIP
        gid_t               task_gid;                   // WIP
    } attribute(packed, aligned(1));
} __pack tcb_t;
typedef struct __task_info
{
    addr_t      self;                               // self-pointer
    addr_t      frame_ptr;                          // this will be a pointer to a uframe_tag struct for a userspace task. The kernel has its own frame pointer of a different type
    regstate_t  saved_regs;                         // this stores all the register states when the process is not active
    uint16_t    quantum_val;                        // base amount of time allocated per timeslice
    uint16_t    quantum_rem     CXX_INI(0US);       // amount of time remaining in the current timeslice
    tcb_t       task_ctl;                           // this contains information related to the process' scheduling information, as well as PID and signaling info
    fx_state    fxsv            CXX_INI();          // stored state for floating-point registers
    uint64_t    run_split       CXX_INI(0UL);       // timer-split of when the task began its most recent timeslice; when it finishes, the delta to the current time is added to the run time counter
    uint64_t    run_time        CXX_INI(0UL);       // total runtime
    uint64_t    sys_time        CXX_INI(0UL);       // approximate time in syscalls (for the moment, all syscalls simply count as 1)
    addr_t      tls_block;                          // pointer to the end of the process TLS template block (WIP)
    size_t      num_child_procs;                    // how many children are in the array below
    addr_t*     child_procs;                        // array of pointers to child process info structures (for things like process-tree termination)
    addr_t      next            CXX_INI(nullptr);   // updated when the scheduling event fires.
} attribute(packed, aligned(16)) task_t;
#ifdef __cplusplus
}
#endif
#endif