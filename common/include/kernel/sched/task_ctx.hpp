#ifndef __TASK_CTX
#define __TASK_CTX
#include "kernel/kernel_mm.hpp"
#include "kernel/sched/task.h"
#include "kernel/fs/fs.hpp"
#include "kernel/shared_object_map.hpp"
#include "kernel/elf64_exec.hpp"
#include "sys/times.h"
#include "fs/posix_dirent.hpp"
#include "map"
#include "compare"
#include "vector"
#include "array"
typedef int (attribute(sysv_abi) task_closure)(int, char**);
typedef decltype(std::addressof(std::declval<task_closure>())) task_functor;
extern "C"
{
    void user_entry(addr_t);
    [[noreturn]] void kernel_reentry();
    void init_pit();
    void init_tss(addr_t k_rsp);
}
enum class execution_state
{
    STOPPED     = 0,
    RUNNING     = 1,
    TERMINATED  = 2,
    IN_DYN_EXIT = 3
};
struct task_ctx
{
    task_t task_struct;                                             //  The c-style struct from task.h; gs base will point here when the task is active
    std::vector<task_ctx*> child_tasks                      {};     //  The array in task_struct will be redirected here.
    std::vector<const char*> arg_vec;                               //  Argv will be taken from this vector's data() member; argc is its size()
    std::vector<const char*> env_vec                        {};     //  Environment variables will go here
    std::vector<std::string> dl_search_paths                {};
    std::vector<elf64_shared_object*> attached_so_handles   {};
    addr_t entry;
    addr_t allocated_stack;
    size_t stack_allocated_size;
    addr_t tls;
    size_t tls_size;
    filesystem* ctx_filesystem;
    file_node* stdio_ptrs[3]                                {};
    execution_state current_state                           { execution_state::STOPPED };
    int exit_code                                           { 0 };
    addr_t exit_target                                      { nullptr };
    addr_t dynamic_exit                                     { nullptr };
    addr_t notif_target                                     { nullptr };
    task_ctx* last_notified                                 { nullptr };
    elf64_executable* program_handle                        { nullptr };
    shared_object_map* local_so_map                         { nullptr };
    addr_t rt_argv_ptr                                      { nullptr };
    addr_t rt_env_ptr                                       { nullptr };
    task_signal_info_t task_sig_info                        {};
    std::map<int, posix_directory> opened_directories       {};
    task_ctx(task_functor task, std::vector<const char*>&& args, addr_t stack_base, ptrdiff_t stack_size, addr_t tls_base, size_t tls_len, addr_t frame_ptr, uint64_t pid, int64_t parent_pid, priority_val prio, uint16_t quantum);
    task_ctx(elf64_program_descriptor const& desc, std::vector<const char*>&& args, uint64_t pid, int64_t parent_pid, priority_val prio, uint16_t quantum);
    task_ctx(task_ctx const& that);
    task_ctx(task_ctx&& that);
    ~task_ctx();
    constexpr uint64_t get_pid() const noexcept { return task_struct.task_ctl.task_id; }
    constexpr int64_t get_parent_pid() const noexcept { return task_struct.task_ctl.parent_pid; }
    constexpr void change_pid(uint64_t pid, int64_t parent_pid) noexcept { task_struct.task_ctl.parent_pid = parent_pid; task_struct.task_ctl.task_id = pid; }
    constexpr bool is_system() const noexcept { return *static_cast<uint64_t*>(task_struct.frame_ptr) == kframe_magic; }
    constexpr bool is_user() const noexcept { return *static_cast<uint64_t*>(task_struct.frame_ptr) == uframe_magic; }
    constexpr static uint16_t code_segment(uint64_t fmagic) noexcept { return fmagic == kframe_magic ? 0x08 : 0x23; }
    constexpr static uint16_t data_segment(uint64_t fmagic) noexcept { return fmagic == kframe_magic ? 0x10 : 0x1B; }
    friend constexpr std::strong_ordering operator<=>(task_ctx const& __this, task_ctx const& __that) noexcept { return __this.get_pid() <=> __that.get_pid(); }
    friend constexpr std::strong_ordering operator<=>(task_ctx const& __this, uint64_t __that) noexcept { return __this.get_pid() <=> __that; }
    friend constexpr std::strong_ordering operator<=>(uint64_t __this, task_ctx const& __that) noexcept { return __this <=> __that.get_pid(); }
    friend constexpr bool operator==(task_ctx const& __this, task_ctx const& __that) noexcept { return __this.task_struct.self == __that.task_struct.self; }
    void set_stdio_ptrs(std::array<file_node*, 3>&& ptrs);
    void set_stdio_ptrs(file_node* ptrs[3]);
    filesystem* get_vfs_ptr();
    void add_child(task_ctx* that);
    bool remove_child(task_ctx* that);
    void start_task(addr_t exit_fn);
    void start_task();
    void restart_task(addr_t exit_fn);
    void restart_task();
    void set_exit(int n);
    void terminate();
    void attach_object(elf64_object* obj);
    tms get_times() const noexcept;
    void init_task_state();
    void set_arg_registers(register_t rdi, register_t rsi, register_t rdx);
    void stack_push(register_t val);
    register_t stack_pop();
    void set_signal(int sig, bool save_state);
    register_t end_signal();
    bool set_fork();
    bool subsume(elf64_program_descriptor const& desc, std::vector<const char*>&& args, std::vector<const char*>&& env);
} __align(16);
file_node* get_by_fd(filesystem* fsptr, task_ctx* ctx, int fd);
inline task_t* current_active_task() { task_t* gsb; asm volatile("movq %%gs:0x000, %0" : "=r"(gsb) :: "memory"); return gsb->next; }
inline task_ctx* active_task_context() { return current_active_task()->self; }
inline addr_t active_frame() { return current_active_task()->frame_ptr; }
void task_exec(elf64_program_descriptor const& prg, std::vector<const char*>&& args, std::vector<const char*>&& env, std::array<file_node*, 3>&& stdio_ptrs, addr_t exit_fn = nullptr, int64_t parent_pid = -1L, priority_val pv = priority_val::PVNORM, uint16_t quantum = 3);
extern "C"
{
    [[noreturn]] void handle_exit();
    [[noreturn]] void fallthrough_reentry(task_t*);
    [[noreturn]] void user_reentry();
    void signal_exit(int code);
    void sigtramp_enter(int sig, signal_handler handler);
    void sigtramp_return();
    clock_t syscall_times(struct tms* out);                                                                     // clock_t times(struct tms* out);
    long syscall_getpid();                                                                                      // pid_t getpid();
    long syscall_fork();                                                                                        // pid_t fork();
    long syscall_vfork();                                                                                       // pid_t vfork();
    void syscall_exit(int n);                                                                                   // void exit(int code) __attribute__((noreturn));
    int syscall_kill(long pid, unsigned long sig);                                                              // int kill(pid_t pid, int sig);
    pid_t syscall_wait(int* sc_out);                                                                            // pid_t wait(int* sc_out);
    int syscall_sleep(unsigned long seconds);                                                                   // int sleep(time_t seconds);
    int syscall_execve(char* restrict name, char** restrict argv, char** restrict env);                         // int execve(char* restrict name, char* restrict* restrict argv, char* restrict* restrict env);
    long syscall_spawn(char* restrict name, char** restrict argv, char** restrict env);                         // pid_t spawn(char* restrict name, char* restrict* restrict argv, char* restrict* restrict env);
    long syscall_sigret();                                                                                      // (only called from the signal trampoline)
    signal_handler syscall_signal(int sig, signal_handler new_handler);                                         // int (*signal(int sig, void(*new_handler)(int)))(int);
    int syscall_raise(int sig);                                                                                 // int raise(int sig);
    void force_signal(task_ctx* task, int8_t sig);                                                              // (only called by the system on invalid syscalls or hardware exceptions)
    int syscall_sigprocmask(sigprocmask_action how, sigset_t const* restrict set, sigset_t* restrict oset);     // int sigprocmask(int how, sigset_t const* restrict set, sigset_t* restrict oset);
}
#endif