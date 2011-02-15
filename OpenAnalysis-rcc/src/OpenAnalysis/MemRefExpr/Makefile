# $Id: Makefile,v 1.2 2005/01/18 21:13:16 mstrout Exp $

# Invoke this Makefile to generate the tests mretest (MMS):
#   % setenv CC g++
#   % make all


#---------------------------------------------------------------------
CC             = g++
CXXFLAGS       = -g -I../../ -DOA_IRHANDLETYPE_UL -Wall

# file related definitions

MRETEST        = mretest
ALLOBJS        = MemRefExpr.o OutputBuilderText.o Annotation.o \
                 OutputBuilder.o
MRETESTOBJS    = mretest.o $(ALLOBJS)
#---------------------------------------------------------------------


#---------------------------------------------------------------------

all: $(ALLOBJS) $(MRETEST)

$(MRETEST): $(MRETESTOBJS)
	$(CC) $(CXXFLAGS) -o $(MRETEST) $(MRETESTOBJS) $(LIBS)

#config.o:
#	$(CC) $(CXXFLAGS) -c ../config.cpp

OutputBuilderText.o:
	$(CC) $(CXXFLAGS) -c ../Utils/OutputBuilderText.cpp

OutputBuilder.o:
	$(CC) $(CXXFLAGS) -c ../Utils/OutputBuilder.cpp

Annotation.o:
	$(CC) $(CXXFLAGS) -c ../OABase/Annotation.cpp

.cpp.o:
	$(CC) $(CXXFLAGS) -c $<

clean:
	/bin/rm -f $(MRETEST) $(MRETEST).o $(ALLOBJS)

listobjs:
	@echo $(ALLOBJS)

# header file dependencies
MemRefExpr.o: MemRefExpr.hpp

#---------------------------------------------------------------------
