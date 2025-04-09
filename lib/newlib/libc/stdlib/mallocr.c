
typedef long int ptrdiff_t;
typedef long unsigned int size_t;
typedef int wchar_t;
typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;








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




typedef __builtin_va_list __gnuc_va_list;





typedef __gnuc_va_list va_list;


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










typedef int error_t;



extern int *__errno (void);




extern const char * const _sys_errlist[];
extern int _sys_nerr;


struct stat;
struct tms;
struct timeval;
struct timezone;
extern int _close_r (struct _reent *, int);
extern int _execve_r (struct _reent *, const char *, char *const *, char *const *);
extern int _fcntl_r (struct _reent *, int, int, int);
extern int _fork_r (struct _reent *);
extern int _fstat_r (struct _reent *, int, struct stat *);
extern int _getpid_r (struct _reent *);
extern int _isatty_r (struct _reent *, int);
extern int _kill_r (struct _reent *, int, int);
extern int _link_r (struct _reent *, const char *, const char *);
extern _off_t _lseek_r (struct _reent *, int, _off_t, int);
extern int _mkdir_r (struct _reent *, const char *, int);
extern int _open_r (struct _reent *, const char *, int, int);
extern _ssize_t _read_r (struct _reent *, int, void *, size_t);
extern int _rename_r (struct _reent *, const char *, const char *);
extern void *_sbrk_r (struct _reent *, ptrdiff_t);
extern int _stat_r (struct _reent *, const char *, struct stat *);
extern unsigned long _times_r (struct _reent *, struct tms *);
extern int _unlink_r (struct _reent *, const char *);
extern int _wait_r (struct _reent *, int *);
extern _ssize_t _write_r (struct _reent *, int, const void *, size_t);


extern int _gettimeofday_r (struct _reent *, struct timeval *__tp, void *__tzp);

extern void __malloc_lock(struct _reent *);
extern void __malloc_unlock(struct _reent *);
void* memset(void*, int, size_t);
void* memcpy(void*, const void*, size_t);
void* memmove(void*, const void*, size_t);





extern char **environ;

void _exit (int __status) __attribute__ ((__noreturn__));

int access (const char *__path, int __amode);
unsigned alarm (unsigned __secs);
int chdir (const char *__path);
int chmod (const char *__path, mode_t __mode);

int chown (const char *__path, uid_t __owner, gid_t __group);


int chroot (const char *__path);

int close (int __fildes);

size_t confstr (int __name, char *__buf, size_t __len);
int daemon (int nochdir, int noclose);

int dup (int __fildes);
int dup2 (int __fildes, int __fildes2);
void endusershell (void);




int execl (const char *__path, const char *, ...);
int execle (const char *__path, const char *, ...);
int execlp (const char *__file, const char *, ...);

int execlpe (const char *__file, const char *, ...);

int execv (const char *__path, char * const __argv[]);
int execve (const char *__path, char * const __argv[], char * const __envp[]);
int execvp (const char *__file, char * const __argv[]);




int faccessat (int __dirfd, const char *__path, int __mode, int __flags);


int fchdir (int __fildes);


int fchmod (int __fildes, mode_t __mode);



int fchown (int __fildes, uid_t __owner, gid_t __group);



int fchownat (int __dirfd, const char *__path, uid_t __owner, gid_t __group, int __flags);


int fexecve (int __fd, char * const __argv[], char * const __envp[]);

pid_t fork (void);
long fpathconf (int __fd, int __name);
int fsync (int __fd);

int fdatasync (int __fd);




char * getcwd (char *__buf, size_t __size);

int getdomainname (char *__name, size_t __len);


int getentropy (void *, size_t);


gid_t getegid (void);
uid_t geteuid (void);
gid_t getgid (void);

int getgroups (int __gidsetsize, gid_t __grouplist[]);

long gethostid (void);

char * getlogin (void);




char * getpass (const char *__prompt);
int getpagesize (void);


int getpeereid (int, uid_t *, gid_t *);


pid_t getpgid (pid_t);

pid_t getpgrp (void);
pid_t getpid (void);
pid_t getppid (void);

pid_t getsid (pid_t);


uid_t getuid (void);


char * getusershell (void);


char * getwd (char *__buf);


int iruserok (unsigned long raddr, int superuser, const char *ruser, const char *luser);

int isatty (int __fildes);

int issetugid (void);



int lchown (const char *__path, uid_t __owner, gid_t __group);


int link (const char *__path1, const char *__path2);

int linkat (int __dirfd1, const char *__path1, int __dirfd2, const char *__path2, int __flags);


int nice (int __nice_value);


off_t lseek (int __fildes, off_t __offset, int __whence);






int lockf (int __fd, int __cmd, off_t __len);

long pathconf (const char *__path, int __name);
int pause (void);

int pthread_atfork (void (*)(void), void (*)(void), void (*)(void));

int pipe (int __fildes[2]);




ssize_t pread (int __fd, void *__buf, size_t __nbytes, off_t __offset);
ssize_t pwrite (int __fd, const void *__buf, size_t __nbytes, off_t __offset);

int read (int __fd, void *__buf, size_t __nbyte);

int rresvport (int *__alport);
int revoke (char *__path);

int rmdir (const char *__path);

int ruserok (const char *rhost, int superuser, const char *ruser, const char *luser);


void * sbrk (ptrdiff_t __incr);



int setegid (gid_t __gid);
int seteuid (uid_t __uid);

int setgid (gid_t __gid);


int setgroups (int ngroups, const gid_t *grouplist);


int sethostname (const char *, size_t);

int setpgid (pid_t __pid, pid_t __pgid);

int setpgrp (void);


int setregid (gid_t __rgid, gid_t __egid);
int setreuid (uid_t __ruid, uid_t __euid);

pid_t setsid (void);

int setuid (uid_t __uid);


void setusershell (void);

unsigned sleep (unsigned int __seconds);



long sysconf (int __name);
pid_t tcgetpgrp (int __fildes);
int tcsetpgrp (int __fildes, pid_t __pgrp_id);
char * ttyname (int __fildes);
int ttyname_r (int, char *, size_t);
int unlink (const char *__path);

int usleep (useconds_t __useconds);


int vhangup (void);

int write (int __fd, const void *__buf, size_t __nbyte);






extern char *optarg;
extern int optind, opterr, optopt;
int getopt(int, char * const [], const char *);
extern int optreset;



pid_t vfork (void);
int ftruncate (int __fd, off_t __length);


int truncate (const char *, off_t __length);




int getdtablesize (void);


useconds_t ualarm (useconds_t __useconds, useconds_t __interval);





 int gethostname (char *__name, size_t __len);




int setdtablesize (int);



void sync (void);



ssize_t readlink (const char *restrict __path,
                          char *restrict __buf, size_t __buflen);
int symlink (const char *__name1, const char *__name2);


ssize_t readlinkat (int __dirfd1, const char *restrict __path,
                            char *restrict __buf, size_t __buflen);
int symlinkat (const char *, int, const char *);
int unlinkat (int, const char *, int);

struct mallinfo {
  int arena;
  int ordblks;
  int smblks;
  int hblks;
  int hblkhd;
  int usmblks;
  int fsmblks;
  int uordblks;
  int fordblks;
  int keepcost;
};
void* _malloc_r(struct _reent *reent_ptr, size_t);
void _free_r(struct _reent *reent_ptr, void*);
void* _realloc_r(struct _reent *reent_ptr, void*, size_t);
void* _memalign_r(struct _reent *reent_ptr, size_t, size_t);
void* _valloc_r(struct _reent *reent_ptr, size_t);
void* _pvalloc_r(struct _reent *reent_ptr, size_t);
void* _calloc_r(struct _reent *reent_ptr, size_t, size_t);
void cfree(void*);
int _malloc_trim_r(struct _reent *reent_ptr, size_t);
size_t _malloc_usable_size_r(struct _reent *reent_ptr, void*);
void _malloc_stats_r(struct _reent *reent_ptr);
int _mallopt_r(struct _reent *reent_ptr, int, int);
struct mallinfo _mallinfo_r(struct _reent *reent_ptr);
struct malloc_chunk
{
  size_t prev_size;
  size_t size;
  struct malloc_chunk* fd;
  struct malloc_chunk* bk;
};

typedef struct malloc_chunk* mchunkptr;
typedef struct malloc_chunk* mbinptr;
 mbinptr __malloc_av_[128 * 2 + 2] = {
 0, 0,
 ((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(2) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(2) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(3) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(3) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(4) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(4) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(5) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(5) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(6) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(6) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(7) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(7) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(8) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(8) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(9) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(9) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(10) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(10) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(11) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(11) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(12) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(12) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(13) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(13) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(14) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(14) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(15) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(15) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(16) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(16) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(17) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(17) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(18) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(18) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(19) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(19) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(20) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(20) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(21) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(21) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(22) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(22) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(23) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(23) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(24) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(24) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(25) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(25) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(26) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(26) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(27) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(27) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(28) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(28) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(29) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(29) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(30) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(30) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(31) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(31) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(32) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(32) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(33) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(33) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(34) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(34) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(35) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(35) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(36) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(36) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(37) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(37) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(38) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(38) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(39) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(39) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(40) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(40) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(41) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(41) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(42) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(42) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(43) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(43) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(44) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(44) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(45) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(45) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(46) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(46) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(47) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(47) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(48) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(48) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(49) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(49) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(50) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(50) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(51) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(51) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(52) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(52) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(53) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(53) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(54) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(54) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(55) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(55) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(56) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(56) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(57) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(57) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(58) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(58) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(59) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(59) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(60) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(60) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(61) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(61) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(62) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(62) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(63) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(63) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(64) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(64) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(65) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(65) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(66) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(66) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(67) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(67) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(68) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(68) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(69) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(69) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(70) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(70) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(71) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(71) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(72) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(72) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(73) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(73) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(74) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(74) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(75) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(75) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(76) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(76) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(77) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(77) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(78) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(78) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(79) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(79) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(80) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(80) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(81) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(81) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(82) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(82) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(83) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(83) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(84) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(84) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(85) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(85) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(86) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(86) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(87) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(87) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(88) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(88) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(89) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(89) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(90) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(90) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(91) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(91) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(92) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(92) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(93) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(93) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(94) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(94) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(95) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(95) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(96) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(96) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(97) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(97) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(98) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(98) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(99) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(99) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(100) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(100) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(101) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(101) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(102) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(102) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(103) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(103) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(104) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(104) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(105) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(105) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(106) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(106) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(107) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(107) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(108) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(108) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(109) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(109) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(110) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(110) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(111) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(111) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(112) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(112) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(113) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(113) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(114) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(114) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(115) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(115) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(116) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(116) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(117) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(117) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(118) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(118) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(119) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(119) + 2]) - 2*(sizeof(size_t)))),
 ((mbinptr)((char*)&(__malloc_av_[2*(120) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(120) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(121) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(121) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(122) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(122) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(123) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(123) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(124) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(124) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(125) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(125) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(126) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(126) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(127) + 2]) - 2*(sizeof(size_t)))), ((mbinptr)((char*)&(__malloc_av_[2*(127) + 2]) - 2*(sizeof(size_t))))
};
 unsigned long __malloc_trim_threshold = (128L * 1024L);
 unsigned long __malloc_top_pad = (0);






 char* __malloc_sbrk_base = (char*)(-1);


 unsigned long __malloc_max_sbrked_mem = 0;


 unsigned long __malloc_max_total_mem = 0;


 struct mallinfo __malloc_current_mallinfo = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static void malloc_extend_top(struct _reent *reent_ptr, size_t nb)



{
  char* brk;
  size_t front_misalign;
  size_t correction;
  int correction_failed = 0;
  char* new_brk;
  size_t top_size;

  mchunkptr old_top = (((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd);
  size_t old_top_size = ((old_top)->size & ~((0x1|0x2)));
  char* old_end = (char*)(((mchunkptr)(((char*)(old_top)) + (old_top_size))));



  size_t sbrk_size = nb + __malloc_top_pad + (sizeof(struct malloc_chunk));
  unsigned long pagesz = (4096);





  if (__malloc_sbrk_base != (char*)(-1))
    sbrk_size = (sbrk_size + (pagesz - 1)) & ~(pagesz - 1);

  brk = (char*)(_sbrk_r(reent_ptr, (sbrk_size)));


  if (brk == (char*)(-1) ||
      (brk < old_end && old_top != ((mchunkptr)(((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))))))
    return;

  (__malloc_current_mallinfo.arena) += sbrk_size;

  if (brk == old_end

      && ((unsigned 
          long
                      )old_end & (pagesz - 1)) == 0)
  {
    top_size = sbrk_size + old_top_size;
    (((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size = (top_size | 0x1));
  }
  else
  {
    if (__malloc_sbrk_base == (char*)(-1))
      __malloc_sbrk_base = brk;
    else
      (__malloc_current_mallinfo.arena) += brk - (char*)old_end;


    front_misalign = (unsigned 
                     long
                                 )((void*)((char*)(brk) + 2*(sizeof(size_t)))) & (((sizeof(size_t)) < 4 ? 8 : ((sizeof(size_t)) + (sizeof(size_t)))) - 1);
    if (front_misalign > 0)
    {
      correction = (((sizeof(size_t)) < 4 ? 8 : ((sizeof(size_t)) + (sizeof(size_t))))) - front_misalign;
      brk += correction;
    }
    else
      correction = 0;


    correction += pagesz - ((unsigned 
                            long
                                        )(brk + sbrk_size) & (pagesz - 1));


    correction &= (pagesz - 1);


    new_brk = (char*)(_sbrk_r(reent_ptr, (correction)));
    if (new_brk == (char*)(-1))
      {
 correction = 0;
 correction_failed = 1;
 new_brk = brk + sbrk_size;
 if (front_misalign > 0)
   new_brk -= (((sizeof(size_t)) < 4 ? 8 : ((sizeof(size_t)) + (sizeof(size_t))))) - front_misalign;
      }

    (__malloc_current_mallinfo.arena) += correction;

    (((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd) = (mchunkptr)brk;
    top_size = new_brk - brk + correction;
    (((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size = (top_size | 0x1));

    if (old_top != ((mchunkptr)(((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t)))))))
    {





      if (old_top_size < (sizeof(struct malloc_chunk)))
      {
        (((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size = (0x1));
        return;
      }


      old_top_size = (old_top_size - 3*(sizeof(size_t))) & ~(((sizeof(size_t)) < 4 ? 8 : ((sizeof(size_t)) + (sizeof(size_t)))) - 1);
      ((old_top)->size = (((old_top)->size & 0x1) | (old_top_size)));
      ((mchunkptr)(((char*)(old_top)) + (old_top_size)))->size =
        (sizeof(size_t))|0x1;
      ((mchunkptr)(((char*)(old_top)) + (old_top_size + (sizeof(size_t)))))->size =
        (sizeof(size_t))|0x1;

      if (old_top_size >= (sizeof(struct malloc_chunk)))
        _free_r(reent_ptr, ((void*)((char*)(old_top) + 2*(sizeof(size_t)))));
    }
  }

  if ((unsigned long)(__malloc_current_mallinfo.arena) > (unsigned long)__malloc_max_sbrked_mem)
    __malloc_max_sbrked_mem = (__malloc_current_mallinfo.arena);




  if ((unsigned long)((__malloc_current_mallinfo.arena)) > (unsigned long)__malloc_max_total_mem)
    __malloc_max_total_mem = (__malloc_current_mallinfo.arena);



  ((void)0)
                       ;
}
void* _malloc_r(struct _reent *reent_ptr, size_t bytes)



{






  mchunkptr victim;
  size_t victim_size;
  int idx;
  mbinptr bin;
  mchunkptr remainder;
  long remainder_size;
  int remainder_index;
  unsigned long block;
  int startidx;
  mchunkptr fwd;
  mchunkptr bck;
  mbinptr q;

  size_t nb = (((unsigned long)((bytes) + ((sizeof(size_t)) + (((sizeof(size_t)) < 4 ? 8 : ((sizeof(size_t)) + (sizeof(size_t)))) - 1))) < (unsigned long)((sizeof(struct malloc_chunk)) + (((sizeof(size_t)) < 4 ? 8 : ((sizeof(size_t)) + (sizeof(size_t)))) - 1))) ? (((sizeof(struct malloc_chunk)) + (((sizeof(size_t)) < 4 ? 8 : ((sizeof(size_t)) + (sizeof(size_t)))) - 1)) & ~((((sizeof(size_t)) < 4 ? 8 : ((sizeof(size_t)) + (sizeof(size_t)))) - 1))) : (((bytes) + ((sizeof(size_t)) + (((sizeof(size_t)) < 4 ? 8 : ((sizeof(size_t)) + (sizeof(size_t)))) - 1))) & ~((((sizeof(size_t)) < 4 ? 8 : ((sizeof(size_t)) + (sizeof(size_t)))) - 1))));


  if (nb > 0x7fffffff 
                  || nb < bytes)
  {
    reent_ptr->_errno = 
            12
                  ;
    return 0;
  }

  __malloc_lock(reent_ptr);



  if ((nb < 512 - 8))
  {
    idx = (((unsigned long)(nb)) >> 3);



    q = ((mbinptr)((char*)&(__malloc_av_[2*(idx) + 2]) - 2*(sizeof(size_t))));
    victim = ((q)->bk);



    if (victim == q)
    {
      q = ((mbinptr)((char*)(q) + 2 * sizeof(mbinptr)));
      victim = ((q)->bk);
    }

    if (victim != q)
    {
      victim_size = ((victim)->size & ~((0x1|0x2)));
      { bck = victim->bk; fwd = victim->fd; fwd->bk = bck; bck->fd = fwd; };
      (((mchunkptr)(((char*)(victim)) + (victim_size)))->size |= 0x1);
      ;
      __malloc_unlock(reent_ptr);
      return ((void*)((char*)(victim) + 2*(sizeof(size_t))));
    }

    idx += 2;

  }
  else
  {
    idx = (((((unsigned long)(nb)) >> 9) == 0) ? (((unsigned long)(nb)) >> 3): ((((unsigned long)(nb)) >> 9) <= 4) ? 56 + (((unsigned long)(nb)) >> 6): ((((unsigned long)(nb)) >> 9) <= 20) ? 91 + (((unsigned long)(nb)) >> 9): ((((unsigned long)(nb)) >> 9) <= 84) ? 110 + (((unsigned long)(nb)) >> 12): ((((unsigned long)(nb)) >> 9) <= 340) ? 119 + (((unsigned long)(nb)) >> 15): ((((unsigned long)(nb)) >> 9) <= 1364) ? 124 + (((unsigned long)(nb)) >> 18): 126);
    bin = ((mbinptr)((char*)&(__malloc_av_[2*(idx) + 2]) - 2*(sizeof(size_t))));

    for (victim = ((bin)->bk); victim != bin; victim = victim->bk)
    {
      victim_size = ((victim)->size & ~((0x1|0x2)));
      remainder_size = (sizeof (long) > sizeof (size_t) && victim_size < nb ? -(long) (nb - victim_size) : (long) (victim_size - nb));

      if (remainder_size >= (long)(sizeof(struct malloc_chunk)))
      {
        --idx;
        break;
      }

      else if (remainder_size >= 0)
      {
        { bck = victim->bk; fwd = victim->fd; fwd->bk = bck; bck->fd = fwd; };
        (((mchunkptr)(((char*)(victim)) + (victim_size)))->size |= 0x1);
        ;
 __malloc_unlock(reent_ptr);
        return ((void*)((char*)(victim) + 2*(sizeof(size_t))));
      }
    }

    ++idx;

  }



  if ( (victim = (((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t)))))->fd) != (((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t))))))
  {
    victim_size = ((victim)->size & ~((0x1|0x2)));
    remainder_size = (sizeof (long) > sizeof (size_t) && victim_size < nb ? -(long) (nb - victim_size) : (long) (victim_size - nb));

    if (remainder_size >= (long)(sizeof(struct malloc_chunk)))
    {
      remainder = ((mchunkptr)(((char*)(victim)) + (nb)));
      ((victim)->size = (nb | 0x1));
      { (((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t)))))->fd = (((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t)))))->bk = remainder; remainder->fd = remainder->bk = (((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t))))); };
      ((remainder)->size = (remainder_size | 0x1));
      (((mchunkptr)((char*)(remainder) + (remainder_size)))->prev_size = (remainder_size));
      ;
      __malloc_unlock(reent_ptr);
      return ((void*)((char*)(victim) + 2*(sizeof(size_t))));
    }

    ((((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t)))))->fd = (((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t)))))->bk = (((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t))))));

    if (remainder_size >= 0)
    {
      (((mchunkptr)(((char*)(victim)) + (victim_size)))->size |= 0x1);
      ;
      __malloc_unlock(reent_ptr);
      return ((void*)((char*)(victim) + 2*(sizeof(size_t))));
    }



    { if (victim_size < 512) { remainder_index = (((unsigned long)(victim_size)) >> 3); ((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->size) |= ((unsigned long)1 << (remainder_index / 4))); bck = ((mbinptr)((char*)&(__malloc_av_[2*(remainder_index) + 2]) - 2*(sizeof(size_t)))); fwd = bck->fd; victim->bk = bck; victim->fd = fwd; fwd->bk = bck->fd = victim; } else { remainder_index = (((((unsigned long)(victim_size)) >> 9) == 0) ? (((unsigned long)(victim_size)) >> 3): ((((unsigned long)(victim_size)) >> 9) <= 4) ? 56 + (((unsigned long)(victim_size)) >> 6): ((((unsigned long)(victim_size)) >> 9) <= 20) ? 91 + (((unsigned long)(victim_size)) >> 9): ((((unsigned long)(victim_size)) >> 9) <= 84) ? 110 + (((unsigned long)(victim_size)) >> 12): ((((unsigned long)(victim_size)) >> 9) <= 340) ? 119 + (((unsigned long)(victim_size)) >> 15): ((((unsigned long)(victim_size)) >> 9) <= 1364) ? 124 + (((unsigned long)(victim_size)) >> 18): 126); bck = ((mbinptr)((char*)&(__malloc_av_[2*(remainder_index) + 2]) - 2*(sizeof(size_t)))); fwd = bck->fd; if (fwd == bck) ((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->size) |= ((unsigned long)1 << (remainder_index / 4))); else { while (fwd != bck && victim_size < ((fwd)->size & ~((0x1|0x2)))) fwd = fwd->fd; bck = fwd->bk; } victim->bk = bck; victim->fd = fwd; fwd->bk = bck->fd = victim; } };
  }






  if ( (block = ((unsigned long)1 << (idx / 4))) <= (((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->size))
  {



    if ( (block & (((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->size)) == 0)
    {

      idx = (idx & ~(4 - 1)) + 4;
      block <<= 1;
      while ((block & (((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->size)) == 0)
      {
        idx += 4;
        block <<= 1;
      }
    }


    for (;;)
    {
      startidx = idx;
      q = bin = ((mbinptr)((char*)&(__malloc_av_[2*(idx) + 2]) - 2*(sizeof(size_t))));


      do
      {


        for (victim = ((bin)->bk); victim != bin; victim = victim->bk)
        {
          victim_size = ((victim)->size & ~((0x1|0x2)));
          remainder_size = (sizeof (long) > sizeof (size_t) && victim_size < nb ? -(long) (nb - victim_size) : (long) (victim_size - nb));

          if (remainder_size >= (long)(sizeof(struct malloc_chunk)))
          {
            remainder = ((mchunkptr)(((char*)(victim)) + (nb)));
            ((victim)->size = (nb | 0x1));
            { bck = victim->bk; fwd = victim->fd; fwd->bk = bck; bck->fd = fwd; };
            { (((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t)))))->fd = (((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t)))))->bk = remainder; remainder->fd = remainder->bk = (((mbinptr)((char*)&(__malloc_av_[2*(1) + 2]) - 2*(sizeof(size_t))))); };
            ((remainder)->size = (remainder_size | 0x1));
            (((mchunkptr)((char*)(remainder) + (remainder_size)))->prev_size = (remainder_size));
            ;
     __malloc_unlock(reent_ptr);
            return ((void*)((char*)(victim) + 2*(sizeof(size_t))));
          }

          else if (remainder_size >= 0)
          {
            (((mchunkptr)(((char*)(victim)) + (victim_size)))->size |= 0x1);
            { bck = victim->bk; fwd = victim->fd; fwd->bk = bck; bck->fd = fwd; };
            ;
     __malloc_unlock(reent_ptr);
            return ((void*)((char*)(victim) + 2*(sizeof(size_t))));
          }

        }

       bin = ((mbinptr)((char*)(bin) + 2 * sizeof(mbinptr)));
      } while ((++idx & (4 - 1)) != 0);



      do
      {
        if ((startidx & (4 - 1)) == 0)
        {
          (((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->size) &= ~block;
          break;
        }
        --startidx;
       q = ((mbinptr)((char*)(q) - 2 * sizeof(mbinptr)));
      } while (((q)->fd) == q);



      if ( (block <<= 1) <= (((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->size) && (block != 0) )
      {
        while ((block & (((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->size)) == 0)
        {
          idx += 4;
          block <<= 1;
        }
      }
      else
        break;
    }
  }





  remainder_size = (sizeof (long) > sizeof (size_t) && (((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size & ~((0x1|0x2))) < nb ? -(long) (nb - (((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size & ~((0x1|0x2)))) : (long) ((((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size & ~((0x1|0x2))) - nb));
  if ((((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size & ~((0x1|0x2))) < nb || remainder_size < (long)(sizeof(struct malloc_chunk)))
  {
    malloc_extend_top(reent_ptr, nb);
    remainder_size = (sizeof (long) > sizeof (size_t) && (((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size & ~((0x1|0x2))) < nb ? -(long) (nb - (((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size & ~((0x1|0x2)))) : (long) ((((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size & ~((0x1|0x2))) - nb));
    if ((((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size & ~((0x1|0x2))) < nb || remainder_size < (long)(sizeof(struct malloc_chunk)))
    {
      __malloc_unlock(reent_ptr);
      return 0;
    }
  }

  victim = (((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd);
  ((victim)->size = (nb | 0x1));
  (((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd) = ((mchunkptr)(((char*)(victim)) + (nb)));
  (((((mbinptr)((char*)&(__malloc_av_[2*(0) + 2]) - 2*(sizeof(size_t))))->fd))->size = (remainder_size | 0x1));
  ;
  __malloc_unlock(reent_ptr);
  return ((void*)((char*)(victim) + 2*(sizeof(size_t))));


}
extern void __malloc_update_mallinfo(void);
