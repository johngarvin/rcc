# -*-Mode: makefile;-*-
# $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/Makefile.am,v 1.4 2005/08/29 09:30:17 garvin Exp $

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

R_INCLUDES = -I@R_SOURCES@/src/include

OA_INCLUDES = -DOA_IRHANDLETYPE_UL -I@OA_PREFIX@/include

MYSOURCES = \
	AnnotationBase.hpp AnnotationBase.cpp \
	Annotation.hpp Annotation.cpp \
	AnnotationSet.hpp AnnotationSet.cpp \
	PropertySet.hpp PropertySet.cpp \
	PropertySetMgr.hpp PropertySetMgr.cpp

MYINCLUDES = \
	-I$(top_srcdir)/src/include   \
	-I..

MYCFLAGS   = $(R_INCLUDES) $(OA_INCLUDES) $(MYINCLUDES)
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

