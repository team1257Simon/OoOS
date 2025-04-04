/* Useful if you wish to make target-specific GCC changes. */
#undef  TARGET_OOOS
#define TARGET_OOOS 1

/* Default arguments you want when running your i686-ooos-gcc/x86_64-ooos-gcc toolchain */
#undef  LIB_SPEC
#define LIB_SPEC "-lc -lg -lm -lnosys"

/* Files that are linked before user code. The %s tells GCC to look for these files in the library directory. */
#undef  STARTFILE_SPEC
#define STARTFILE_SPEC "crt0.o%s crti.o%s crtbegin.o%s"

/* Files that are linked after user code. */
#undef  ENDFILE_SPEC
#define ENDFILE_SPEC "crtend.o%s crtn.o%s"

/* Additional predefined macros. */
#undef  TARGET_OS_CPP_BUILTINS
#define TARGET_OS_CPP_BUILTINS()    \
do {                                \
  builtin_define("__ooos__");      \
  builtin_define("__unix__");      \
  builtin_assert("system=ooos");   \
  builtin_assert("system=unix");   \
} while(0);

#undef  LINK_SPEC
#define LINK_SPEC "-z max-page-size=4096 %{shared:-shared} %{static:-static} %{!shared: %{!static: %{rdynamic:-export-dynamic}}}"