# -*-Mode: makefile;-*-
# $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/Makefile.am,v 1.1 2005/08/17 19:01:14 johnmc Exp $

## * BeginRiceCopyright *****************************************************
## ******************************************************* EndRiceCopyright *

#############################################################################
#
# File:
#   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/Makefile.am,v $
#
# Description:
#   *Process with automake to produce Makefile.in*
#   
#   Note: All local variables are prefixed with MY to prevent name
#   clashes with automatic automake variables.
#
#############################################################################

# We do not want the standard GNU files (NEWS README AUTHORS ChangeLog...)
AUTOMAKE_OPTIONS = foreign

#############################################################################
# Common settings
#############################################################################

#include $(top_srcdir)/src/Makeinclude.config

#############################################################################
# Local settings
#############################################################################

R_LIB_HOME = @R_LIB_PATH@

R_INCLUDE = @R_PREFIX@/include

MYSOURCES = \
	AnnotationBase.hpp AnnotationBase.cpp \
	Annotation.hpp Annotation.cpp \
	AnnotationSet.hpp AnnotationSet.cpp \
	PropertySet.hpp PropertySet.cpp \
	PropertySetMgr.hpp PropertySetMgr.cpp

MYCFLAGS   = -I$(R_INCLUDE)
MYCXXFLAGS = $(MYCFLAGS)

MYLDFLAGS = 

MYLIBADD = -L$(R_LIB_HOME) -lR

MYCLEAN = 

#############################################################################
# Automake rules
#############################################################################

lib_LTLIBRARIES = libannot.la
libannot_la_SOURCES  = $(MYSOURCES)
libannot_la_CFLAGS   = $(MYCFLAGS)
libannot_la_CXXFLAGS = $(MYCXXFLAGS)
libannot_la_LDFLAGS  = $(MYLDFLAGS)
libannot_la_LIBADD   = $(MYLIBADD)

MOSTLYCLEANFILES = $(MYCLEAN)

#############################################################################
# Common rules
#############################################################################

#include $(top_srcdir)/src/Makeinclude.rules

