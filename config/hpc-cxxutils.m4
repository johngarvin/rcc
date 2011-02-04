# -*-Mode: m4;-*-
# $Header: /Volumes/cvsrep/developer/shared/config/hpc-cxxutils.m4,v 1.7 2006/05/29 21:18:59 eraxxon Exp $

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
#   $Source: /Volumes/cvsrep/developer/shared/config/hpc-cxxutils.m4,v $
#
# Description:
#   HPC custom C++ macros for use with autoconf.
#
#############################################################################

#---------------------------------------------------------------
# HPC_CXX_LIST, HPC_CC_LIST
#---------------------------------------------------------------

# Order of preference:
# 1. Non-GNU vendor compiler (on Linux the GNU compiler is technically
#    the system compiler)
#    - Sun|SGI|Compaq/Alpha, PGI, Intel
# 2. GNU compiler

define([HPC_CXX_LIST], [g++ gcc CC  cxx  pgCC  icpc ecpc icc ecc c++])dnl

define([HPC_CC_LIST], [cc  pgcc  icc ecc  gcc])dnl


#---------------------------------------------------------------
# HPC_CLEAR_CXXFLAGS, HPC_CLEAR_CCFLAGS
#---------------------------------------------------------------

AC_DEFUN([HPC_ENSURE_DEFINED_CXXFLAGS],
  [if test -z "$CXXFLAGS" ; then
     CXXFLAGS=""
   fi
  ])dnl

AC_DEFUN([HPC_ENSURE_DEFINED_CFLAGS],
  [if test -z "$CFLAGS" ; then
     CFLAGS=""
   fi
  ])dnl


#---------------------------------------------------------------
# Check C++ standards-conforming access to C headers: #include <cheader>
#---------------------------------------------------------------

AC_DEFUN([HPC_CHECK_CXX_STDC_HEADERS],
  [AC_CACHE_CHECK(
     [whether ${CXX-CC} supports C++ style C headers (<cheader>)],
     [ac_cv_check_cxx_ctd_cheaders],
     [AC_TRY_COMPILE([
#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <cassert>
#include <cerrno>

#include <cstdarg>
#include <cstring>
#include <cmath>
#include <csetjmp>
#include <csignal>

#include <cctype>
#include <climits>
#include <cfloat>
#include <clocale>
],
       [std::strcmp("fee fi", "fo fum");],
       [ac_cv_check_cxx_ctd_cheaders="yes"],
       [ac_cv_check_cxx_ctd_cheaders="no"])])
   if test "$ac_cv_check_cxx_ctd_cheaders" = "no" ; then
     AC_DEFINE([NO_STD_CHEADERS], [], [Standard C headers])
   fi])dnl

# Here is an unreliable way of doing this...
#AC_CHECK_HEADERS([cstdlib cstdio cstring cassert],
#                 [AC_DEFINE([HAVE_STD_CHEADERS])],
#                 [AC_DEFINE([NO_STD_CHEADERS])])

#---------------------------------------------------------------
# HPCcxxcmp
#---------------------------------------------------------------

# Macro for MYcxxcmp
# args: ($1): (proposed_compiler)
define([HPCcxxcmp], [MYcxxcmp $CXX $@])dnl
define([HPCcccmp], [MYcxxcmp $CC $@])dnl


# Given the current compiler and a list of proposed compiler names,
# return 0 if there is a match, 1 otherwise
# args: ($1 $2...): (current_compiler, proposed_compiler...)
AC_DEFUN([HPC_DEF_CXXCMP],
  [MYcxxcmp()
   {
     mycxx=$[]1
     shift
  
     while test $[]# -ge 1 ; do
       if ( echo "$mycxx" | grep "$[]1\$" >/dev/null 2>&1 ); then
         return 0
       fi
       shift
     done
     return 1
   }])dnl

