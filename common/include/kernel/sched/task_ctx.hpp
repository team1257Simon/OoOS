#ifndef __TASK_CTX
#define __TASK_CTX
#include "kernel/sched/task.h"
#include "kernel/kernel_mm.hpp"
#include "kernel/fs/fs.hpp"
#include "kernel/shared_object_map.hpp"
#include "sys/times.h"
#include "compare"
#include "vector"
#include "array"
extern "C" [[noreturn]] void handle_exit();
enum class execution_state
{
    STOPPED     = 0,
    RUNNING     = 1,
    TERMINATED  = 2  
};
struct task_ctx
{
    task_t task_struct;                     //  The c-style struct from task.h; gs base will point here when the task is active
    std::vector<task_ctx*> child_tasks{};   //  The array in task_struct will be redirected here.
    std::vector<const char*> arg_vec;       //  Argv will be this vector's data() member; argc is its size()
    std::vector<const char*> env_vec{};     //  Environment variables will go here
    std::vector<std::string> dl_search_paths{};
    addr_t allocated_stack;
    size_t stack_allocated_size;
    addr_t tls;
    size_t tls_size;
    filesystem* ctx_filesystem;
    file_node* stdio_ptrs[3]{};
    execution_state current_state{ execution_state::STOPPED };
    int exit_code{ 0 };
    addr_t exit_target{ nullptr };
    addr_t notif_target{ nullptr };
    task_ctx* last_notified{ nullptr };
    elf64_object* object_handle{ nullptr };
    shared_object_map* local_so_map{ nullptr };
    task_ctx(task_functor task, std::vector<const char*>&& args, addr_t stack_base, ptrdiff_t stack_size, addr_t tls_base, size_t tls_len, addr_t frame_ptr, uint64_t pid, int64_t parent_pid, priority_val prio, uint16_t quantum);
    task_ctx(elf64_program_descriptor const& desc, std::vector<const char*>&& args, uint64_t pid, int64_t parent_pid, priority_val prio, uint16_t quantum);
    ~task_ctx(); 
    constexpr uint64_t get_pid() const noexcept { return task_struct.task_ctl.task_id; }
    constexpr int64_t get_parent_pid() const noexcept { return task_struct.task_ctl.parent_pid; }
    constexpr bool is_system() const noexcept { return *static_cast<uint64_t*>(task_struct.frame_ptr) == kframe_magic; }
    constexpr bool is_user() const noexcept { return *static_cast<uint64_t*>(task_struct.frame_ptr) == uframe_magic; }
    friend constexpr std::strong_ordering operator<=>(task_ctx const& __this, task_ctx const& __that) noexcept { return __this.get_pid() <=> __that.get_pid(); }
    friend constexpr std::strong_ordering operator<=>(task_ctx const& __this, uint64_t __that) noexcept { return __this.get_pid() <=> __that; }
    friend constexpr std::strong_ordering operator<=>(uint64_t __this, task_ctx const& __that) noexcept { return __this <=> __that.get_pid(); }
    friend constexpr bool operator==(task_ctx const& __this, task_ctx const& __that) noexcept { return __this.task_struct.self == __that.task_struct.self; }
    void set_stdio_ptrs(std::array<file_node*, 3>&& ptrs);
    void set_stdio_ptrs(file_node* ptrs[3]);
    filesystem* get_vfs_ptr();
    void add_child(task_ctx* that);
    bool remove_child(task_ctx* that);
    void start_task(addr_t exit_fn = addr_t{ &handle_exit });
    void set_exit(int n);
    void terminate();
    tms get_times() const noexcept;
    void init_task_state();
    void init_task_state(task_ctx const& that);
} __align(16);
file_node* get_by_fd(filesystem* fsptr, task_ctx* ctx, int fd);
inline task_ctx* active_task_context() { return current_active_task()->self; }
void task_exec(elf64_program_descriptor const& prg, std::vector<const char*>&& args, std::vector<const char*>&& env, std::array<file_node*, 3>&& stdio_ptrs, addr_t exit_fn = nullptr, int64_t parent_pid = -1L, priority_val pv = priority_val::PVNORM, uint16_t quantum = 3);
extern "C"
{
    clock_t syscall_times(struct tms* out);
    long syscall_getpid();
    long syscall_fork();
    void syscall_exit(int n);
    int syscall_kill(unsigned long pid, unsigned long sig);
    pid_t syscall_wait(int* sc_out);
    int syscall_sleep(unsigned long seconds);
    int syscall_execve(char *name, char **argv, char **env);
}
#endif