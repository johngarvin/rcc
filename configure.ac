include(config/hpc-cxxutils.m4)

AC_INIT(rcc, 1.1.7, garvin@rice.edu)
AC_PREFIX_DEFAULT([/usr/local])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/Main.cc])
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
	AC_MSG_ERROR([--with-R-prefix does not specify a valid R 2.1.1 installation directory.])
fi

if test -d ${R_PREFIX}/R.framework/Versions/2.1.1; then
   R_HOME=${R_PREFIX}/R.framework/Versions/2.1.1/Resources
fi

if test -d ${R_PREFIX}/lib/R; then
   R_HOME=${R_PREFIX}/lib/R
fi

if test -z "${R_HOME}"; then
	AC_MSG_ERROR([--with-R-prefix does not specify a valid R 2.1.1 installation directory.])
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

# Use C++ compiler for all checks
AC_LANG(C++)

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([stdlib.h string.h unistd.h sys/file.h])

HPC_CHECK_CXX_STDC_HEADERS

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC
#AC_FUNC_ALLOCA
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([memset pow sqrt strchr strdup strerror floor])

R_LIB_PATH=${R_HOME}/lib

RCC_MACRO_PATH=${prefix}/macros
RCC_INCLUDE_PATH=${prefix}/include/rcc
RCC_LIB_PATH=${prefix}/lib
RCC_BIN_PATH=${prefix}/bin

AC_SUBST(CC)

AC_SUBST(R_SOURCES)

AC_SUBST(R_LIB_PATH)
AC_SUBST(R_HOME)
AC_SUBST(R_PREFIX)

AC_SUBST(RCC_BIN_PATH)
AC_SUBST(RCC_MACRO_PATH)
AC_SUBST(RCC_INCLUDE_PATH)
AC_SUBST(RCC_LIB_PATH)
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

#----------------------------------------------------------------------------
# Host-dependent configuration
# $host: $host_cpu-$host_vendor-$host_os
#----------------------------------------------------------------------------

HPC_DEF_CXXCMP

#-------------------------------------------------
# Host-specific settings
#-------------------------------------------------

HOST_CXXFLAGS=""
HOST_AR=""
HOST_LIBADD=""


# Options
if test "$OPT_ENABLE_DEVELOP" = "yes" ; then
  my_opt_cxxflags="-g -O0"
else
  # On some compilers, -g implies -O0.  Make sure -O2 takes precedence!
  my_opt_cxxflags="-g -O2"
fi


case ${host} in
  alpha*-*-linux* | alpha*-*-osf*)
    if HPCcxxcmp([cxx]) ; then
      # Compaq compiler
      CXXFLAGS=""            # turn off default -g
      HOST_CXXFLAGS="-std strict_ansi -rtti -pt -ptr ./OA_cxx_trepository"
      HOST_AR="${openanalysis_top}/config/hpcar 'ar cru'"
      HOST_LIBADD="'./OA_cxx_trepository/*.o'"
      if test "$OPT_ENABLE_DEVELOP" = "yes" ; then
        my_opt_cxxflags="-gall"
      else 
        my_opt_cxxflags="-g3 -O2"
      fi
    fi
    ;;

  i386*-*-linux* | i686*-*-linux* | x86_64*-*-linux* | ia64*-*-linux*)
    if HPCcxxcmp([icpc ecpc icc ecc]) ; then
      # Intel compiler
      :
    elif HPCcxxcmp([g++]) ; then
      CXXFLAGS=""
      my_opt_cxxflags="-g -O2 -pedantic"
    fi
    ;;

  mips*-*-irix*)
    if HPCcxxcmp([CC]) ; then
      # SGI MIPSpro
      HOST_CXXFLAGS="-LANG:std"
      HOST_AR="$CXX $CXXFLAGS $HOST_CXXFLAGS -ar -o"
    fi
    ;;

  sparc*-*-solaris*)
    if HPCcxxcmp([CC]) ; then
      # Sun Forte/ONE
      HOST_AR="$CXX $CXXFLAGS -xar -o"
      if test "$OPT_ENABLE_DEVELOP" = "yes" ; then
        my_opt_cxxflags="-g"
      else 
        my_opt_cxxflags="-O"
      fi
    fi
    ;;

  *)
    ;;
esac

# Grab optimization flags
HOST_CXXFLAGS="${HOST_CXXFLAGS} ${my_opt_cxxflags}"

#-------------------------------------------------
# Distribute these settings into makefiles
#-------------------------------------------------
AC_SUBST([HOST_CXXFLAGS])
AC_SUBST([HOST_AR])
AC_SUBST([HOST_LIBADD])
AM_CONDITIONAL(IS_HOST_AR, test "${HOST_AR}" != "")

AC_CONFIG_FILES([Makefile
		 rcc.pc
		 lib/Makefile
		 build/Makefile
		 build/src/Makefile
		 macros/Makefile
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
#		 src/Makefile
#		 src/macros/Makefile
#		 src/analysis/Makefile
#		 src/support/Makefile
#		 src/matmul/Makefile

AC_CONFIG_FILES([tests/regression/run-compiled],
                [chmod +x tests/regression/run-compiled])

AC_CONFIG_FILES([tests/regression/run-interpreted],
                [chmod +x tests/regression/run-interpreted])

AC_CONFIG_FILES([tests/performance/run-test],
                [chmod +x tests/performance/run-test])

AC_OUTPUT
