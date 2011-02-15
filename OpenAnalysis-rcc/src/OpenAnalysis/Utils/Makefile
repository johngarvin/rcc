# $Id: Makefile,v 1.3 2004/06/03 13:45:35 mstrout Exp $
# -*-makefile-*-
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

# Best seen in 120-column wide window (or print in landscape mode).
#---------------------------------------------------------------------------------------------------------------------
# This file is part of Mint.
# Arun Chauhan (achauhan@cs.rice.edu), Dept of Computer Science, Rice University, 2000, 2001.
#---------------------------------------------------------------------------------------------------------------------


# Invoke this Makefile to generate the tests grtest and trtest (MMS):
#   % setenv CC g++
#   % make all


#---------------------------------------------------------------------------------------------------------------------
CC             = g++
CXXFLAGS       = -g -I../../

# file related definitions

TRTEST         = trtest
GRTEST         = grtest
ALLOBJS        = DGraphStandard.o Graph.o BaseGraph.o #Tree.o UnionFindUniverse.o
GRTESTOBJS     = grtest.o $(ALLOBJS)
TRTESTOBJS     = trtest.o Tree.o
PARSEROBJS     = Tree.o
#---------------------------------------------------------------------------------------------------------------------


#---------------------------------------------------------------------------------------------------------------------

all: $(ALLOBJS)

alltests: $(GRTEST) #$(TRTEST)

parser: $(PARSEROBJS)

$(TRTEST): $(TRTESTOBJS)
	$(CC) $(CXXFLAGS) -o $(TRTEST) $(TRTESTOBJS) $(LIBS)

$(GRTEST): $(GRTESTOBJS)
	$(CC) $(CXXFLAGS) -o $(GRTEST) $(GRTESTOBJS) $(LIBS)

DGraphStandard.o:
	$(CC) $(CXXFLAGS) -c DGraph/DGraphStandard.cpp

.cpp.o:
	$(CC) $(CXXFLAGS) -c $<

clean:
	/bin/rm -f $(GRTEST) $(GRTEST).o $(TRTEST) $(TRTEST).o $(ALLOBJS)

listobjs:
	@echo $(ALLOBJS)

# header file dependencies
BaseGraph.o : BaseGraph.hpp Iterator.hpp Exception.hpp
Graph.o : Graph.hpp Iterator.hpp Exception.hpp BaseGraph.hpp
#DGraph.o : DGraph.hpp Iterator.hpp Exception.hpp BaseGraph.hpp
Tree.o : Tree.hpp Iterator.hpp Exception.hpp
UnionFindUniverse.o : UnionFindUniverse.hpp

#---------------------------------------------------------------------------------------------------------------------
