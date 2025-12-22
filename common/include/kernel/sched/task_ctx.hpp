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
}
typedef std::vector<const char*> cstrvec;
constexpr uint16_t user_code	= 0x23US;
constexpr uint16_t user_data	= 0x1BUS;
constexpr uid_t uid_undef		= static_cast<uid_t>(-1);
constexpr gid_t gid_undef		= static_cast<gid_t>(-1);
struct task_descriptor
{
	elf64_program_descriptor const& program;
	cstrvec argv;
	spid_t parent_pid;
	uid_t uid;
	gid_t gid;
	uint16_t quantum;
	priority_val priority;
};
struct task_ctx
{
	task_t task_struct;												//  The c-style struct from task.h; the GS base will point here when the task is active
	std::vector<task_ctx*> child_tasks						{};		//  The array in task_struct will point to this vector's data() member
	cstrvec arg_vec;												//  Argv will be taken from this vector's data() member; argc is its size()
	cstrvec env_vec											{};		//  Environment variables will go here
	std::vector<std::string> dl_search_paths				{};		//  Cache of the dynamic linker search paths for this task's program image, if any
	std::vector<elf64_shared_object*> attached_so_handles	{};		//  Cache of the SO handles attached to this task, if any
	std::vector<elf64_dynamic_object*> tls_modules			{};		//	Handles to all objects with TLS blocks, if any, attached to the task; order is same as dtv
	addr_t entry;													//	Address of the entry point of the task
	addr_t allocated_stack;
	size_t stack_allocated_size;
	filesystem* ctx_filesystem;
	file_vnode* stdio_ptrs[3]								{};		//	The stdio file descriptors (stdin, stdout, stderr), in order, for quick access
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
	pid_t next_assigned_thread_id							{};
	thread_t* sigret_thread									{};
	std::map<pid_t, thread_t*> thread_ptr_by_id				{};
	std::map<pid_t, std::vector<thread_t*>> notify_threads	{};
	std::vector<thread_t*> inactive_threads					{};
	size_t active_added_thread_count						{};		// Active threads excluding the initial (thread 0)
	bool impersonate										{};
	uid_t imp_uid											{};		// Effective UID if it is different from the real UID in task_struct; 0 otherwise
	gid_t imp_gid											{};		// Effective GID if it is different from the real GID in task_struct; 0 otherwise
	constexpr task_t* header() { return std::addressof(task_struct); }
	constexpr pid_t get_pid() const noexcept { return task_struct.task_ctl.task_pid; }
	constexpr spid_t get_parent_pid() const noexcept { return task_struct.task_ctl.parent_pid; }
	constexpr uframe_tag& get_frame() const noexcept { return task_struct.frame_ptr.deref<uframe_tag>(); }
	constexpr uid_t uid() const noexcept { return task_struct.task_ctl.task_uid; }
	constexpr task_ctx& uid(uid_t id) noexcept { task_struct.task_ctl.task_uid = id; return *this; }
	constexpr gid_t gid() const noexcept { return task_struct.task_ctl.task_gid; }
	constexpr task_ctx& gid(gid_t id) noexcept { task_struct.task_ctl.task_gid = id; return *this; }
	constexpr uid_t euid() const noexcept { return impersonate ? imp_uid : uid(); }
	constexpr task_ctx& euid(uid_t id) noexcept { (impersonate ? imp_uid : task_struct.task_ctl.task_uid) = id; return *this; }
	constexpr gid_t egid() const noexcept { return impersonate ? imp_gid : gid(); }
	constexpr task_ctx& egid(gid_t id) noexcept { (impersonate ? imp_gid : task_struct.task_ctl.task_gid) = id; return *this; }
	friend constexpr std::strong_ordering operator<=>(task_ctx const& __this, task_ctx const& __that) noexcept { return __this.get_pid() <=> __that.get_pid(); }
	friend constexpr std::strong_ordering operator<=>(task_ctx const& __this, pid_t __that) noexcept { return __this.get_pid() <=> __that; }
	friend constexpr std::strong_ordering operator<=>(pid_t __this, task_ctx const& __that) noexcept { return __this <=> __that.get_pid(); }
	friend constexpr bool operator==(task_ctx const& __this, task_ctx const& __that) noexcept { return __this.task_struct.self == __that.task_struct.self; }
	task_ctx(task_descriptor&& desc, pid_t pid);
	task_ctx(task_ctx const& that);					//	nontrivial copy constructor for use in vfork()
	task_ctx(task_ctx&& that);						//	move constructor used by the task manager
	~task_ctx();
	elf64_dynamic_object* assert_dynamic();
	void set_stdio_ptrs(std::array<file_vnode*, 3>&& ptrs);
	void set_stdio_ptrs(file_vnode* ptrs[3]);
	filesystem* get_vfs_ptr();
	void add_child(task_ctx* that);
	bool remove_child(task_ctx* that);
	void start_task(addr_t exit_fn);
	void start_task();
	void restart_task(addr_t exit_fn);
	void restart_task();
	void terminate();
	void attach_object(elf64_object* obj, bool is_init);
	tms get_times() const noexcept;
	void init_task_state();
	void set_arg_registers(register_t rdi, register_t rsi, register_t rdx);
	void stack_push(register_t val);
	register_t stack_pop();
	void end_signal();
	void tls_assemble();
	void init_thread_0();
	void thread_switch(pid_t to_thread);
	thread_t* current_thread_ptr();
	join_result thread_join(pid_t with_thread);
	int thread_detach(pid_t thread_id);
	pid_t thread_fork();
	pid_t thread_add(addr_t entry_point, addr_t exit_point, size_t stack_target_size, bool start_detached, register_t arg);
	void thread_exit(pid_t thread_id, register_t result_val);
	thread_t* thread_init(thread_t const& current_thread, bool copy_all_regs, size_t stack_target_size = 0UZ, bool start_detached = false);
	void set_exit(int n);																// implements exit()
	void set_signal(int sig, bool save_state);											// implements raise()
	bool set_fork();																	// implements fork()
	bool subsume(elf64_program_descriptor const& desc, cstrvec&& args, cstrvec&& env);	// implements exec() variants
	addr_t tls_get(size_t mod_idx, size_t offs);										// implements __tls_get_addr()
} __align(16);
file_vnode* get_by_fd(filesystem* fsptr, task_ctx* ctx, int fd);
// Task struct base when in ISRs. In syscalls, use current_active_task instead
inline task_t* get_task_base() { task_t* gsb; asm volatile("movq %%gs:0x000, %0" : "=r"(gsb) :: "memory"); return gsb; }
// Task struct base when in syscalls. In ISRs, use get_task_base instead
inline task_t* current_active_task() { task_t* gsb; asm volatile("movq %%gs:0x000, %0" : "=r"(gsb) :: "memory"); return gsb->next; }
// Shortcut because this gets used a lot
inline task_ctx* active_task_context() { task_t* task = current_active_task(); if(task && task->frame_ptr.deref<uint64_t>() == uframe_magic) return reinterpret_cast<task_ctx*>(task); else return nullptr; }
// Shortcut because this also gets used a lot
inline addr_t active_frame() { return current_active_task()->frame_ptr; }
// Shortcut because I said so
inline void write_thread_ptr(addr_t value) { asm volatile("wrfsbase %0" :: "r"(value.full) : "memory"); }
extern "C"
{
	[[noreturn]] void handle_exit();
	[[noreturn]] void handle_thread_exit(thread_t* thread_ptr, register_t retval);
	[[noreturn]] void fallthrough_reentry(task_t*);
	void signal_exit(int code);
	void sigtramp_enter(int sig, signal_handler handler);
	void sigtramp_return();
	clock_t syscall_times(struct tms* out);																		// clock_t times(struct tms* out);
	spid_t syscall_getpid();																					// pid_t getpid();
	spid_t syscall_fork();																						// pid_t fork();
	spid_t syscall_vfork();																						// pid_t vfork();
	void syscall_exit(int n);																					// void exit(int code) __attribute__((noreturn));
	int syscall_kill(long pid, unsigned long sig);																// int kill(pid_t pid, int sig);
	pid_t syscall_wait(int* sc_out);																			// pid_t wait(int* sc_out);
	int syscall_sleep(unsigned long seconds);																	// int sleep(time_t seconds);
	int syscall_execve(char* restrict name, char** restrict argv, char** restrict env);							// int execve(char* restrict name, char** restrict argv, char** restrict env);
	spid_t syscall_spawn(char* restrict name, char** restrict argv, char** restrict env);						// pid_t spawn(char* restrict name, char** restrict argv, char** restrict env);
	signal_handler syscall_signal(int sig, signal_handler new_handler);											// int (*signal(int sig, void(*new_handler)(int)))(int);
	int syscall_raise(int sig);																					// int raise(int sig);
	int syscall_sigprocmask(sigprocmask_action how, sigset_t const* restrict set, sigset_t* restrict oset);		// int sigprocmask(int how, sigset_t const* restrict set, sigset_t* restrict oset);
	long syscall_sigret();																						// (only called from the signal trampoline)
	void force_signal(task_ctx* task, int8_t sig);																// (only called by the system on invalid syscalls or hardware exceptions)
	int syscall_tlinit();																						// int tlinit(); used to initialize TLS for dynamic objects
	addr_t syscall_tlget(tls_index* idx);																		// void* tlget(tls_index* idx); implements __tls_get_addr via LD-SO
	int syscall_fexecve(int fd, char** restrict argv, char** restrict env);										// int fexecve(int fd, char** restrict argv, char** restrict env);
	spid_t syscall_fspawn(int fd, char** restrict argv, char** restrict env);									// pid_t fspawn(int fd, char** restrict argv, char** restrict env);
}
#endif