#ifndef __TASK_CTX
#define __TASK_CTX
#include <fs/fs.hpp>
#include <fs/posix_dirent.hpp>
#include <sched/thread.hpp>
#include <sys/times.h>
#include <array>
#include <compare>
#include <elf64_exec.hpp>
#include <map>
#include <shared_object_map.hpp>
extern "C"
{
	void user_entry(addr_t);
	[[noreturn]] void kernel_reentry();
	void init_tss(addr_t k_rsp);
}
constexpr uint16_t user_code	= 0x23US;
constexpr uint16_t user_data	= 0x1BUS;
struct task_ctx
{
	task_t task_struct;                                             //  The c-style struct from task.h; the GS base will point here when the task is active
	std::vector<task_ctx*> child_tasks						{};     //  The array in task_struct will point to this vector's data() member
	std::vector<const char*> arg_vec;                               //  Argv will be taken from this vector's data() member; argc is its size()
	std::vector<const char*> env_vec						{};     //  Environment variables will go here
	std::vector<std::string> dl_search_paths				{};     //  Cache of the dynamic linker search paths for this task's program image, if any
	std::vector<elf64_shared_object*> attached_so_handles	{};     //  Cache of the SO handles attached to this task, if any
	std::vector<elf64_dynamic_object*> tls_modules			{};
	addr_t entry;
	addr_t allocated_stack;
	size_t stack_allocated_size;
	filesystem* ctx_filesystem;
	file_vnode* stdio_ptrs[3]								{};
	execution_state current_state;
	int exit_code											{};
	addr_t exit_target										{};
	addr_t dynamic_exit										{};
	addr_t notif_target										{};
	task_ctx* last_notified									{};
	elf64_executable* program_handle						{};
	shared_object_map* local_so_map							{};
	addr_t rt_argv_ptr										{};
	addr_t rt_env_ptr										{};
	task_signal_info_t task_sig_info						{};
	std::map<int, posix_directory> opened_directories		{};
	ooos::task_dtv dyn_thread								{};
	std::map<uint32_t, thread_t*> thread_ptr_by_id			{};
	uint32_t next_thread_id									{};
	constexpr pid_t get_pid() const noexcept { return task_struct.task_ctl.task_id; }
	constexpr spid_t get_parent_pid() const noexcept { return task_struct.task_ctl.parent_pid; }
	constexpr void change_pid(pid_t pid, spid_t parent_pid) noexcept { task_struct.task_ctl.parent_pid = parent_pid; task_struct.task_ctl.task_id = pid; }
	friend constexpr std::strong_ordering operator<=>(task_ctx const& __this, task_ctx const& __that) noexcept { return __this.get_pid() <=> __that.get_pid(); }
	friend constexpr std::strong_ordering operator<=>(task_ctx const& __this, pid_t __that) noexcept { return __this.get_pid() <=> __that; }
	friend constexpr std::strong_ordering operator<=>(pid_t __this, task_ctx const& __that) noexcept { return __this <=> __that.get_pid(); }
	friend constexpr bool operator==(task_ctx const& __this, task_ctx const& __that) noexcept { return __this.task_struct.self == __that.task_struct.self; }
	constexpr task_t* header() { return std::addressof(task_struct); }
	task_ctx(elf64_program_descriptor const& desc, std::vector<const char*>&& args, pid_t pid, spid_t parent_pid, priority_val prio, uint16_t quantum);
	task_ctx(task_ctx const& that);         // implements vfork()
	task_ctx(task_ctx&& that);
	~task_ctx();
	void set_stdio_ptrs(std::array<file_vnode*, 3>&& ptrs);
	void set_stdio_ptrs(file_vnode* ptrs[3]);
	filesystem* get_vfs_ptr();
	void add_child(task_ctx* that);
	bool remove_child(task_ctx* that);
	void start_task(addr_t exit_fn);
	void start_task();
	void restart_task(addr_t exit_fn);
	void restart_task();
	void set_exit(int n);                       // implements exit()
	void terminate();
	void attach_object(elf64_object* obj, bool is_init);
	tms get_times() const noexcept;
	void init_task_state();
	void set_arg_registers(register_t rdi, register_t rsi, register_t rdx);
	void stack_push(register_t val);
	register_t stack_pop();
	void set_signal(int sig, bool save_state);  // implements raise()
	register_t end_signal();
	bool set_fork();                            // implements fork()
	bool subsume(elf64_program_descriptor const& desc, std::vector<const char*>&& args, std::vector<const char*>&& env);    // implements execve()
	void tls_assemble();
	void init_thread_0();
	void thread_switch(uint32_t to_thread);
	addr_t tls_get(size_t mod_idx, size_t offs);	// implements __tls_get_addr()
	elf64_dynamic_object* assert_dynamic();
} __align(16);
file_vnode* get_by_fd(filesystem* fsptr, task_ctx* ctx, int fd);
// Task struct base when in ISRs. In syscalls, use current_active_task instead
inline task_t* get_task_base() { task_t* gsb; asm volatile("movq %%gs:0x000, %0" : "=r"(gsb) :: "memory"); return gsb; }
// Task struct base when in syscalls. In ISRs, use get_task_base instead
inline task_t* current_active_task() { task_t* gsb; asm volatile("movq %%gs:0x000, %0" : "=r"(gsb) :: "memory"); return gsb->next; }
// Shortcut because this gets used a lot
inline task_ctx* active_task_context() { return reinterpret_cast<task_ctx*>(current_active_task()); }
// Shortcut because this also gets used a lot
inline addr_t active_frame() { return current_active_task()->frame_ptr; }
void task_exec(elf64_program_descriptor const& prg, std::vector<const char*>&& args, std::vector<const char*>&& env, std::array<file_vnode*, 3>&& stdio_ptrs, addr_t exit_fn = nullptr, int64_t parent_pid = -1L, priority_val pv = priority_val::PVNORM, uint16_t quantum = 3);
extern "C"
{
	[[noreturn]] void handle_exit();
	[[noreturn]] void fallthrough_reentry(task_t*);
	void signal_exit(int code);
	void sigtramp_enter(int sig, signal_handler handler);
	void sigtramp_return();
	clock_t syscall_times(struct tms* out);                                                                     // clock_t times(struct tms* out);
	spid_t syscall_getpid();                                                                                    // pid_t getpid();
	spid_t syscall_fork();                                                                                      // pid_t fork();
	spid_t syscall_vfork();                                                                                     // pid_t vfork();
	void syscall_exit(int n);                                                                                   // void exit(int code) __attribute__((noreturn));
	int syscall_kill(long pid, unsigned long sig);                                                              // int kill(pid_t pid, int sig);
	pid_t syscall_wait(int* sc_out);                                                                            // pid_t wait(int* sc_out);
	int syscall_sleep(unsigned long seconds);                                                                   // int sleep(time_t seconds);
	int syscall_execve(char* restrict name, char** restrict argv, char** restrict env);                         // int execve(char* restrict name, char** restrict argv, char* restrict* restrict env);
	spid_t syscall_spawn(char* restrict name, char** restrict argv, char** restrict env);                       // pid_t spawn(char* restrict name, char** restrict argv, char* restrict* restrict env);
	signal_handler syscall_signal(int sig, signal_handler new_handler);                                         // int (*signal(int sig, void(*new_handler)(int)))(int);
	int syscall_raise(int sig);                                                                                 // int raise(int sig);
	int syscall_sigprocmask(sigprocmask_action how, sigset_t const* restrict set, sigset_t* restrict oset);     // int sigprocmask(int how, sigset_t const* restrict set, sigset_t* restrict oset);
	long syscall_sigret();                                                                                      // (only called from the signal trampoline)
	void force_signal(task_ctx* task, int8_t sig);                                                              // (only called by the system on invalid syscalls or hardware exceptions)
	int syscall_tlinit();																						// int tlinit(); used to initialize TLS for dynamic objects
	addr_t syscall_tlget(tls_index* idx);																		// void* tlget(tls_index* idx); implements __tls_get_addr via LD-SO
}
#endif