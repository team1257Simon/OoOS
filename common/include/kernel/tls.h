#ifndef __TLS
#define __TLS
#include "sys/types.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct __fx_state
{
    uint16_t fcw;
    uint16_t fsw;
    uint8_t ftw;
    uint8_t rsv0;
    uint16_t fop;
    uint64_t fip;
    uint64_t fdp;
    uint32_t mxcsr;
    uint32_t mxcsr_mask;
    int128_t stmm[8];
    int128_t xmm[16];
    int128_t rsv1[3];
    int128_t avail[3];
} __align(1) __pack fx_state;
typedef struct __thread_control
{
    register_t saved_rsp;       // %fs:0x08
    register_t saved_rbp;       // %fs:0x10
    register_t saved_rip;       // %fs:0x18
    struct                      // %fs:0x20
    {
        bool block          : 1; // sleep or wait
        bool can_interrupt  : 1; // true if the wait can be interrupted
        bool sigxx          : 1; // signal flag to process (only one of these for now)
        bool sysrq          : 1; // signal flag from process (only one for now)
        bool                : 4; // Look at me, I get to be the one reserving bits this time!
    } __align(1) __pack;
    int8_t prio_base;           // %fs:0x21; the base priority of the thread/process
    uint16_t skips;             // %fs:0x22; the number of times the process has been skipped for a higher-priority one 
    uint32_t wait_ticks_delta;  // %fs:0x24; for a sleeping process, how many ticks remain in the set time as an offset from the previous waiting process (or from zero if it is the first waiting process)
    int64_t owner_pid;          // %fs:0x28
    int64_t thread_id;          // %fs:0x30
    uint64_t run_split;         // %fs:0x38; timer-split of when the process began its most recent timeslice; when it finishes, the delta to the current time is added to the run time counter
    uint64_t run_time;          // %fs:0x40
    fx_state fxsv;              // %fs:0x48 - %fs:0x0248; saved floating-point state
} __pack tcb_t;
typedef struct __tls_store_t
{
    uintptr_t __fs_baseptr;     // %fs:0x00; this will be a self-pointer
    tcb_t __tcb;                // %fs:0x08 - %fs:0x0248; the first entry will be the value of the fs_base for the next task
    size_t __data_size;         // %fs:0x0250; the size of the following array
    uint8_t __tl_data[];        // %fs:0x0258 - ???; will include thread-local variables like errno (eventually)
} __pack tls_t;
#ifdef __cplusplus
}
#endif
#endif