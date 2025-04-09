













typedef signed char __int8_t;

typedef unsigned char __uint8_t;
typedef short int __int16_t;

typedef short unsigned int __uint16_t;
typedef int __int32_t;

typedef unsigned int __uint32_t;
typedef long int __int64_t;

typedef long unsigned int __uint64_t;
typedef signed char __int_least8_t;

typedef unsigned char __uint_least8_t;
typedef short int __int_least16_t;

typedef short unsigned int __uint_least16_t;
typedef int __int_least32_t;

typedef unsigned int __uint_least32_t;
typedef long int __int_least64_t;

typedef long unsigned int __uint_least64_t;
typedef long int __intmax_t;







typedef long unsigned int __uintmax_t;







typedef long int __intptr_t;

typedef long unsigned int __uintptr_t;

typedef long int ptrdiff_t;
typedef long unsigned int size_t;
typedef int wchar_t;
typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;


typedef unsigned long __blkcnt_t;
typedef unsigned long __blksize_t;
typedef unsigned long __fsblkcnt_t;
typedef unsigned int __fsfilcnt_t;
typedef long _off_t;
typedef int __pid_t;
typedef int __dev_t;
typedef unsigned long __uid_t;
typedef unsigned long __gid_t;
typedef unsigned int __id_t;
typedef unsigned long __ino_t;
typedef unsigned int __mode_t;
__extension__ typedef long long _off64_t;
typedef _off_t __off_t;
typedef _off64_t __loff_t;
typedef long __key_t;
typedef long _fpos_t;

typedef unsigned long ;

typedef signed long _ssize_t;
struct __lock;
typedef struct __lock *_flock_t;
typedef void *_iconv_t;
typedef unsigned long __clock_t;
typedef long __time_t;


typedef unsigned long __clockid_t;
typedef unsigned long __timer_t;
typedef unsigned char __sa_family_t;
typedef unsigned int __socklen_t;
typedef int __nl_item;
typedef unsigned long __nlink_t;
typedef long __suseconds_t;
typedef unsigned long __useconds_t;

typedef __builtin_va_list __va_list;




typedef unsigned int wint_t;

typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    unsigned char __wchb[4];
  } __value;
} _mbstate_t;




typedef int error_t;



typedef unsigned int __ULong;
struct _reent;

struct __locale_t;






struct _Bigint
{
  struct _Bigint *_next;
  int _k, _maxwds, _sign, _wds;
  __ULong _x[1];
};


struct __tm
{
  int __tm_sec;
  int __tm_min;
  int __tm_hour;
  int __tm_mday;
  int __tm_mon;
  int __tm_year;
  int __tm_wday;
  int __tm_yday;
  int __tm_isdst;
};







struct _on_exit_args {
 void * _fnargs[32];
 void * _dso_handle[32];

 __ULong _fntypes;


 __ULong _is_cxa;
};
struct _atexit {
 struct _atexit *_next;
 int _ind;

 void (*_fns[32])(void);
        struct _on_exit_args _on_exit_args;
};
struct __sbuf {
 unsigned char *_base;
 int _size;
};
struct __sFILE {
  unsigned char *_p;
  int _r;
  int _w;
  short _flags;
  short _file;
  struct __sbuf _bf;
  int _lbfsize;






  void * _cookie;

  int (*_read) (struct _reent *, void *,
        char *, int);
  int (*_write) (struct _reent *, void *,
         const char *,
         int);
  _fpos_t (*_seek) (struct _reent *, void *, _fpos_t, int);
  int (*_close) (struct _reent *, void *);


  struct __sbuf _ub;
  unsigned char *_up;
  int _ur;


  unsigned char _ubuf[3];
  unsigned char _nbuf[1];


  struct __sbuf _lb;


  int _blksize;
  _off_t _offset;


  struct _reent *_data;



  _flock_t _lock;

  _mbstate_t _mbstate;
  int _flags2;
};
typedef struct __sFILE __FILE;



struct _glue
{
  struct _glue *_next;
  int _niobs;
  __FILE *_iobs;
};
struct _rand48 {
  unsigned short _seed[3];
  unsigned short _mult[3];
  unsigned short _add;




};
struct _reent
{
  int _errno;




  __FILE *_stdin, *_stdout, *_stderr;

  int _inc;
  char _emergency[25];


  int _unspecified_locale_info;
  struct __locale_t *_locale;

  int __sdidinit;

  void (*__cleanup) (struct _reent *);


  struct _Bigint *_result;
  int _result_k;
  struct _Bigint *_p5s;
  struct _Bigint **_freelist;


  int _cvtlen;
  char *_cvtbuf;

  union
    {
      struct
        {
          unsigned int _unused_rand;
          char * _strtok_last;
          char _asctime_buf[26];
          struct __tm _localtime_buf;
          int _gamma_signgam;
          __extension__ unsigned long long _rand_next;
          struct _rand48 _r48;
          _mbstate_t _mblen_state;
          _mbstate_t _mbtowc_state;
          _mbstate_t _wctomb_state;
          char _l64a_buf[8];
          char _signal_buf[24];
          int _getdate_err;
          _mbstate_t _mbrlen_state;
          _mbstate_t _mbrtowc_state;
          _mbstate_t _mbsrtowcs_state;
          _mbstate_t _wcrtomb_state;
          _mbstate_t _wcsrtombs_state;
   int _h_errno;
        } _reent;



      struct
        {

          unsigned char * _nextf[30];
          unsigned int _nmalloc[30];
        } _unused;
    } _new;



  struct _atexit *_atexit;
  struct _atexit _atexit0;



  void (**(_sig_func))(int);




  struct _glue __sglue;

  __FILE __sf[3];

};
extern struct _reent *_impure_ptr ;
extern struct _reent *const _global_impure_ptr ;

void _reclaim_reent (struct _reent *);



extern int *__errno (void);




extern const char * const _sys_errlist[];
extern int _sys_nerr;









typedef __builtin_va_list __gnuc_va_list;





typedef __gnuc_va_list va_list;
typedef __uint8_t u_int8_t;


typedef __uint16_t u_int16_t;


typedef __uint32_t u_int32_t;


typedef __uint64_t u_int64_t;

typedef int register_t;

typedef __int8_t int8_t ;



typedef __uint8_t uint8_t ;







typedef __int16_t int16_t ;



typedef __uint16_t uint16_t ;







typedef __int32_t int32_t ;



typedef __uint32_t uint32_t ;







typedef __int64_t int64_t ;



typedef __uint64_t uint64_t ;






typedef __intmax_t intmax_t;




typedef __uintmax_t uintmax_t;




typedef __intptr_t intptr_t;




typedef __uintptr_t uintptr_t;



typedef unsigned long __sigset_t;
typedef __suseconds_t suseconds_t;




typedef long time_t;
struct timeval {
 time_t tv_sec;
 suseconds_t tv_usec;
};
struct timespec {
 time_t tv_sec;
 long tv_nsec;
};
struct itimerspec {
 struct timespec it_interval;
 struct timespec it_value;
};



typedef __sigset_t sigset_t;
typedef unsigned long fd_mask;







typedef struct _types_fd_set {
 fd_mask fds_bits[(((64)+(((sizeof (fd_mask) * 8))-1))/((sizeof (fd_mask) * 8)))];
} _types_fd_set;


int select (int __n, _types_fd_set *__readfds, _types_fd_set *__writefds, _types_fd_set *__exceptfds, struct timeval *__timeout)
                                                   ;

int pselect (int __n, _types_fd_set *__readfds, _types_fd_set *__writefds, _types_fd_set *__exceptfds, const struct timespec *__timeout, const sigset_t *__set)

                           ;







typedef __uint32_t in_addr_t;




typedef __uint16_t in_port_t;
typedef unsigned char u_char;



typedef unsigned short u_short;



typedef unsigned int u_int;



typedef unsigned long u_long;







typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;



typedef __blkcnt_t blkcnt_t;




typedef __blksize_t blksize_t;




typedef unsigned long clock_t;
typedef long daddr_t;



typedef char * caddr_t;




typedef __fsblkcnt_t fsblkcnt_t;
typedef __fsfilcnt_t fsfilcnt_t;




typedef __id_t id_t;




typedef __ino_t ino_t;
typedef __off_t off_t;



typedef __dev_t dev_t;



typedef __uid_t uid_t;



typedef __gid_t gid_t;




typedef __pid_t pid_t;




typedef __key_t key_t;




typedef _ssize_t ssize_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __clockid_t clockid_t;





typedef __timer_t timer_t;





typedef __useconds_t useconds_t;
typedef __int64_t sbintime_t;


struct sched_param {
  int sched_priority;
};
typedef __uint32_t pthread_t;
typedef struct {
  int is_initialized;
  void *stackaddr;
  int stacksize;
  int contentionscope;
  int inheritsched;
  int schedpolicy;
  struct sched_param schedparam;





  int detachstate;
} pthread_attr_t;
typedef __uint32_t pthread_mutex_t;

typedef struct {
  int is_initialized;
  int recursive;
} pthread_mutexattr_t;






typedef __uint32_t pthread_cond_t;



typedef struct {
  int is_initialized;
  clock_t clock;



} pthread_condattr_t;



typedef __uint32_t pthread_key_t;

typedef struct {
  int is_initialized;
  int init_executed;
} pthread_once_t;




typedef __FILE FILE;






typedef _fpos_t fpos_t;








typedef int _LOCK_T;
typedef int _LOCK_RECURSIVE_T;
char * ctermid (char *);




FILE * tmpfile (void);
char * tmpnam (char *);

char * tempnam (const char *, const char *);

int fclose (FILE *);
int fflush (FILE *);
FILE * freopen (const char *restrict, const char *restrict, FILE *restrict);
void setbuf (FILE *restrict, char *restrict);
int setvbuf (FILE *restrict, char *restrict, int, size_t);
int fprintf (FILE *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int fscanf (FILE *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
int printf (const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 1, 2)));
int scanf (const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 1, 2)));
int sscanf (const char *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
int vfprintf (FILE *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int vprintf (const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 1, 0)));
int vsprintf (char *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int fgetc (FILE *);
char * fgets (char *restrict, int, FILE *restrict);
int fputc (int, FILE *);
int fputs (const char *restrict, FILE *restrict);
int getc (FILE *);
int getchar (void);
char * gets (char *);
int putc (int, FILE *);
int putchar (int);
int puts (const char *);
int ungetc (int, FILE *);
size_t fread (void *restrict, size_t _size, size_t _n, FILE *restrict);
size_t fwrite (const void *restrict , size_t _size, size_t _n, FILE *);



int fgetpos (FILE *restrict, fpos_t *restrict);

int fseek (FILE *, long, int);



int fsetpos (FILE *, const fpos_t *);

long ftell ( FILE *);
void rewind (FILE *);
void clearerr (FILE *);
int feof (FILE *);
int ferror (FILE *);
void perror (const char *);

FILE * fopen (const char *restrict _name, const char *restrict _type);
int sprintf (char *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int remove (const char *);
int rename (const char *, const char *);
int fseeko (FILE *, off_t, int);
off_t ftello (FILE *);







int snprintf (char *restrict, size_t, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int vsnprintf (char *restrict, size_t, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int vfscanf (FILE *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int vscanf (const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 1, 0)));
int vsscanf (const char *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int asiprintf (char **, const char *, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
char * asniprintf (char *, size_t *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
char * asnprintf (char *restrict, size_t *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));

int diprintf (int, const char *, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));

int fiprintf (FILE *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int fiscanf (FILE *, const char *, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
int iprintf (const char *, ...)
               __attribute__ ((__format__ (__printf__, 1, 2)));
int iscanf (const char *, ...)
               __attribute__ ((__format__ (__scanf__, 1, 2)));
int siprintf (char *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int siscanf (const char *, const char *, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
int sniprintf (char *, size_t, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int vasiprintf (char **, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
char * vasniprintf (char *, size_t *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
char * vasnprintf (char *, size_t *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int vdiprintf (int, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int vfiprintf (FILE *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int vfiscanf (FILE *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int viprintf (const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 1, 0)));
int viscanf (const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 1, 0)));
int vsiprintf (char *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int vsiscanf (const char *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int vsniprintf (char *, size_t, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
FILE * fdopen (int, const char *);

int fileno (FILE *);


int pclose (FILE *);
FILE * popen (const char *, const char *);



void setbuffer (FILE *, char *, int);
int setlinebuf (FILE *);



int getw (FILE *);
int putw (int, FILE *);


int getc_unlocked (FILE *);
int getchar_unlocked (void);
void flockfile (FILE *);
int ftrylockfile (FILE *);
void funlockfile (FILE *);
int putc_unlocked (int, FILE *);
int putchar_unlocked (int);
int dprintf (int, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));

FILE * fmemopen (void *restrict, size_t, const char *restrict);


FILE * open_memstream (char **, size_t *);
int vdprintf (int, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));



int renameat (int, const char *, int, const char *);
int _asiprintf_r (struct _reent *, char **, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
char * _asniprintf_r (struct _reent *, char *, size_t *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 4, 5)));
char * _asnprintf_r (struct _reent *, char *restrict, size_t *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 4, 5)));
int _asprintf_r (struct _reent *, char **restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _diprintf_r (struct _reent *, int, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _dprintf_r (struct _reent *, int, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _fclose_r (struct _reent *, FILE *);
int _fcloseall_r (struct _reent *);
FILE * _fdopen_r (struct _reent *, int, const char *);
int _fflush_r (struct _reent *, FILE *);
int _fgetc_r (struct _reent *, FILE *);
int _fgetc_unlocked_r (struct _reent *, FILE *);
char * _fgets_r (struct _reent *, char *restrict, int, FILE *restrict);
char * _fgets_unlocked_r (struct _reent *, char *restrict, int, FILE *restrict);




int _fgetpos_r (struct _reent *, FILE *, fpos_t *);
int _fsetpos_r (struct _reent *, FILE *, const fpos_t *);

int _fiprintf_r (struct _reent *, FILE *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _fiscanf_r (struct _reent *, FILE *, const char *, ...)
               __attribute__ ((__format__ (__scanf__, 3, 4)));
FILE * _fmemopen_r (struct _reent *, void *restrict, size_t, const char *restrict);
FILE * _fopen_r (struct _reent *, const char *restrict, const char *restrict);
FILE * _freopen_r (struct _reent *, const char *restrict, const char *restrict, FILE *restrict);
int _fprintf_r (struct _reent *, FILE *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _fpurge_r (struct _reent *, FILE *);
int _fputc_r (struct _reent *, int, FILE *);
int _fputc_unlocked_r (struct _reent *, int, FILE *);
int _fputs_r (struct _reent *, const char *restrict, FILE *restrict);
int _fputs_unlocked_r (struct _reent *, const char *restrict, FILE *restrict);
size_t _fread_r (struct _reent *, void *restrict, size_t _size, size_t _n, FILE *restrict);
size_t _fread_unlocked_r (struct _reent *, void *restrict, size_t _size, size_t _n, FILE *restrict);
int _fscanf_r (struct _reent *, FILE *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 3, 4)));
int _fseek_r (struct _reent *, FILE *, long, int);
int _fseeko_r (struct _reent *, FILE *, _off_t, int);
long _ftell_r (struct _reent *, FILE *);
_off_t _ftello_r (struct _reent *, FILE *);
void _rewind_r (struct _reent *, FILE *);
size_t _fwrite_r (struct _reent *, const void *restrict, size_t _size, size_t _n, FILE *restrict);
size_t _fwrite_unlocked_r (struct _reent *, const void *restrict, size_t _size, size_t _n, FILE *restrict);
int _getc_r (struct _reent *, FILE *);
int _getc_unlocked_r (struct _reent *, FILE *);
int _getchar_r (struct _reent *);
int _getchar_unlocked_r (struct _reent *);
char * _gets_r (struct _reent *, char *);
int _iprintf_r (struct _reent *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int _iscanf_r (struct _reent *, const char *, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
FILE * _open_memstream_r (struct _reent *, char **, size_t *);
void _perror_r (struct _reent *, const char *);
int _printf_r (struct _reent *, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int _putc_r (struct _reent *, int, FILE *);
int _putc_unlocked_r (struct _reent *, int, FILE *);
int _putchar_unlocked_r (struct _reent *, int);
int _putchar_r (struct _reent *, int);
int _puts_r (struct _reent *, const char *);
int _remove_r (struct _reent *, const char *);
int _rename_r (struct _reent *,
      const char *_old, const char *_new);
int _scanf_r (struct _reent *, const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
int _siprintf_r (struct _reent *, char *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _siscanf_r (struct _reent *, const char *, const char *, ...)
               __attribute__ ((__format__ (__scanf__, 3, 4)));
int _sniprintf_r (struct _reent *, char *, size_t, const char *, ...)
               __attribute__ ((__format__ (__printf__, 4, 5)));
int _snprintf_r (struct _reent *, char *restrict, size_t, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 4, 5)));
int _sprintf_r (struct _reent *, char *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _sscanf_r (struct _reent *, const char *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 3, 4)));
char * _tempnam_r (struct _reent *, const char *, const char *);
FILE * _tmpfile_r (struct _reent *);
char * _tmpnam_r (struct _reent *, char *);
int _ungetc_r (struct _reent *, int, FILE *);
int _vasiprintf_r (struct _reent *, char **, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
char * _vasniprintf_r (struct _reent*, char *, size_t *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 4, 0)));
char * _vasnprintf_r (struct _reent*, char *, size_t *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 4, 0)));
int _vasprintf_r (struct _reent *, char **, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vdiprintf_r (struct _reent *, int, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vdprintf_r (struct _reent *, int, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vfiprintf_r (struct _reent *, FILE *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vfiscanf_r (struct _reent *, FILE *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 3, 0)));
int _vfprintf_r (struct _reent *, FILE *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vfscanf_r (struct _reent *, FILE *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 3, 0)));
int _viprintf_r (struct _reent *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int _viscanf_r (struct _reent *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int _vprintf_r (struct _reent *, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int _vscanf_r (struct _reent *, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int _vsiprintf_r (struct _reent *, char *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vsiscanf_r (struct _reent *, const char *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 3, 0)));
int _vsniprintf_r (struct _reent *, char *, size_t, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 4, 0)));
int _vsnprintf_r (struct _reent *, char *restrict, size_t, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 4, 0)));
int _vsprintf_r (struct _reent *, char *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vsscanf_r (struct _reent *, const char *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 3, 0)));



int fpurge (FILE *);
ssize_t __getdelim (char **, size_t *, int, FILE *);
ssize_t __getline (char **, size_t *, FILE *);


void clearerr_unlocked (FILE *);
int feof_unlocked (FILE *);
int ferror_unlocked (FILE *);
int fileno_unlocked (FILE *);
int fflush_unlocked (FILE *);
int fgetc_unlocked (FILE *);
int fputc_unlocked (int, FILE *);
size_t fread_unlocked (void *restrict, size_t _size, size_t _n, FILE *restrict);
size_t fwrite_unlocked (const void *restrict , size_t _size, size_t _n, FILE *);
int __srget_r (struct _reent *, FILE *);
int __swbuf_r (struct _reent *, int, FILE *);
FILE *funopen (const void *__cookie,
  int (*__readfn)(void *__cookie, char *__buf,
    int __n),
  int (*__writefn)(void *__cookie, const char *__buf,
     int __n),
  fpos_t (*__seekfn)(void *__cookie, fpos_t __off, int __whence),
  int (*__closefn)(void *__cookie));
FILE *_funopen_r (struct _reent *, const void *__cookie,
  int (*__readfn)(void *__cookie, char *__buf,
    int __n),
  int (*__writefn)(void *__cookie, const char *__buf,
     int __n),
  fpos_t (*__seekfn)(void *__cookie, fpos_t __off, int __whence),
  int (*__closefn)(void *__cookie));
static __inline__ int __sputc_r(struct _reent *_ptr, int _c, FILE *_p) {




 if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
  return (*_p->_p++ = _c);
 else
  return (__swbuf_r(_ptr, _c, _p));
}
static __inline int
_getchar_unlocked(void)
{
 struct _reent *_ptr;

 _ptr = _impure_ptr;
 return ((--(((_ptr)->_stdin))->_r < 0 ? __srget_r(_ptr, ((_ptr)->_stdin)) : (int)(*(((_ptr)->_stdin))->_p++)));
}

static __inline int
_putchar_unlocked(int _c)
{
 struct _reent *_ptr;

 _ptr = _impure_ptr;
 return (__sputc_r(_ptr, _c, ((_ptr)->_stdout)));
}












typedef struct
{
  int quot;
  int rem;
} div_t;

typedef struct
{
  long quot;
  long rem;
} ldiv_t;


typedef struct
{
  long long int quot;
  long long int rem;
} lldiv_t;




typedef int (*__compar_fn_t) (const void *, const void *);







int __locale_mb_cur_max (void);



void abort (void) __attribute__ ((__noreturn__));
int abs (int);

__uint32_t arc4random (void);
__uint32_t arc4random_uniform (__uint32_t);
void arc4random_buf (void *, size_t);

int atexit (void (*__func)(void));
double atof (const char *__nptr);

float atoff (const char *__nptr);

int atoi (const char *__nptr);
int _atoi_r (struct _reent *, const char *__nptr);
long atol (const char *__nptr);
long _atol_r (struct _reent *, const char *__nptr);
void * bsearch (const void *__key,
         const void *__base,
         size_t __nmemb,
         size_t __size,
         __compar_fn_t _compar);
void *calloc(size_t, size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__))
      __attribute__((__alloc_size__(1, 2))) ;
div_t div (int __numer, int __denom);
void exit (int __status) __attribute__ ((__noreturn__));
void free (void *) ;
char * getenv (const char *__string);
char * _getenv_r (struct _reent *, const char *__string);
char * _findenv (const char *, int *);
char * _findenv_r (struct _reent *, const char *, int *);

extern char *suboptarg;
int getsubopt (char **, char * const *, char **);

long labs (long);
ldiv_t ldiv (long __numer, long __denom);
void *malloc(size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(1))) ;
int mblen (const char *, size_t);
int _mblen_r (struct _reent *, const char *, size_t, _mbstate_t *);
int mbtowc (wchar_t *restrict, const char *restrict, size_t);
int _mbtowc_r (struct _reent *, wchar_t *restrict, const char *restrict, size_t, _mbstate_t *);
int wctomb (char *, wchar_t);
int _wctomb_r (struct _reent *, char *, wchar_t, _mbstate_t *);
size_t mbstowcs (wchar_t *restrict, const char *restrict, size_t);
size_t _mbstowcs_r (struct _reent *, wchar_t *restrict, const char *restrict, size_t, _mbstate_t *);
size_t wcstombs (char *restrict, const wchar_t *restrict, size_t);
size_t _wcstombs_r (struct _reent *, char *restrict, const wchar_t *restrict, size_t, _mbstate_t *);


char * mkdtemp (char *);






int mkstemp (char *);


int mkstemps (char *, int);


char * mktemp (char *) __attribute__ ((__deprecated__("the use of `mktemp' is dangerous; use `mkstemp' instead")));


char * _mkdtemp_r (struct _reent *, char *);
int _mkostemp_r (struct _reent *, char *, int);
int _mkostemps_r (struct _reent *, char *, int, int);
int _mkstemp_r (struct _reent *, char *);
int _mkstemps_r (struct _reent *, char *, int);
char * _mktemp_r (struct _reent *, char *) __attribute__ ((__deprecated__("the use of `mktemp' is dangerous; use `mkstemp' instead")));
void qsort (void *__base, size_t __nmemb, size_t __size, __compar_fn_t _compar);
int rand (void);
void *realloc(void *, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2))) ;

void *reallocarray(void *, size_t, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2, 3)));
void *reallocf(void *, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));


char * realpath (const char *restrict path, char *restrict resolved_path);







void srand (unsigned __seed);
double strtod (const char *restrict __n, char **restrict __end_PTR);
double _strtod_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR);

float strtof (const char *restrict __n, char **restrict __end_PTR);







long strtol (const char *restrict __n, char **restrict __end_PTR, int __base);
long _strtol_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR, int __base);
unsigned long strtoul (const char *restrict __n, char **restrict __end_PTR, int __base);
unsigned long _strtoul_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR, int __base);
int system (const char *__string);


long a64l (const char *__input);
char * l64a (long __input);
char * _l64a_r (struct _reent *,long __input);


int on_exit (void (*__func)(int, void *),void *__arg);


void _Exit (int __status) __attribute__ ((__noreturn__));


int putenv (char *__string);

int _putenv_r (struct _reent *, char *__string);
void * _reallocf_r (struct _reent *, void *, size_t);

int setenv (const char *__string, const char *__value, int __overwrite);

int _setenv_r (struct _reent *, const char *__string, const char *__value, int __overwrite);
char * __itoa (int, char *, int);
char * __utoa (unsigned, char *, int);

char * itoa (int, char *, int);
char * utoa (unsigned, char *, int);


int rand_r (unsigned *__seed);



double drand48 (void);
double _drand48_r (struct _reent *);
double erand48 (unsigned short [3]);
double _erand48_r (struct _reent *, unsigned short [3]);
long jrand48 (unsigned short [3]);
long _jrand48_r (struct _reent *, unsigned short [3]);
void lcong48 (unsigned short [7]);
void _lcong48_r (struct _reent *, unsigned short [7]);
long lrand48 (void);
long _lrand48_r (struct _reent *);
long mrand48 (void);
long _mrand48_r (struct _reent *);
long nrand48 (unsigned short [3]);
long _nrand48_r (struct _reent *, unsigned short [3]);
unsigned short *
       seed48 (unsigned short [3]);
unsigned short *
       _seed48_r (struct _reent *, unsigned short [3]);
void srand48 (long);
void _srand48_r (struct _reent *, long);


char * initstate (unsigned, char *, size_t);
long random (void);
char * setstate (char *);
void srandom (unsigned);


long long atoll (const char *__nptr);

long long _atoll_r (struct _reent *, const char *__nptr);

long long llabs (long long);
lldiv_t lldiv (long long __numer, long long __denom);
long long strtoll (const char *restrict __n, char **restrict __end_PTR, int __base);

long long _strtoll_r (struct _reent *, const char *restrict __n, char **restrict __end_PTR, int __base);

unsigned long long strtoull (const char *restrict __n, char **restrict __end_PTR, int __base);

unsigned long long _strtoull_r (struct _reent *, const char *restrict __n, char **restrict __end_PTR, int __base);



void cfree (void *);


int unsetenv (const char *__string);

int _unsetenv_r (struct _reent *, const char *__string);



int __attribute__((__nonnull__ (1))) posix_memalign (void **, size_t, size_t);


char * _dtoa_r (struct _reent *, double, int, int, int *, int*, char**);

void * _malloc_r (struct _reent *, size_t) ;
void * _calloc_r (struct _reent *, size_t, size_t) ;
void _free_r (struct _reent *, void *) ;
void * _realloc_r (struct _reent *, void *, size_t) ;
void _mstats_r (struct _reent *, char *);

int _system_r (struct _reent *, const char *);

void __eprintf (const char *, const char *, unsigned int, const char *);
void qsort_r (void *__base, size_t __nmemb, size_t __size, void *__thunk, int (*_compar)(void *, const void *, const void *))
             __asm__ ("" "__bsd_qsort_r");
extern long double _strtold_r (struct _reent *, const char *restrict, char **restrict);

extern long double strtold (const char *restrict, char **restrict);







void * aligned_alloc(size_t, size_t) __attribute__((__malloc__)) __attribute__((__alloc_align__(1)))
     __attribute__((__alloc_size__(2)));
int at_quick_exit(void (*)(void));
_Noreturn void
 quick_exit(int);





struct __locale_t;
typedef struct __locale_t *locale_t;





int bcmp(const void *, const void *, size_t) __attribute__((__pure__));
void bcopy(const void *, void *, size_t);
void bzero(void *, size_t);


void explicit_bzero(void *, size_t);


int ffs(int) __attribute__((__const__));


int ffsl(long) __attribute__((__const__));
int ffsll(long long) __attribute__((__const__));
int fls(int) __attribute__((__const__));
int flsl(long) __attribute__((__const__));
int flsll(long long) __attribute__((__const__));


char *index(const char *, int) __attribute__((__pure__));
char *rindex(const char *, int) __attribute__((__pure__));

int strcasecmp(const char *, const char *) __attribute__((__pure__));
int strncasecmp(const char *, const char *, size_t) __attribute__((__pure__));


int strcasecmp_l (const char *, const char *, locale_t);
int strncasecmp_l (const char *, const char *, size_t, locale_t);






void * memchr (const void *, int, size_t);
int memcmp (const void *, const void *, size_t);
void * memcpy (void *restrict, const void *restrict, size_t);
void * memmove (void *, const void *, size_t);
void * memset (void *, int, size_t);
char *strcat (char *restrict, const char *restrict);
char *strchr (const char *, int);
int strcmp (const char *, const char *);
int strcoll (const char *, const char *);
char *strcpy (char *restrict, const char *restrict);
size_t strcspn (const char *, const char *);
char *strerror (int);
size_t strlen (const char *);
char *strncat (char *restrict, const char *restrict, size_t);
int strncmp (const char *, const char *, size_t);
char *strncpy (char *restrict, const char *restrict, size_t);
char *strpbrk (const char *, const char *);
char *strrchr (const char *, int);
size_t strspn (const char *, const char *);
char *strstr (const char *, const char *);

char *strtok (char *restrict, const char *restrict);

size_t strxfrm (char *restrict, const char *restrict, size_t);


int strcoll_l (const char *, const char *, locale_t);
char *strerror_l (int, locale_t);
size_t strxfrm_l (char *restrict, const char *restrict, size_t, locale_t);


char *strtok_r (char *restrict, const char *restrict, char **restrict);


int timingsafe_bcmp (const void *, const void *, size_t);
int timingsafe_memcmp (const void *, const void *, size_t);


void * memccpy (void *restrict, const void *restrict, int, size_t);
char *stpcpy (char *restrict, const char *restrict);
char *stpncpy (char *restrict, const char *restrict, size_t);






char *strdup (const char *);

char *_strdup_r (struct _reent *, const char *);

char *strndup (const char *, size_t);

char *_strndup_r (struct _reent *, const char *, size_t);
int strerror_r (int, char *, size_t)

             __asm__ ("" "__xpg_strerror_r")

  ;







char * _strerror_r (struct _reent *, int, int, int *);


size_t strlcat (char *, const char *, size_t);
size_t strlcpy (char *, const char *, size_t);


size_t strnlen (const char *, size_t);


char *strsep (char **, const char *);


char *strnstr(const char *, const char *, size_t) __attribute__((__pure__));



char *strlwr (char *);
char *strupr (char *);



char *strsignal (int __signo);














typedef __uint32_t pgno_t;

typedef __uint16_t indx_t;

typedef __uint32_t recno_t;


typedef struct {
 void *data;
 size_t size;
} DBT;
typedef enum { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;
typedef struct __db {
 DBTYPE type;
 int (*close)(struct __db *);
 int (*del)(const struct __db *, const DBT *, u_int);
 int (*get)(const struct __db *, const DBT *, DBT *, u_int);
 int (*put)(const struct __db *, DBT *, const DBT *, u_int);
 int (*seq)(const struct __db *, DBT *, DBT *, u_int);
 int (*sync)(const struct __db *, u_int);
 void *internal;
 int (*fd)(const struct __db *);
} DB;





typedef struct {

 u_long flags;
 u_int cachesize;
 int maxkeypage;
 int minkeypage;
 u_int psize;
 int (*compare)
     (const DBT *, const DBT *);
 size_t (*prefix)
     (const DBT *, const DBT *);
 int lorder;
} BTREEINFO;





typedef struct {
 u_int bsize;
 u_int ffactor;
 u_int nelem;
 u_int cachesize;
 __uint32_t
  (*hash)(const void *, size_t);
 int lorder;
} HASHINFO;


typedef struct {



 u_long flags;
 u_int cachesize;
 u_int psize;
 int lorder;
 size_t reclen;
 u_char bval;
 char *bfname;
} RECNOINFO;

DB *dbopen(const char *, int, int, DBTYPE, const void *);








       
       
       
       
       
       
       
       
       
       
       
       
       
       








typedef __int_least8_t int_least8_t;
typedef __uint_least8_t uint_least8_t;




typedef __int_least16_t int_least16_t;
typedef __uint_least16_t uint_least16_t;




typedef __int_least32_t int_least32_t;
typedef __uint_least32_t uint_least32_t;




typedef __int_least64_t int_least64_t;
typedef __uint_least64_t uint_least64_t;
  typedef int int_fast8_t;
  typedef unsigned int uint_fast8_t;
  typedef int int_fast16_t;
  typedef unsigned int uint_fast16_t;
  typedef int int_fast32_t;
  typedef unsigned int uint_fast32_t;
  typedef long int int_fast64_t;
  typedef long unsigned int uint_fast64_t;

typedef enum {
 HASH_GET, HASH_PUT, HASH_PUTNEW, HASH_DELETE, HASH_FIRST, HASH_NEXT
} ACTION;


typedef struct _bufhead BUFHEAD;

struct _bufhead {
 BUFHEAD *prev;
 BUFHEAD *next;
 BUFHEAD *ovfl;
 __uint32_t addr;
 char *page;
 char flags;




};



typedef BUFHEAD **SEGMENT;


typedef struct hashhdr {
 int32_t magic;
 int32_t version;
 __uint32_t lorder;
 int32_t bsize;
 int32_t bshift;
 int32_t dsize;
 int32_t ssize;
 int32_t sshift;
 int32_t ovfl_point;

 int32_t last_freed;
 int32_t max_bucket;
 int32_t high_mask;
 int32_t low_mask;

 int32_t ffactor;
 int32_t nkeys;
 int32_t hdrpages;
 int32_t h_charkey;


 int32_t spares[32];
 __uint16_t bitmaps[32];

} HASHHDR;

typedef struct htab {
 HASHHDR hdr;
 int nsegs;
 int exsegs;

 __uint32_t
     (*hash)(const void *, size_t);
 int flags;
 int fp;
 char *tmp_buf;
 char *tmp_key;
 BUFHEAD *cpage;
 int32_t cbucket;
 int cndx;
 int error;

 int new_file;

 int save_file;


 __uint32_t *mapp[32];
 int nmaps;
 int nbufs;

 BUFHEAD bufhead;
 SEGMENT *dir;
} HTAB;
typedef struct {
 BUFHEAD *newp;
 BUFHEAD *oldp;
 BUFHEAD *nextp;
 __uint16_t next_addr;
} SPLIT_RETURN;
BUFHEAD *__add_ovflpage(HTAB *, BUFHEAD *);
int __addel(HTAB *, BUFHEAD *, const DBT *, const DBT *);
int __big_delete(HTAB *, BUFHEAD *);
int __big_insert(HTAB *, BUFHEAD *, const DBT *, const DBT *);
int __big_keydata(HTAB *, BUFHEAD *, DBT *, DBT *, int);
int __big_return(HTAB *, BUFHEAD *, int, DBT *, int);
int __big_split(HTAB *, BUFHEAD *, BUFHEAD *, BUFHEAD *,
  int, __uint32_t, SPLIT_RETURN *);
int __buf_free(HTAB *, int, int);
void __buf_init(HTAB *, int);
__uint32_t __call_hash(HTAB *, char *, int);
int __delpair(HTAB *, BUFHEAD *, int);
int __expand_table(HTAB *);
int __find_bigpair(HTAB *, BUFHEAD *, int, char *, int);
__uint16_t __find_last_page(HTAB *, BUFHEAD **);
void __free_ovflpage(HTAB *, BUFHEAD *);
BUFHEAD *__get_buf(HTAB *, __uint32_t, BUFHEAD *, int);
int __get_page(HTAB *, char *, __uint32_t, int, int, int);
int __ibitmap(HTAB *, int, int, int);
__uint32_t __log2(__uint32_t);
int __put_page(HTAB *, char *, __uint32_t, int, int);
void __reclaim_buf(HTAB *, BUFHEAD *);
int __split_page(HTAB *, __uint32_t, __uint32_t);


extern __uint32_t (*__default_hash)(const void *, size_t);

static int collect_key(HTAB *, BUFHEAD *, int, DBT *, int);
static int collect_data(HTAB *, BUFHEAD *, int, int);
extern int
__big_insert(hashp, bufp, key, val)
 HTAB *hashp;
 BUFHEAD *bufp;
 const DBT *key, *val;
{
 __uint16_t *p;
 int key_size, n, val_size;
 __uint16_t space, move_bytes, off;
 char *cp, *key_data, *val_data;

 cp = bufp->page;
 p = (__uint16_t *)cp;

 key_data = (char *)key->data;
 key_size = key->size;
 val_data = (char *)val->data;
 val_size = val->size;


 for (space = ((p)[(p)[0]+1]) - (4*sizeof(__uint16_t)); key_size;
     space = ((p)[(p)[0]+1]) - (4*sizeof(__uint16_t))) {
  move_bytes = 
              ((
              space
              ) < (
              key_size
              ) ? (
              space
              ) : (
              key_size
              ))
                                  ;
  off = ((p)[(p)[0]+2]) - move_bytes;
  memmove(cp + off, key_data, move_bytes);
  key_size -= move_bytes;
  key_data += move_bytes;
  n = p[0];
  p[++n] = off;
  p[0] = ++n;
  ((p)[(p)[0]+1]) = off - (((n)+3) * sizeof(__uint16_t));
  ((p)[(p)[0]+2]) = off;
  p[n] = 1;
  bufp = __add_ovflpage(hashp, bufp);
  if (!bufp)
   return (-1);
  n = p[0];
  if (!key_size) {
   if (((p)[(p)[0]+1])) {
    move_bytes = 
                ((
                ((p)[(p)[0]+1])
                ) < (
                val_size
                ) ? (
                ((p)[(p)[0]+1])
                ) : (
                val_size
                ))
                                           ;
    off = ((p)[(p)[0]+2]) - move_bytes;
    p[n] = off;
    memmove(cp + off, val_data, move_bytes);
    val_data += move_bytes;
    val_size -= move_bytes;
    p[n - 2] = 3;
    ((p)[(p)[0]+1]) = ((p)[(p)[0]+1]) - move_bytes;
    ((p)[(p)[0]+2]) = off;
   } else
    p[n - 2] = 2;
  }
  p = (__uint16_t *)bufp->page;
  cp = bufp->page;
  bufp->flags |= 0x0001;
 }


 for (space = ((p)[(p)[0]+1]) - (4*sizeof(__uint16_t)); val_size;
     space = ((p)[(p)[0]+1]) - (4*sizeof(__uint16_t))) {
  move_bytes = 
              ((
              space
              ) < (
              val_size
              ) ? (
              space
              ) : (
              val_size
              ))
                                  ;




  if (space == val_size && val_size == val->size)
   move_bytes--;
  off = ((p)[(p)[0]+2]) - move_bytes;
  memmove(cp + off, val_data, move_bytes);
  val_size -= move_bytes;
  val_data += move_bytes;
  n = p[0];
  p[++n] = off;
  p[0] = ++n;
  ((p)[(p)[0]+1]) = off - (((n)+3) * sizeof(__uint16_t));
  ((p)[(p)[0]+2]) = off;
  if (val_size) {
   p[n] = 2;
   bufp = __add_ovflpage(hashp, bufp);
   if (!bufp)
    return (-1);
   cp = bufp->page;
   p = (__uint16_t *)cp;
  } else
   p[n] = 3;
  bufp->flags |= 0x0001;
 }
 return (0);
}
extern int
__big_delete(hashp, bufp)
 HTAB *hashp;
 BUFHEAD *bufp;
{
 BUFHEAD *last_bfp, *rbufp;
 __uint16_t *bp, pageno;
 int key_done, n;

 rbufp = bufp;
 last_bfp = 
           ((void *)0)
               ;
 bp = (__uint16_t *)bufp->page;
 pageno = 0;
 key_done = 0;

 while (!key_done || (bp[2] != 3)) {
  if (bp[2] == 2 || bp[2] == 3)
   key_done = 1;






  if (bp[2] == 3 && ((bp)[(bp)[0]+1]))
   break;
  pageno = bp[bp[0] - 1];
  rbufp->flags |= 0x0001;
  rbufp = __get_buf(hashp, pageno, rbufp, 0);
  if (last_bfp)
   __free_ovflpage(hashp, last_bfp);
  last_bfp = rbufp;
  if (!rbufp)
   return (-1);
  bp = (__uint16_t *)rbufp->page;
 }
 n = bp[0];
 pageno = bp[n - 1];


 bp = (__uint16_t *)bufp->page;
 if (n > 2) {

  bp[1] = pageno;
  bp[2] = 0;
  bufp->ovfl = rbufp->ovfl;
 } else

  bufp->ovfl = 
              ((void *)0)
                  ;
 n -= 2;
 bp[0] = n;
 ((bp)[(bp)[0]+1]) = hashp->hdr.bsize - (((n)+3) * sizeof(__uint16_t));
 ((bp)[(bp)[0]+2]) = hashp->hdr.bsize - 1;

 bufp->flags |= 0x0001;
 if (rbufp)
  __free_ovflpage(hashp, rbufp);
 if (last_bfp != rbufp)
  __free_ovflpage(hashp, last_bfp);

 hashp->hdr.nkeys--;
 return (0);
}







extern int
__find_bigpair(hashp, bufp, ndx, key, size)
 HTAB *hashp;
 BUFHEAD *bufp;
 int ndx;
 char *key;
 int size;
{
 __uint16_t *bp;
 char *p;
 int ksize;
 __uint16_t bytes;
 char *kkey;

 bp = (__uint16_t *)bufp->page;
 p = bufp->page;
 ksize = size;
 kkey = key;

 for (bytes = hashp->hdr.bsize - bp[ndx];
     bytes <= size && bp[ndx + 1] == 1;
     bytes = hashp->hdr.bsize - bp[ndx]) {
  if (memcmp(p + bp[ndx], kkey, bytes))
   return (-2);
  kkey += bytes;
  ksize -= bytes;
  bufp = __get_buf(hashp, bp[ndx + 2], bufp, 0);
  if (!bufp)
   return (-3);
  p = bufp->page;
  bp = (__uint16_t *)p;
  ndx = 1;
 }

 if (bytes != ksize || memcmp(p + bp[ndx], kkey, bytes)) {



  return (-2);
 } else
  return (ndx);
}
extern __uint16_t
__find_last_page(hashp, bpp)
 HTAB *hashp;
 BUFHEAD **bpp;
{
 BUFHEAD *bufp;
 __uint16_t *bp, pageno;
 int n;

 bufp = *bpp;
 bp = (__uint16_t *)bufp->page;
 for (;;) {
  n = bp[0];






  if (bp[2] == 3 &&
      ((n == 2) || (bp[n] == 0) || (((bp)[(bp)[0]+1]))))
   break;

  pageno = bp[n - 1];
  bufp = __get_buf(hashp, pageno, bufp, 0);
  if (!bufp)
   return (0);
  bp = (__uint16_t *)bufp->page;
 }

 *bpp = bufp;
 if (bp[0] > 2)
  return (bp[3]);
 else
  return (0);
}





extern int
__big_return(hashp, bufp, ndx, val, set_current)
 HTAB *hashp;
 BUFHEAD *bufp;
 int ndx;
 DBT *val;
 int set_current;
{
 BUFHEAD *save_p;
 __uint16_t *bp, len, off, save_addr;
 char *tp;

 bp = (__uint16_t *)bufp->page;
 while (bp[ndx + 1] == 1) {
  bufp = __get_buf(hashp, bp[bp[0] - 1], bufp, 0);
  if (!bufp)
   return (-1);
  bp = (__uint16_t *)bufp->page;
  ndx = 1;
 }

 if (bp[ndx + 1] == 2) {
  bufp = __get_buf(hashp, bp[bp[0] - 1], bufp, 0);
  if (!bufp)
   return (-1);
  bp = (__uint16_t *)bufp->page;
  save_p = bufp;
  save_addr = save_p->addr;
  off = bp[1];
  len = 0;
 } else
  if (!((bp)[(bp)[0]+1])) {







   off = bp[bp[0]];
   len = bp[1] - off;
   save_p = bufp;
   save_addr = bufp->addr;
   bufp = __get_buf(hashp, bp[bp[0] - 1], bufp, 0);
   if (!bufp)
    return (-1);
   bp = (__uint16_t *)bufp->page;
  } else {

   tp = (char *)bp;
   off = bp[bp[0]];
   val->data = (u_char *)tp + off;
   val->size = bp[1] - off;
   if (set_current) {
    if (bp[0] == 2) {

     hashp->cpage = 
                   ((void *)0)
                       ;
     hashp->cbucket++;
     hashp->cndx = 1;
    } else {
     hashp->cpage = __get_buf(hashp,
         bp[bp[0] - 1], bufp, 0);
     if (!hashp->cpage)
      return (-1);
     hashp->cndx = 1;
     if (!((__uint16_t *)
         hashp->cpage->page)[0]) {
      hashp->cbucket++;
      hashp->cpage = 
                    ((void *)0)
                        ;
     }
    }
   }
   return (0);
  }

 val->size = collect_data(hashp, bufp, (int)len, set_current);
 if (val->size == -1)
  return (-1);
 if (save_p->addr != save_addr) {

  
 (*__errno()) 
       = 
         22
               ;
  return (-1);
 }
 memmove(hashp->tmp_buf, (save_p->page) + off, len);
 val->data = (u_char *)hashp->tmp_buf;
 return (0);
}




static int
collect_data(hashp, bufp, len, set)
 HTAB *hashp;
 BUFHEAD *bufp;
 int len, set;
{
 __uint16_t *bp;
 char *p;
 BUFHEAD *xbp;
 __uint16_t save_addr;
 int mylen, totlen;

 p = bufp->page;
 bp = (__uint16_t *)p;
 mylen = hashp->hdr.bsize - bp[1];
 save_addr = bufp->addr;

 if (bp[2] == 3) {
  totlen = len + mylen;
  if (hashp->tmp_buf)
   free(hashp->tmp_buf);
  if ((hashp->tmp_buf = (char *)malloc(totlen)) == 
                                                  ((void *)0)
                                                      )
   return (-1);
  if (set) {
   hashp->cndx = 1;
   if (bp[0] == 2) {
    hashp->cpage = 
                  ((void *)0)
                      ;
    hashp->cbucket++;
   } else {
    hashp->cpage =
        __get_buf(hashp, bp[bp[0] - 1], bufp, 0);
    if (!hashp->cpage)
     return (-1);
    else if (!((__uint16_t *)hashp->cpage->page)[0]) {
     hashp->cbucket++;
     hashp->cpage = 
                   ((void *)0)
                       ;
    }
   }
  }
 } else {
  xbp = __get_buf(hashp, bp[bp[0] - 1], bufp, 0);
  if (!xbp || ((totlen =
      collect_data(hashp, xbp, len + mylen, set)) < 1))
   return (-1);
 }
 if (bufp->addr != save_addr) {
  
 (*__errno()) 
       = 
         22
               ;
  return (-1);
 }
 memmove(&hashp->tmp_buf[len], (bufp->page) + bp[1], mylen);
 return (totlen);
}




extern int
__big_keydata(hashp, bufp, key, val, set)
 HTAB *hashp;
 BUFHEAD *bufp;
 DBT *key, *val;
 int set;
{
 key->size = collect_key(hashp, bufp, 0, val, set);
 if (key->size == -1)
  return (-1);
 key->data = (u_char *)hashp->tmp_key;
 return (0);
}





static int
collect_key(hashp, bufp, len, val, set)
 HTAB *hashp;
 BUFHEAD *bufp;
 int len;
 DBT *val;
 int set;
{
 BUFHEAD *xbp;
 char *p;
 int mylen, totlen;
 __uint16_t *bp, save_addr;

 p = bufp->page;
 bp = (__uint16_t *)p;
 mylen = hashp->hdr.bsize - bp[1];

 save_addr = bufp->addr;
 totlen = len + mylen;
 if (bp[2] == 2 || bp[2] == 3) {
  if (hashp->tmp_key != 
                       ((void *)0)
                           )
   free(hashp->tmp_key);
  if ((hashp->tmp_key = (char *)malloc(totlen)) == 
                                                  ((void *)0)
                                                      )
   return (-1);
  if (__big_return(hashp, bufp, 1, val, set))
   return (-1);
 } else {
  xbp = __get_buf(hashp, bp[bp[0] - 1], bufp, 0);
  if (!xbp || ((totlen =
      collect_key(hashp, xbp, totlen, val, set)) < 1))
   return (-1);
 }
 if (bufp->addr != save_addr) {
  
 (*__errno()) 
       = 
         22
               ;
  return (-1);
 }
 memmove(&hashp->tmp_key[len], (bufp->page) + bp[1], mylen);
 return (totlen);
}






extern int
__big_split(hashp, op, np, big_keyp, addr, obucket, ret)
 HTAB *hashp;
 BUFHEAD *op;
 BUFHEAD *np;

 BUFHEAD *big_keyp;
 int addr;
 __uint32_t obucket;
 SPLIT_RETURN *ret;
{
 BUFHEAD *tmpp;
 __uint16_t *tp;
 BUFHEAD *bp;
 DBT key, val;
 __uint32_t change;
 __uint16_t free_space, n, off;

 bp = big_keyp;


 if (__big_keydata(hashp, big_keyp, &key, &val, 0))
  return (-1);
 change = (__call_hash(hashp, key.data, key.size) != obucket);

 if ( (ret->next_addr = __find_last_page(hashp, &big_keyp)) ) {
  if (!(ret->nextp =
      __get_buf(hashp, ret->next_addr, big_keyp, 0)))
   return (-1);;
 } else
  ret->nextp = 
              ((void *)0)
                  ;





 if (change)
  tmpp = np;
 else
  tmpp = op;

 tmpp->flags |= 0x0001;





 tmpp->ovfl = bp;
 tp = (__uint16_t *)tmpp->page;



 n = tp[0];
 off = ((tp)[(tp)[0]+2]);
 free_space = ((tp)[(tp)[0]+1]);
 tp[++n] = (__uint16_t)addr;
 tp[++n] = 0;
 tp[0] = n;
 ((tp)[(tp)[0]+2]) = off;
 ((tp)[(tp)[0]+1]) = free_space - (2*sizeof(__uint16_t));
 ret->newp = np;
 ret->oldp = op;

 tp = (__uint16_t *)big_keyp->page;
 big_keyp->flags |= 0x0001;
 if (tp[0] > 2) {







  n = tp[4];
  free_space = ((tp)[(tp)[0]+1]);
  off = ((tp)[(tp)[0]+2]);
  tp[0] -= 2;
  ((tp)[(tp)[0]+1]) = free_space + (2*sizeof(__uint16_t));
  ((tp)[(tp)[0]+2]) = off;
  tmpp = __add_ovflpage(hashp, big_keyp);
  if (!tmpp)
   return (-1);
  tp[4] = n;
 } else
  tmpp = big_keyp;

 if (change)
  ret->newp = tmpp;
 else
  ret->oldp = tmpp;
 return (0);
}
