typedef unsigned long sigset_t;
extern int* __errno(void);
int sigemptyset(sigset_t* set) { if(set) *set = 0UL; else { *__errno() = 22; return -1; } return 0; }
int sigfillset(sigset_t* set) { if(set) *set = ~0UL; else { *__errno() = 22; return -1; } return 0; }
int sigaddset(sigset_t* set, int sig) { if(set) *set |= (1 << sig); else { *__errno() = 22; return -1; } return 0; }
int sigdelset(sigset_t* set, int sig) { if(set) *set &= ~(1 << sig); else { *__errno() = 22; return -1; } return 0; }
int sigismember(sigset_t const* set, int sig) { if(set) return (*set & (1 << sig)) ? 1 : 0; *__errno() = 22; return -1; }
int raise(int sig)
{ 
    int retval;
    asm volatile("syscall" : "=a"(retval) : "0"(37), "D"(sig) : "memory", "%rcx", "%r11");
    if(retval < 0 && retval > -4096) { *__errno() = -retval; return -1; }
    return 0;
}