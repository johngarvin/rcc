AC_INIT(rcc, 1.1.0, garvin@rice.edu)
AM_INIT_AUTOMAKE
AC_PREFIX_DEFAULT([/usr])
AC_CONFIG_SRCDIR([src/rcc.cc])
AM_CONFIG_HEADER([config.h])
 
# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_LIBTOOL
AC_PROG_RANLIB

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])
                                                                                
# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
                                                                                
# Checks for library functions.
AC_FUNC_MALLOC
#AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset pow sqrt strchr strdup strerror])
                                                                                

AC_ARG_WITH(R-sources,
	    [ --with-R-sources=DIR  Prefix of R source tree],
	    R_SOURCES="$withval",
	    R_SOURCES="/usr/src/R")
AC_ARG_WITH(R-prefix,
	    [ --with-R-prefix=DIR   Directory containing R installation],
	    R_PREFIX="$withval",
	    R_PREFIX="/usr/lib/R")
AC_SUBST(R_SOURCES)
AC_SUBST(R_PREFIX)

### ** Platform.

AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED(R_PLATFORM, "${host}",
[Define this to be the canonical name (cpu-vendor-os) of your system.])
AC_DEFINE_UNQUOTED(R_CPU, "${host_cpu}",
[Define this to be the name of the CPU of your system.])
AC_DEFINE_UNQUOTED(R_VENDOR, "${host_vendor}",
[Define this to be the name of the vendor of your system.])
AC_DEFINE_UNQUOTED(R_OS, "${host_os}",
[Define this to be the name of the OS of your system.])

R_PLATFORM="${host}"
AC_SUBST(R_PLATFORM)

case "${host_os}" in
  cygwin*|mingw*|windows*|winnt)
    AC_DEFINE(Win32, 1,
              [Define according to your operating system type.])
    R_OSTYPE="windows"
    ;;
  *)
    AC_DEFINE(Unix, 1,
              [Define according to your operating system type.])
    R_OSTYPE="unix"
   ;;
esac
AC_SUBST(R_OSTYPE)

AC_CONFIG_FILES([Makefile
		 rcc.pc
		 src/Makefile
		 lib/Makefile
		 R-patches/Makefile
		 tests/Makefile tests/run-test])

AC_OUTPUT
