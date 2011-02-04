# $Id: Makefile, 05/20/2008 malusare $

#---------------------------------------------------------------------
CC             = g++
CXXFLAGS       = -g0 -I../../ -DOA_IRHANDLETYPE_UL -Wall

# file related definitions

RCPairTEST     = driver_RCPairTest
ALLOBJS        = driver_RCPairTest.o \
                 RCPair.o \
                 RCPairSet.o \
                 OutputBuilderText.o \
                 Annotation.o \
                 OutputBuilder.o \
                 AliasTag.o \
                 AliasTagSet.o
                 

#---------------------------------------------------------------------

all: $(ALLOBJS) $(RCPairTEST)

$(RCPairTEST): *.cpp *.hpp
	$(CC) $(CXXFLAGS) -o $(RCPairTEST) $(ALLOBJS) $(LIBS)

OutputBuilderText.o:
	$(CC) $(CXXFLAGS) -c ../Utils/OutputBuilderText.cpp

OutputBuilder.o:
	$(CC) $(CXXFLAGS) -c ../Utils/OutputBuilder.cpp

Annotation.o:
	$(CC) $(CXXFLAGS) -c ../OABase/Annotation.cpp

AliasTag.o:
	$(CC) $(CXXFLAGS) -c ../Alias/AliasTag.cpp

AliasTagSet.o:
	$(CC) $(CXXFLAGS) -c ../Alias/AliasTagSet.cpp

RCPair.o: RCPair.hpp RCPair.cpp
	$(CC) $(CXXFLAGS) -c RCPair.cpp

RCPairSet.o: RCPairSet.hpp RCPairSet.cpp
	$(CC) $(CXXFLAGS) -c RCPairSet.cpp

.cpp.o:
	$(CC) $(CXXFLAGS) -c $<

clean:
	/bin/rm -f $(RCPairTEST) $(ALLOBJS)


#---------------------------------------------------------------------
