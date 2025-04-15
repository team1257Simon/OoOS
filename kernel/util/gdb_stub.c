/****************************************************************************
*  Header: remcom.c,v 1.34 91/03/09 12:29:49 glenne Exp $
*
*  Module name: remcom.c
*  Revision: 1.34
*  Date: 91/03/09 12:29:49
*  Contributor:     Lake Stevens Instrument Division
*
*  Description:     low level support for gdb debugger.
*
*  Considerations:  only works on target hardware
*
*  Written by:      Glenn Engel
*  ModuleState:     Experimental
*
*  NOTES:           See Below
*
*  Modified for 386 by Jim Kingdon, Cygnus Support.
*
*  To enable debugger support, two things need to happen.  One, a
*  call to set_debug_traps() is necessary in order to allow any breakpoints
*  or error conditions to be properly intercepted and reported to gdb.
*  Two, a breakpoint needs to be generated to begin communication.  This
*  is most easily accomplished by a call to breakpoint().  Breakpoint()
*  simulates a breakpoint by executing a trap #1.
*
*  The external function exception_handler() is
*  used to attach a specific handler to a specific 386 vector number.
*  It should use the same privilege level it runs at.  It should
*  install it as an interrupt gate so that interrupts are masked
*  while the handler runs.
*
*  Because gdb will sometimes write to the stack area to execute function
*  calls, this program cannot rely on using the supervisor stack so it
*  uses its own stack area reserved in the int array remcom_stack.
*
*************
*
*    The following gdb commands are supported:
*
* command          function                               Return value
*
*    g             return the value of the CPU registers  hex data or ENN
*    G             set the value of the CPU registers     OK or ENN
*
*    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
*    MAA..AA,LLLL: Write LLLL bytes at address AA.AA      OK or ENN
*
*    c             Resume at current address              SNN   ( signal NN)
*    cAA..AA       Continue at address AA..AA             SNN
*
*    s             Step one instruction                   SNN
*    sAA..AA       Step one instruction from AA..AA       SNN
*
*    k             kill
*
*    ?             What was the last sigval ?             SNN   (signal NN)
*
* All commands and responses are sent with a packet which includes a
* checksum.  A packet consists of
*
* $<packet info>#<checksum>.
*
* where
* <packet info> :: <characters representing the command or response>
* <checksum>    :: < two hex digits computed as modulo 256 sum of <packetinfo>>
*
* When a packet is received, it is first acknowledged with either '+' or '-'.
* '+' indicates a successful transfer.  '-' indicates a failed transfer.
*
* Example:
*
* Host:                  Reply:
* $m0,10#2a               +$00010203040506070809101112131415#42
*
****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "sched/task.h"
extern void put_debug_char(int);	        /* write a single character      */
extern int get_debug_char();	            /* read and return a single char */
extern void exception_handler(int, void*);	/* assign an exception handler   */
#define BUFMAX 400
static char initialized;  /* boolean flag. != 0 means we've been initialized */
int remote_debug;
/*  debug >  0 prints ill-formed commands in valid packets & checksum errors */
static const char hexchars[] = "0123456789ABCDEF";
/* Number of full registers.  */
#define NUMREGS	18
/* Number of bytes of registers. */
#define NUMREGBYTES ((NUMREGS * 8) + 6)
enum regnames
{
	EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI,
	PC /* also known as eip */,
	PS /* also known as eflags */,
	CS, SS, DS
};
regstate_t registers;
#define STACKSIZE 10000
long remcom_stack[STACKSIZE/sizeof(int)];
long* stack_ptr = &remcom_stack[STACKSIZE/sizeof(long) - 1];
/***************************  ASSEMBLY CODE MACROS *************************/
attribute(naked) void return_to_prog()
{
    asm volatile("movq %0, %%rsp" :: "m"(registers.rsp) : "memory");
    asm volatile("movq %0, %%rcx" :: "m"(registers.rcx) : "memory");
    asm volatile("movq %0, %%rdx" :: "m"(registers.rdx) : "memory");
    asm volatile("movq %0, %%rbx" :: "m"(registers.rbx) : "memory");
    asm volatile("movq %0, %%rbp" :: "m"(registers.rbp) : "memory");
    asm volatile("movq %0, %%rsi" :: "m"(registers.rsi) : "memory");
    asm volatile("movq %0, %%rdi" :: "m"(registers.rdi) : "memory");
    asm volatile("xorq %%rax, %%rax" ::: "memory");
    asm volatile("movw %0, %%ax"  :: "m"(registers.ds) : "memory");
    asm volatile("movw %%ax, %%ds" ::: "memory");
    asm volatile("movw %0, %%ax"  :: "m"(registers.ss) : "memory");
    asm volatile("movw %%ax, %%ss" ::: "memory");
    asm volatile("movq %0, %%rax"  :: "m"(registers.rflags) : "memory");
    asm volatile("push %%rax" ::: "memory");
    asm volatile("xorq %%rax, %%rax" ::: "memory");
    asm volatile("movw %0, %%ax" :: "m"(registers.cs) : "memory");
    asm volatile("push %%rax" ::: "memory");
    asm volatile("movq %0, %%rax" :: "m"(registers.rip) : "memory");
    asm volatile("push %%rax" ::: "memory");
    asm volatile("movq %0, %%rax" :: "m"(registers.rax) : "memory");
    asm volatile("iretq");
}
#define BREAKPOINT() asm volatile("int $3");
/* Put the error code here just in case the user cares. */
long gdb_i386errcode;
/* Likewise, the vector number here (since GDB only gets the signal number through the usual means, and that's not very specific). */
int gdb_i386vector = -1;
/* GDB stores segment registers in 32-bit words (that's just the way m-i386v.h is written).  So zero the appropriate areas in registers. */
#define SAVE_REGISTERS1() \
asm volatile("movq %%rax, %0" : "=m"(registers.rax) :: "memory"); \
asm volatile("movq %%rcx, %0" : "=m"(registers.rcx) :: "memory"); \
asm volatile("movq %%rdx, %0" : "=m"(registers.rdx) :: "memory"); \
asm volatile("movq %%rbx, %0" : "=m"(registers.rbx) :: "memory"); \
asm volatile("movq %%rbp, %0" : "=m"(registers.rbp) :: "memory"); \
asm volatile("movq %%rsi, %0" : "=m"(registers.rsi) :: "memory"); \
asm volatile("movq %%rdi, %0" : "=m"(registers.rdi) :: "memory"); \
asm volatile("movq %%r8, %0" : "=m"(registers.r8) :: "memory"); \
asm volatile("movq %%r9, %0" : "=m"(registers.r9) :: "memory"); \
asm volatile("movq %%r10, %0" : "=m"(registers.r10) :: "memory"); \
asm volatile("movq %%r11, %0" : "=m"(registers.r11) :: "memory"); \
asm volatile("movq %%r12, %0" : "=m"(registers.r12) :: "memory"); \
asm volatile("movq %%r13, %0" : "=m"(registers.r13) :: "memory"); \
asm volatile("movq %%r14, %0" : "=m"(registers.r14) :: "memory"); \
asm volatile("movq %%r15, %0" : "=m"(registers.r15) :: "memory"); \
asm volatile("movw %%ds, %%ax" ::: "memory"); \
asm volatile("movw %%ax, %0" : "=m"(registers.ds));
#define SAVE_ERRCODE() \
asm volatile("pop %rbx"); \
asm volatile("movq %rbx, gdb_i386errcode");
#define SAVE_REGISTERS2() \
asm volatile("pop %%rbx" ::: "memory"); /* old eip */ \
asm volatile("movq %%rbx, %0" : "=m"(registers.rip) :: "memory"); \
asm volatile("pop %%rbx" ::: "memory");	 /* old cs */ \
asm volatile("movw %%bx, %0" : "=m"(registers.cs) :: "memory"); \
asm volatile("pop %%rbx" ::: "memory");	 /* old eflags */ \
asm volatile("movq %%rbx, %0" : "=m"(registers.rflags) :: "memory"); \
/* Now that we've done the pops, we can save the stack pointer.");  */ \
asm volatile("movw %%ss, %%ax" ::: "memory"); \
asm volatile("movw %%ax, %0" : "=m"(registers.ss) :: "memory"); \
asm volatile("movq %%rsp, %0" : "=m"(registers.rsp) :: "memory");
/* See if mem_fault_routine is set, if so just IRET to that address.  */
#define CHECK_FAULT() \
asm volatile("cmpq $0, mem_fault_routine" ::: "memory"); \
asm volatile("jne mem_fault" ::: "memory");
attribute(naked) void mem_fault()
{
    /* Pop error code from the stack and save it.  */
    asm volatile("pop %%rax" ::: "memory");
    asm volatile("movq %%rax, gdb_i386errcode" ::: "memory");
    asm volatile("pop %%rax" ::: "memory"); /* eip */
    /* We don't want to return there, we want to return to the function pointed to by mem_fault_routine instead.  */
    asm volatile("movq mem_fault_routine, %%rax" ::: "memory");
    asm volatile("pop %%rcx" ::: "memory"); /* cs (low 16 bits; junk in hi 16 bits).  */
    asm volatile("pop %%rdx" ::: "memory"); /* eflags */
    /* Remove this stack frame; when we do the iret, we will be going to the start of a function, so we want the stack to look just like it would after a "call" instruction.  */
    asm volatile("leave" ::: "memory");
    /* Push the stuff that iret wants.  */
    asm volatile("push %%rdx" ::: "memory"); /* eflags */
    asm volatile("push %%rcx" ::: "memory"); /* cs */
    asm volatile("push %%rax" ::: "memory"); /* eip */
    /* Zero mem_fault_routine.  */
    asm volatile("xorq %%rax, %%rax" ::: "memory");
    asm volatile("movq %%rax, mem_fault_routine" ::: "memory");
    asm volatile("iretq");
	__builtin_unreachable();
}
#define CALL_HOOK() asm volatile("call remcom_handler");
#define DEF_CATCH_EXCEPT(n) \
attribute(naked) void catch_exception_##n() \
{ \
   SAVE_REGISTERS1(); \
   SAVE_REGISTERS2(); \
   asm volatile("push $"#n ::: "memory"); \
   CALL_HOOK(); \
   __builtin_unreachable(); \
}
#define DEF_CATCH_EXCEPT_WITH_ERRORCODE(n) \
attribute(naked) void catch_exception_##n() \
{ \
   SAVE_REGISTERS1(); \
   SAVE_ERRCODE(); \
   SAVE_REGISTERS2(); \
   asm volatile("push $"#n ::: "memory"); \
   CALL_HOOK(); \
   __builtin_unreachable(); \
}
#define DEF_CATCH_FAULT_WITH_ERRORCODE(n) \
attribute(naked) void catch_exception_##n() \
{ \
   CHECK_FAULT(); \
   SAVE_REGISTERS1(); \
   SAVE_ERRCODE(); \
   SAVE_REGISTERS2(); \
   asm volatile("push $"#n ::: "memory"); \
   CALL_HOOK(); \
   __builtin_unreachable(); \
}
/* This function is called when a i386 exception occurs.  It saves
* all the cpu regs in the registers array, munges the stack a bit,
* and invokes an exception handler (remcom_handler).
*
* stack on entry:                       stack on exit:
*   old eflags                          vector number
*   old cs (zero-filled to 32 bits)
*   old eip
*/
DEF_CATCH_EXCEPT(0)
DEF_CATCH_EXCEPT(1)
DEF_CATCH_EXCEPT(3)
DEF_CATCH_EXCEPT(4)
DEF_CATCH_EXCEPT(5)
DEF_CATCH_EXCEPT(6)
DEF_CATCH_EXCEPT(7)
DEF_CATCH_EXCEPT_WITH_ERRORCODE(8)
DEF_CATCH_EXCEPT(9)
DEF_CATCH_EXCEPT_WITH_ERRORCODE(10)
DEF_CATCH_FAULT_WITH_ERRORCODE(11)
DEF_CATCH_EXCEPT_WITH_ERRORCODE(12)
DEF_CATCH_FAULT_WITH_ERRORCODE(13)
DEF_CATCH_FAULT_WITH_ERRORCODE(14)
DEF_CATCH_EXCEPT(16)
/* remcom_handler is a front end for handle_exception.  It moves the stack pointer into an area reserved for debugger use. */
asm("remcom_handler:");
asm("pop %rax");             	 /* pop off return address     */
asm("pop %rax");            	 /* get the exception number   */
asm("movq stack_ptr, %rsp");     /* move to remcom stack area  */
asm("movq %rax, %rdi");	         /* pass exception number  */
asm("call  handle_exception");   /* this never returns */
void return_from_exception() { return_to_prog(); }
int hex(char ch)
{
	if((ch >= 'a') && (ch <= 'f')) return (ch - 'a' + 10);
	if((ch >= '0') && (ch <= '9')) return (ch - '0');
	if((ch >= 'A') && (ch <= 'F')) return (ch - 'A' + 10);
	return (-1);
}
static char remcom_in_buffer[BUFMAX];
static char remcom_out_buffer[BUFMAX];
/* scan for the sequence $<data>#<checksum> */
unsigned char* getpacket(void)
{
	unsigned char *buffer = (unsigned char*)&remcom_in_buffer[0];
	unsigned char checksum;
	unsigned char xmitcsum;
	int count;
	char ch;
	while(1)
	{
		/* wait around for the start character, ignore all other characters */
		while((ch = get_debug_char()) != '$');
	retry:
		checksum = 0;
		xmitcsum = -1;
		count = 0;
		/* now, read until a # or end of buffer is found */
		while(count < BUFMAX - 1)
		{
			ch = get_debug_char();
			if(ch == '$') goto retry;
			if(ch == '#') break;
			checksum = checksum + ch;
			buffer[count] = ch;
			count = count + 1;
		}
		buffer[count] = 0;
		if(ch == '#')
		{
			ch = get_debug_char();
			xmitcsum = hex(ch) << 4;
			ch = get_debug_char();
			xmitcsum += hex(ch);
			if(checksum != xmitcsum) { if(remote_debug) { fprintf(stderr, "bad checksum.  My count = 0x%x, sent=0x%x. buf=%s\n", checksum, xmitcsum, buffer); } put_debug_char('-');	/* failed checksum */ }
			else
			{
				put_debug_char('+'); /* successful transfer */
				/* if a sequence char is present, reply the sequence ID */
				if(buffer[2] == ':')
				{
					put_debug_char(buffer[0]);
					put_debug_char(buffer[1]);
					return &buffer[3];
				}
				return &buffer[0];
			}
		}
	}
}
/* send the packet in buffer. */
void putpacket(unsigned char* buffer)
{
	unsigned char checksum;
	int count;
	char ch;
	/*  $<packet info>#<checksum>. */
	do {
		put_debug_char('$');
		checksum = 0;
		count = 0;
        while((ch = buffer[count]))
        {
            put_debug_char(ch);
            checksum += ch;
            count += 1;
        }
        put_debug_char('#');
        put_debug_char(hexchars[checksum >> 4]);
        put_debug_char(hexchars[checksum % 16]);
	} while(get_debug_char() != '+');
}
void debug_error(char* format, char* parm) { if(remote_debug) fprintf(stderr, format, parm); }
/* Address of a routine to RTE to if we get a memory fault.  */
static void (*volatile mem_fault_routine)() = NULL;
/* Indicate to caller of mem2hex or hex2mem that there has been an error.  */
static volatile int mem_err = 0;
void set_mem_err(void) { mem_err = 1; }
/* These are separate functions so that they are so short and sweet that the compiler won't save any registers (if there is a fault to mem_fault, they won't get restored, so there better not be any saved).  */
int get_char(char* addr) { return *addr; }
void set_char(char* addr, int val) { *addr = val; }
/* convert the memory pointed to by mem into hex, placing result in buf */
/* return a pointer to the last char put in buf (null) */
/* If MAY_FAULT is non-zero, then we should set mem_err in response to a fault; if zero treat a fault like any other fault in the stub. */
char* mem2hex(char* mem, char* buf, int count, int may_fault)
{
	int i;
	unsigned char ch;
	if(may_fault) mem_fault_routine = set_mem_err;
	for (i = 0; i < count; i++)
	{
		ch = get_char(mem++);
		if(may_fault && mem_err) return (buf);
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch % 16];
	}
	*buf = 0;
	if(may_fault) mem_fault_routine = NULL;
	return (buf);
}
/* convert the hex array pointed to by buf into binary to be placed in mem */
/* return a pointer to the character AFTER the last byte written */
char* hex2mem(char* mem, char* buf, int count, int may_fault)
{
	int i;
	unsigned char ch;
	if(may_fault) mem_fault_routine = set_mem_err;
	for(i = 0; i < count; i++)
	{
		ch = hex(*buf++) << 4;
		ch = ch + hex(*buf++);
		set_char(mem++, ch);
		if(may_fault && mem_err) return (mem);
	}
	if(may_fault) mem_fault_routine = NULL;
	return (mem);
}
/* this function takes the 386 exception vector and attempts to translate this number into a unix compatible signal value */
int compute_signal(int exception_vector)
{
	int sigval;
	switch(exception_vector)
	{
	case 0:
		sigval = 8;
		break;			/* divide by zero */
	case 1:
		sigval = 5;
		break;			/* debug exception */
	case 3:
		sigval = 5;
		break;			/* breakpoint */
	case 4:
		sigval = 16;
		break;			/* into instruction (overflow) */
	case 5:
		sigval = 16;
		break;			/* bound instruction */
	case 6:
		sigval = 4;
		break;			/* Invalid opcode */
	case 7:
		sigval = 8;
		break;			/* coprocessor not available */
	case 8:
		sigval = 7;
		break;			/* double fault */
	case 9:
		sigval = 11;
		break;			/* coprocessor segment overrun */
	case 10:
		sigval = 11;
		break;			/* Invalid TSS */
	case 11:
		sigval = 11;
		break;			/* Segment not present */
	case 12:
		sigval = 11;
		break;			/* stack exception */
	case 13:
		sigval = 11;
		break;			/* general protection */
	case 14:
		sigval = 11;
		break;			/* page fault */
	case 16:
		sigval = 7;
		break;			/* coprocessor error */
	default:
		sigval = 7;		/* "software generated" */
	}
	return (sigval);
}
/**********************************************/
/* WHILE WE FIND NICE HEX CHARS, BUILD AN INT */
/* RETURN NUMBER OF CHARS PROCESSED           */
/**********************************************/
int hex_to_int(char** ptr, long int* int_value)
{
	int num_chars = 0;
	int hex_value;
	*int_value = 0;
	while(**ptr)
	{
		hex_value = hex(**ptr);
		if (hex_value >= 0) { *int_value = (*int_value << 4) | hex_value; num_chars++; }
		else break;
		(*ptr)++;
	}
	return (num_chars);
}
/*
* This function does all command procesing for interfacing to gdb.
*/
void handle_exception(int exception_vector)
{
	int sigval, stepping;
	long addr, length;
	char *ptr;
	 __attribute__((__unused__)) int new_pc;
	gdb_i386vector = exception_vector;
	if(remote_debug) { printf("vector=%d, sr=0x%x, pc=0x%x\n", exception_vector, registers.rflags, registers.rip); }
	/* reply to host that an exception has occurred */
	sigval = compute_signal(exception_vector);
	ptr = remcom_out_buffer;
	*ptr++ = 'T';			/* notify gdb with signo, PC, FP and SP */
	*ptr++ = hexchars[sigval >> 4];
	*ptr++ = hexchars[sigval & 0xf];
	*ptr++ = hexchars[ESP]; 
	*ptr++ = ':';
	ptr = mem2hex((char*)&registers.rsp, ptr, 4, 0);	/* SP */
	*ptr++ = ';';
	*ptr++ = hexchars[EBP]; 
	*ptr++ = ':';
	ptr = mem2hex((char*)&registers.rbp, ptr, 4, 0); 	/* FP */
	*ptr++ = ';';
	*ptr++ = hexchars[PC]; 
	*ptr++ = ':';
	ptr = mem2hex((char*)&registers.rip, ptr, 4, 0); 	/* PC */
	*ptr++ = ';';
	*ptr = '\0';
	putpacket((unsigned char*)remcom_out_buffer);
	stepping = 0;
	while(1 == 1)
	{
		remcom_out_buffer[0] = 0;
		ptr = (char*)getpacket();
		switch(*ptr++)
		{
		case '?':
			remcom_out_buffer[0] = 'S';
			remcom_out_buffer[1] = hexchars[sigval >> 4];
			remcom_out_buffer[2] = hexchars[sigval % 16];
			remcom_out_buffer[3] = 0;
			break;
		case 'd':
			remote_debug = !(remote_debug);	/* toggle debug flag */
			break;
		case 'g':		/* return the value of the CPU registers */
			mem2hex((char*)&registers, remcom_out_buffer, NUMREGBYTES, 0);
			break;
		case 'G':		/* set the value of the CPU registers - return OK */
			hex2mem(ptr, (char*)&registers, NUMREGBYTES, 0);
			strcpy(remcom_out_buffer, "OK");
			break;
		case 'P':		/* set the value of a single CPU register - return OK */
		{
			long regno;
			if(hex_to_int(&ptr, &regno) && *ptr++ == '=')
			if(regno >= 0 && regno < NUMREGS)
			{
				hex2mem(ptr, &((char*)&registers)[regno * sizeof(long)], 4, 0);
				strcpy(remcom_out_buffer, "OK");
				break;
			}
			strcpy(remcom_out_buffer, "E01");
			break;
		}
		/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
		case 'm':
			/* TRY TO READ %x,%x.  IF SUCCEED, SET PTR = 0 */
			if(hex_to_int(&ptr, &addr)) if(*(ptr++) == ',') if(hex_to_int(&ptr, &length))
			{
				ptr = 0;
				mem_err = 0;
				mem2hex((char *) addr, remcom_out_buffer, length, 1);
				if(mem_err) { strcpy(remcom_out_buffer, "E03"); debug_error("memory fault", ""); }
			}
			if(ptr) { strcpy(remcom_out_buffer, "E01"); }
			break;
		/* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
		case 'M':
			/* TRY TO READ '%x,%x:'.  IF SUCCEED, SET PTR = 0 */
			if(hex_to_int(&ptr, &addr)) if(*(ptr++) == ',') if(hex_to_int(&ptr, &length)) if(*(ptr++) == ':')
			{
				mem_err = 0;
				hex2mem (ptr, (char*)addr, length, 1);
				if(mem_err) { strcpy(remcom_out_buffer, "E03"); debug_error("memory fault", ""); }
				else { strcpy(remcom_out_buffer, "OK"); }
				ptr = 0;
			}
			if(ptr) { strcpy(remcom_out_buffer, "E02"); }
			break;
		/* cAA..AA    Continue at address AA..AA(optional) */
		/* sAA..AA   Step one instruction from AA..AA(optional) */
		case 's':
			stepping = 1;
		case 'c':
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
			/* try to read optional parameter, pc unchanged if no parm */
			if(hex_to_int(&ptr, &addr)) *((long*)(&registers.rip)) = addr;
			/* clear the trace bit */
			*((long*)&registers.rflags) &= 0xFFFFFEFF;
			/* set the trace bit if we're stepping */
			if (stepping) *((long*)&registers.rflags) |= 0x100;
 #pragma GCC diagnostic pop           
			return_from_exception();	/* this is a jump */
			break;
		/* kill the program */
		case 'k':		/* do nothing */
			break;
		}			/* switch */
		/* reply to the request */
		putpacket((unsigned char*)remcom_out_buffer);
	}
}
/* this function is used to set up exception handlers for tracing and breakpoints */
void set_debug_traps(void)
{
	stack_ptr = &remcom_stack[STACKSIZE / sizeof (int) - 1];
	exception_handler(0, catch_exception_0);
	exception_handler(1, catch_exception_1);
	exception_handler(3, catch_exception_3);
	exception_handler(4, catch_exception_4);
	exception_handler(5, catch_exception_5);
	exception_handler(6, catch_exception_6);
	exception_handler(7, catch_exception_7);
	exception_handler(8, catch_exception_8);
	exception_handler(9, catch_exception_9);
	exception_handler(10, catch_exception_10);
	exception_handler(11, catch_exception_11);
	exception_handler(12, catch_exception_12);
	exception_handler(13, catch_exception_13);
	exception_handler(14, catch_exception_14);
	exception_handler(16, catch_exception_16);
	initialized = 1;
}
/* This function will generate a breakpoint exception.  It is used at the
beginning of a program to sync up with a debugger and can be used
otherwise as a quick means to stop program execution and "break" into
the debugger.  */
void breakpoint(void) { if(initialized) BREAKPOINT(); }
