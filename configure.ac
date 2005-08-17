AC_INIT(rcc, 1.1.7, garvin@rice.edu)
AC_PREFIX_DEFAULT([/usr/local])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/rcc.cc])
AM_CONFIG_HEADER([config.h])
 
AC_CANONICAL_HOST


AC_ARG_WITH(R-sources,
	    [ --with-R-sources=DIR  Prefix of R source tree],
	    R_SOURCES="$withval",
	    R_SOURCES="non_existent_directory")

AC_ARG_WITH(R-prefix,
	    [ --with-R-prefix=DIR   Directory containing R installation],
	    R_PREFIX="$withval",
	    R_PREFIX="non_existent_directory")


AC_ARG_WITH(OA-prefix,
	    [ --with-OA-prefix=DIR  Directory containing OpenAnalysis installation],
	    OA_PREFIX="$withval",
	    OA_PREFIX="non_existent_directory")

if test ! -d ${R_SOURCES}; then
	AC_MSG_ERROR([you must specify the source directory of an R 2.1.1 distribution using --with-R-sources])
fi

if test ! -d ${R_PREFIX}; then
	AC_MSG_ERROR([you must specify the directory of an R 2.1.1 installation using --with-R-prefix])
fi


if test ! -d ${OA_PREFIX}; then
	AC_MSG_ERROR([you must specify the installation directory of an OpenAnalysis distribution using --with-OA-prefix])
fi

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_LIBTOOL

AM_INIT_AUTOMAKE

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([stdlib.h string.h unistd.h sys/file.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC
AC_FUNC_ALLOCA
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([memset pow sqrt strchr strdup strerror floor])

case ${host} in
  powerpc-apple-darwin*)
    R_HOME=${R_PREFIX}
    ;;

  *)
    R_HOME=${R_PREFIX}/lib/R
    ;;
esac

R_LIB_PATH=${R_HOME}/lib

RCC_MACRO_PATH=${prefix}/macros
RCC_INCLUDE_PATH=${prefix}/include/rcc
RCC_LIB_PATH=${prefix}/lib
RCC_BIN_PATH=${prefix}/bin

AC_SUBST(CC)
AC_SUBST(R_SOURCES)
AC_SUBST(RCC_BIN_PATH)
AC_SUBST(R_PREFIX)
AC_SUBST(RCC_MACRO_PATH)
AC_SUBST(RCC_INCLUDE_PATH)
AC_SUBST(RCC_LIB_PATH)
AC_SUBST(R_LIB_PATH)
AC_SUBST(R_HOME)
AC_SUBST(OA_PREFIX)

### ** Platform.

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
AC_SUBST(RCC_INCLUDE_PATH)

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
		 lib/Makefile
		 src/Makefile
		 src/macros/Makefile
		 src/annotations/Makefile
		 src/analysis/Makefile
		 src/support/Makefile
		 tests/Makefile
		 tests/regression/Makefile
		 tests/performance/Makefile
		 tools/Makefile
		 tools/rcc
		 tools/rcc-cc
		 tools/rcc-run
                 ])

# config files currently unused
#		 tests/hand-opt/Makefile tests/temp/Makefile

AC_CONFIG_FILES([tests/regression/run-compiled],
                [chmod +x tests/regression/run-compiled])

AC_CONFIG_FILES([tests/regression/run-interpreted],
                [chmod +x tests/regression/run-interpreted])

AC_CONFIG_FILES([tests/performance/run-test],
                [chmod +x tests/performance/run-test])

AC_OUTPUT
