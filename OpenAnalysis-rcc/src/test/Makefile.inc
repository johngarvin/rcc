# Created 6/2002

SHELL=/bin/sh

# Define UNRELEASED_ROSE if using ROSE-January18a-2006 or some derivative.
#GLOBAL_DEFINES = 
GLOBAL_DEFINES = -DUNRELEASED_ROSE

# Specifies what debugging & profile flags are sent to the compiler
# and linker
#PROFILE_OPTIONS = -O0 -g -pg
PROFILE_OPTIONS = -O0 -g
#PROFILE_OPTIONS =

# Top-level UseOA source directory
#topsrcdir=/usr/casc/overture/bwhite/UseOA-ROSE-trunk/
#topsrcdir=/s/parsons/c/fac/mstrout/OA/UseOA-ROSE-FIAlias/
#topsrcdir=/s/parsons/c/fac/mstrout/OA/armstrong/UseOA-ROSE
#topsrcdir=/s/bach/l/under/stonea/berlios/UseOA-ROSE/trunk
#topsrcdir=/home/stone48/berlios/useoa-rose/trunk
#topsrcdir=/s/parsons/b/others/lramos/OA/UseOA-ROSE
#topsrcdir=/s/bach/l/under/stonea/berlios/UseOA-ROSE/trunk
topsrcdir=/s/parsons/b/others/malusare/UseOA-ROSE

# Root of ROSE installation
#ROSE_INST_ROOT=/s/bach/e/proj/oadev/ROSE/rose-for-4.0.2-temp/LINUX-CORE4-gcc4.0.2
ROSE_INST_ROOT=/s/bach/e/proj/oadev/ROSE/rose-0.8.7a/armstrong
#ROSE_INST_ROOT=/home/stone48/rose/gcc3.4.3
#ROSE_INST_ROOT=/s/bach/e/proj/oadev/ROSE/rose-for-4.0.2-temp/armstrong

# OpenAnalysis variables
#OPENANALYSIS_DIR=/s/parsons/c/fac/mstrout/OA/armstrong/OpenAnalysis
#OPENANALYSIS_DIR=/home/stone48/berlios/openanalysis/OpenAnalysis/branch/locationComparisonChanges
#OPENANALYSIS_DIR=/home/stone48/berlios/openanalysis/OpenAnalysis/trunk
#OPENANALYSIS_DIR=/s/parsons/b/others/lramos/OA/OpenAnalysis
#OPENANALYSIS_DIR=/s/parsons/b/others/malusare/OpenAnalysis
OPENANALYSIS_DIR = /home/malusare/GraphRefactor/OpenAnalysis


OPENANALYSIS_ARCH=i686-Linux
#OPENANALYSIS_ARCH=opteron-Linux
OPENANALYSIS_LIB =-L$(OPENANALYSIS_DIR)/$(OPENANALYSIS_ARCH)/lib -lOAul
OPENANALYSIS_INC =-I$(OPENANALYSIS_DIR)/$(OPENANALYSIS_ARCH)/include 

# Compiler/linker
CXX = g++
CXXLD = g++

ROSE_INCLUDES=-I$(ROSE_INST_ROOT)/include
ROSE_LIBS=-Wl,-rpath,$(ROSE_INST_ROOT)/lib -L$(ROSE_INST_ROOT)/lib -lrose -ledg -lm -lfl -lrt -lpthread


# ================ end of package-specific settings ================

.SUFFIXES: .cpp
