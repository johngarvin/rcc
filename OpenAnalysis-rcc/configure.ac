# configure.ac

## * BeginRiceCopyright *****************************************************
## 
## Copyright ((c)) 2002, Rice University 
## All rights reserved.
## 
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are
## met:
## 
## * Redistributions of source code must retain the above copyright
##   notice, this list of conditions and the following disclaimer.
## 
## * Redistributions in binary form must reproduce the above copyright
##   notice, this list of conditions and the following disclaimer in the
##   documentation and/or other materials provided with the distribution.
## 
## * Neither the name of Rice University (RICE) nor the names of its
##   contributors may be used to endorse or promote products derived from
##   this software without specific prior written permission.
## 
## This software is provided by RICE and contributors "as is" and any
## express or implied warranties, including, but not limited to, the
## implied warranties of merchantability and fitness for a particular
## purpose are disclaimed. In no event shall RICE or contributors be
## liable for any direct, indirect, incidental, special, exemplary, or
## consequential damages (including, but not limited to, procurement of
## substitute goods or services; loss of use, data, or profits; or
## business interruption) however caused and on any theory of liability,
## whether in contract, strict liability, or tort (including negligence
## or otherwise) arising in any way out of the use of this software, even
## if advised of the possibility of such damage. 
## 
## ******************************************************* EndRiceCopyright *

#############################################################################
#
# File:
#   OpenAnalysis/configure.ac
#
# Description:
#   autoconf input file for OpenAnalysis package
#
# Modification history:
#   5/25/03 - first draft written by John Mellor-Crummey
#   6/3/03  - extended by Nathan Tallent
#
#############################################################################

m4_include(config/hpc-cxxutils.m4)

#----------------------------------------------------------------------------
# Initialization: 
# Update to autoconf 2.59 and automake 1.8.3
# Require autoconf 2.54 because automake 1.7.6 needs it.  
#  - automake 1.7.6 is needed to generate Intel 7.1 dependencies correctly.
#  - automake 1.6.1 failed to generate separate lists of object files for
#    the two library versions. automake 1.7.5 handles the specification
#    correctly
#----------------------------------------------------------------------------

AC_INIT([OpenAnalysis], [1.0], [openanalysis-dev@lists.berlios.de], [OpenAnalysis])
AC_CONFIG_SRCDIR([src/OpenAnalysis/config.cpp])
AC_CONFIG_AUX_DIR([config])

AC_PREREQ(2.59)
AM_INIT_AUTOMAKE
#AM_INIT_AUTOMAKE(1.9.5)

AC_CANONICAL_HOST

AC_MSG_NOTICE([INFO: On entry: CXX=${CXX} CXXFLAGS=${CXXFLAGS}])

#----------------------------------------------------------------------------
# Prerequisites: programs, libraries
#----------------------------------------------------------------------------

# AC_PROG_CXX will set CXXFLAGS to something like -g -O2 if not     
# already defined. We do not want this!      
HPC_ENSURE_DEFINED_CXXFLAGS      
HPC_ENSURE_DEFINED_CFLAGS    
     
# Checks for programs.
AC_PROG_CXX(HPC_CXX_LIST)
AC_PROG_CXXCPP
AC_PROG_RANLIB
AC_PROG_INSTALL

AC_MSG_NOTICE([INFO: After check: CXX=${CXX} CXXFLAGS=${CXXFLAGS}])

# Use C++ compiler for all checks
AC_LANG(C++)

# Checks for header files
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(inttypes.h)
HPC_CHECK_CXX_STDC_HEADERS
if test "x$BOOSTROOT" != x;
then 
  CPPFLAGS="$CPPFLAGS -I$BOOSTROOT"
fi
AC_CHECK_HEADERS(boost/dynamic_bitset.hpp, [], [AC_MSG_ERROR("boost includes not in system includes or under \$BOOSTROOT")])

# Checks for typedefs, structures, and compiler characteristics.
# -none thus far-

# Checks for library functions.
# -none thus far-

# ${srcdir}, ${ac_top_srcdir}, ${ac_top_builddir}
openanalysis_top=`cd ${srcdir} && pwd`

#----------------------------------------------------------------------------
# Options
#----------------------------------------------------------------------------

#-------------------------------------------------
# enable-libnative/libcross
#-------------------------------------------------

AC_MSG_CHECKING([whether to build native library])

OPT_ENABLE_LIBNATIVE=yes

AC_ARG_ENABLE(libnative,
  AC_HELP_STRING([--enable-libnative],
                 [Build native library]),
  [case "${enableval}" in
     yes) OPT_ENABLE_LIBNATIVE="yes" ;;
     no)  OPT_ENABLE_LIBNATIVE="no" ;;
     *) AC_MSG_ERROR([bad value ${enableval} for --enable-libnative]) ;;
   esac],
  [OPT_ENABLE_LIBNATIVE="yes"])

AC_MSG_RESULT([${OPT_ENABLE_LIBNATIVE}])
AM_CONDITIONAL(OPT_ENABLE_LIBNATIVE, test "${OPT_ENABLE_LIBNATIVE}" = "yes")


AC_MSG_CHECKING([whether to build native library])

OPT_ENABLE_LIBCROSS=yes

AC_ARG_ENABLE(libcross,
  AC_HELP_STRING([--enable-libcross],
                 [Build cross-platform library]),
  [case "${enableval}" in
     yes) OPT_ENABLE_LIBCROSS="yes" ;;
     no)  OPT_ENABLE_LIBCROSS="no" ;;
     *) AC_MSG_ERROR([bad value ${enableval} for --enable-libcross]) ;;
   esac],
  [OPT_ENABLE_LIBCROSS="no"])

AC_MSG_RESULT([${OPT_ENABLE_LIBCROSS}])
AM_CONDITIONAL(OPT_ENABLE_LIBCROSS, test "${OPT_ENABLE_LIBCROSS}" = "yes")


#-------------------------------------------------
# enable-develop
#-------------------------------------------------

AC_MSG_CHECKING([whether DEVELOP mode is enabled])

OPT_ENABLE_DEVELOP=no

AC_ARG_ENABLE([develop],
  AC_HELP_STRING([--enable-develop],
                 [Build development version (enable debugging)]),
  [case "${enableval}" in
     yes) OPT_ENABLE_DEVELOP="yes" ;;
     no)  OPT_ENABLE_DEVELOP="no" ;;
     *) AC_MSG_ERROR([bad value ${enableval} for --enable-develop]) ;;
   esac],
  [OPT_ENABLE_DEVELOP="no"])

AC_MSG_RESULT([${OPT_ENABLE_DEVELOP}])
AM_CONDITIONAL(OPT_ENABLE_DEVELOP, test "${OPT_ENABLE_DEVELOP}" = "yes")


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
  if HPCcxxcmp([g++ gcc]) ; then    
      # GNU compiler         
      :           
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

#----------------------------------------------------------------------------
# Makefiles to create
#----------------------------------------------------------------------------

AC_OUTPUT([ \
  Makefile \
  src/Makefile \
  src/OpenAnalysis/Makefile \
])
