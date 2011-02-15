#NEEDS GENERALIZED!!
#OADIR          = /Users/mstrout/CVSWorkDirs/OACVS/FIAlias/OpenAnalysis
#PLATFORM       = powerpc-Darwin
#PLATFORM       = i686-Linux
PLATFORM        = i686-Linux
#PLATFORM       = opteron-Linux
#OADIR          = ../../
OADIR          = /home/malusare/08032006/DGraphRefactor
CXX            = g++
CXXFLAGS       = -g -O0 -I$(OADIR)/$(PLATFORM)/include -DOA_IRHANDLETYPE_UL -Wall
#CXXFLAGS       = -DYYDEBUG=1 -g -I$(OADIR)/$(PLATFORM)/include -DOA_IRHANDLETYPE_UL -Wall
OALIBDIR       = $(OADIR)/$(PLATFORM)/lib
LIBS           = -L$(OALIBDIR) -lOAul -lfl
#OBJS           = oa-tab.o oa-lex.o SubsidiaryIR.o SubsidiaryIR_output.o \
                 TestAliasIRInterface.o testSubIR.o loadNotation.o \
                 NotationGenerator.o CmdParams.o
#REGRESS        = regress
#REGRESS_OBJS     = oa-tab.o oa-lex.o SubsidiaryIR.o SubsidiaryIR_output.o \
                 TestAliasIRInterface.o loadNotation.o CmdParams.o

TESTGRAPH      = testgraph
TESTGRAPH_OBJS = driver_TestGraph.cpp
#OACOPY         = oacopy
#OUTPUT         = output
CTAGS          = ctags
               
all:   $(TESTGRAPH)   
#$(REGRESS) $(OACOPY) $(OUTPUT) $(TESTSUBIR) tags

#oa-lex.cpp: oa.l oa-tab.hpp oa-tab.cpp
#	flex -Poa_ -ooa-lex.cpp oa.l

#oa-tab.cpp: oa.y
#	bison -d -p oa_ -o oa-tab.cpp oa.y

#oa-tab.hpp: oa.y
#	bison -d -p oa_ -o oa-tab.cpp oa.y

#$(OACOPY): $(OBJS) driver_oacopy.o $(OADIR)/$(PLATFORM)/lib/libOAul.a
#	$(CXX) $(CXXFLAGS) -o $(OACOPY) $(OBJS) driver_oacopy.o $(LIBS)

#$(OUTPUT): $(OBJS) driver_output.o $(OADIR)/$(PLATFORM)/lib/libOAul.a
#	$(CXX) $(CXXFLAGS) -o $(OUTPUT) $(OBJS) driver_output.o $(LIBS)

#$(REGRESS): $(OBJS) driver_regress.o $(OADIR)/$(PLATFORM)/lib/libOAul.a
#	$(CXX) $(CXXFLAGS) -o $(REGRESS) $(REGRESS_OBJS) driver_regress.o $(LIBS)

$(TESTGRAPH): $(TESTGRAPH_OBJS) driver_TestGraph.o $(OADIR)/$(PLATFORM)/lib/libOAul.a
	    $(CXX) $(CXXFLAGS) -o $(TESTGRAPH) $(TESTGRAPH_OBJS)  $(LIBS)

.cpp.o: 
	$(CXX) $(CXXFLAGS) -c $<

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

tags:	*.cpp *.hpp
	$(CTAGS) *.cpp *.hpp

clean:
	/bin/rm -f $(DRIVER) $(OUTPUT) *-tab.* *-lex.c *.o

