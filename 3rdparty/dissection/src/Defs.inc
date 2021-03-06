#! \file DissectionSolver.hpp
#  \brief task mangemanet of dissection algorithm
#  \author Atsushi Suzuki, Laboratoire Jacques-Louis Lions
#  \date   Mar. 30th 2012
#  \date   Jul. 12th 2015
#  \date   Nov. 30th 2016

# This file is part of Dissection
# 
# Dissection is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Linking Dissection statically or dynamically with other modules is making
# a combined work based on Disssection. Thus, the terms and conditions of 
# the GNU General Public License cover the whole combination.
#
# As a special exception, the copyright holders of Dissection give you 
# permission to combine Dissection program with free software programs or 
# libraries that are released under the GNU LGPL and with independent modules 
# that communicate with Dissection solely through the Dissection-fortran 
# interface. You may copy and distribute such a system following the terms of 
# the GNU GPL for Dissection and the licenses of the other code concerned, 
# provided that you include the source code of that other code when and as
# the GNU GPL requires distribution of source code and provided that you do 
# not modify the Dissection-fortran interface.
#
# Note that people who make modified versions of Dissection are not obligated 
# to grant this special exception for their modified versions; it is their
# choice whether to do so. The GNU General Public License gives permission to 
# release a modified version without this exception; this exception also makes
# it possible to release a modified version which carries forward this
# exception. If you modify the Dissection-fortran interface, this exception 
# does not apply to your modified version of Dissection, and you must remove 
# this exception when you distribute your modified version.
#
# This exception is an additional permission under section 7 of the GNU 
# General Public License, version 3 ("GPLv3")
#
# Dissection is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Dissection.  If not, see <http://www.gnu.org/licenses/>.
#

WORK_DIR = $(HOME)/work/Linux
PROJ_DIR = $(WORK_DIR)/dissection

PROJ_SRC = $(PROJ_DIR)/src
PROJ_LIBDIR = $(PROJ_DIR)/lib
UNAME := $(shell uname -s)
# define BLAS MKL or ATLAS : be careful with no space after the name of variable
# QUAD specifies 128bit floating point arithmetic : DD_REAL by qd library
# LOND_DOUBLE on Intel specifies 80bit floating point, which may be enough
ifeq ($(UNAME), Darwin)
  EXT_SHAREDLIB = dylib 
#  BLAS = VECLIB
  BLAS = MKL
  COMPILER=LLVM
  QUAD = DD_REAL
endif 
ifeq ($(UNAME), Linux)
  EXT_SHAREDLIB = so
  BLAS       = MKL
# BLAS = GENERIC
  COMPILER   = INTEL17
#  QUAD = FLOAT128
  QUAD = DD_REAL
endif
PARALLEL   = NONE
MAKE       = make -j 8
#
DEBUGFLAG  = -g -O3
DISSECTION =
#DISSECTION = NO_METIS
#DISSECTION = NO_SCOTCH
#
COMPILERBASE = $(COMPILER)
VER        = $(subst INTEL,,$(COMPILER))
ifneq ($(VER), $(COMPILER))
   COMPILERBASE=$(subst $(VER),,$(COMPILER))
endif
#
ifeq ($(COMPILERBASE), INTEL)
   CXX        = icpc -std=gnu++98 -restrict -fPIC -pthread
   LD         = $(CXX)
else
ifeq ($(COMPILERBASE), GNU)
   CXX        = g++ -std=c++98 -fPIC -pthread -Drestrict=__restrict
   LD         = $(CXX)
   LDOPT      = $(DEBUGFLAG) -shared 
   MAKE_DEP_OPT = -MM
   DEBUGFLAG += #-pg -fno-omit-frame-pointer
   ifeq ($(UNAME), Darwin)
     SHARED_COMPILER = -framework Accelerate 
   else
     SHARED_COMPILER = 
   endif
else
ifeq ($(COMPILERBASE), LLVM)
   ifeq ($(UNAME), Linux)
   CXX        = clang++-3.9 -std=c++03 -pthread -stdlib=libc++ -fPIC -Drestrict=__restrict
   LD         = $(CXX)
   else
   CXX        = clang -x c++ -std=c++03 -pthread -stdlib=libc++ -fPIC -Drestrict=__restrict
   LD         = clang -fPIC -Drestrict=__restrict
   endif
   MAKE_DEP_OPT = -MM
   ifeq ($(UNAME), Darwin)
     LDOPT      = $(DEBUGFLAG) -dynamiclib -install_name '$(PROJ_LIBDIR)/libDissection.dylib' -current_version 1.0
   else
      LDOPT      = $(DEBUGFLAG) -shared
     SHARED_COMPILER = 
   endif
   VER = 17
else
   $(error COMPILER should be specified as one of INTEL14, GNU, LLVM)
endif
endif
endif
ifeq ($(UNAME), Darwin)
#   LDOPT      = $(DEBUGFLAG) -dynamiclib -install_name '$(PROJ_LIBDIR)/libDissection.dylib'
endif
ifeq ($(UNAME), Linux)
   LDOPT      = $(DEBUGFLAG) -shared
endif
   MAKE_DEP_OPT = -MM
#
# metis and scotch are assumed to be compiled as shared library
METIS_INC =  $(WORK_DIR)/metis-5.1.0/include
SCOTCH_INC = $(WORK_DIR)/scotch_6.0.4/include 
QD_INC =     $(WORK_DIR)/include
ifeq ($(UNAME), Darwin)
LIB_DIR_METIS = $(WORK_DIR)/metis-5.1.0/build/Darwin-x86_64/libmetis
endif
ifeq ($(UNAME), Linux)
LIB_DIR_METIS =  $(WORK_DIR)/metis-5.1.0/build/Linux-x86_64/libmetis
endif
LIB_DIR_SCOTCH = $(WORK_DIR)/scotch_6.0.4/lib 
LIB_DIR_QD =     $(WORK_DIR)/lib
#
ifeq ($(BLAS), MKL)
# BLAS_DEF = -DBLAS_MKL -DMKL_ILP64  # 64bit (long long) int for CSR data
  BLAS_DEF = -DBLAS_MKL              # 32bit int
  ifeq ($(VER), 18)
    ifeq ($(UNAME), Linux)
      BLAS_INC = /opt/intel/compilers_and_libraries_2018/linux/mkl/include/
      LIB_DIR_MKL = /opt/intel/compilers_and_libraries_2018/linux/mkl/lib/intel64
      LIB_DIR_INTEL = /opt/intel/compilers_and_libraries_2018/linux/compiler/lib/intel64
    endif
  else
  ifeq ($(VER), 17)
    ifeq ($(UNAME), Linux)
      BLAS_INC = /opt/intel/compilers_and_libraries_2017/linux/mkl/include/
      LIB_DIR_MKL = /opt/intel/compilers_and_libraries_2017/linux/mkl/lib/intel64
      LIB_DIR_INTEL = /opt/intel/compilers_and_libraries_2017/linux/compiler/lib/intel64
    endif
    ifeq ($(UNAME), Darwin)
      BLAS_INC = /opt/intel/compilers_and_libraries_2017/mac/mkl/include/
      LIB_DIR_MKL = /opt/intel/compilers_and_libraries_2017/mac/mkl/lib 
      LIB_DIR_INTEL = /opt/intel/compilers_and_libraries_2017/mac/lib
    endif
  else
  ifeq ($(VER), 16)
    ifeq ($(UNAME), Linux)
      BLAS_INC = /opt/intel/compilers_and_libraries/linux/mkl/include/
      LIB_DIR_MKL = /opt/intel/compilers_and_libraries/linux/mkl/lib/intel64
      LIB_DIR_INTEL = /opt/intel/compilers_and_libraries/linux/compiler/lib/intel64
    endif
  else
  ifeq ($(VER), 15)
    BLAS_INC = /opt/intel/composer_xe_2015/mkl/include
    ifeq ($(UNAME), Linux)
      LIB_DIR_MKL = /opt/intel/composer_xe_2015/mkl/lib/intel64
      LIB_DIR_INTEL = /opt/intel/composer_xe_2015/compiler/lib/intel64
    else
    ifeq ($(UNAME), Darwin)
      LIB_DIR_MKL = /opt/intel/composer_xe_2015/mkl/lib
      LIB_DIR_INTEL = /opt/intel/composer_xe_2015/lib
    endif
    endif
  else
  ifeq ($(VER), 14)
    BLAS_INC = /opt/intel/composer_xe_2013_sp1/mkl/include
    ifeq ($(UNAME), Linux)
      LIB_DIR_MKL = /opt/intel/composer_xe_2013_sp1/mkl/lib/intel64
      LIB_DIR_INTEL = /opt/intel/composer_xe_2013_sp1/compiler/lib/intel64
    else
    ifeq ($(UNAME), Darwin)
      LIB_DIR_MKL = /opt/intel/composer_xe_2013_sp1/mkl/lib
      LIB_DIR_INTEL = /opt/intel/composer_xe_2013_sp1/lib
    endif
    endif
  else
   $(error MKLverion should be 14, 15, 16, 17 or 18)
  endif
  endif
  endif
  endif
  endif
#
  ifeq ($(UNAME), Linux)
     SHARED_COMPILER = -L$(LIB_DIR_INTEL) -lpthread -lrt
     SHARED_BLAS = -L$(LIB_DIR_MKL) -lmkl_intel_lp64 -lmkl_sequential -lmkl_core
#    SHARED_BLAS = -L$(LIB_DIR_MKL) -lmkl_intel_ilp64 -lmkl_sequential -lmkl_core   ## for 64bit integer
  else
  ifeq ($(UNAME), Darwin)
     SHARED_COMPILER =  -L$(LIB_DIR_INTEL) -lc++ -lm #-lintlc -lsvml -lc++ -lm
     SHARED_BLAS = -Wl,-rpath,$(LIB_DIR_MKL) -L$(LIB_DIR_MKL) -lmkl_intel_lp64 -lmkl_sequential -lmkl_core 
  endif
  endif
  #
else
ifeq ($(BLAS), ATLAS)
   BLAS_DEF = -DBLAS_ATLAS
# for ATLAS BLAS
   ifeq ($(COMPILER), INTEL)
      BLAS_INC   = /usr/local/atlas/intel/include
   else
      BLAS_INC   = /usr/local/atlas/include
   endif
else
ifeq ($(BLAS), OPENBLAS)
     BLAS_DEF = -DOPENBLAS
     OPENBLAS_DIR = $(WORK_DIR)/OpenBLAS-0.2.14
     BLAS_INC = $(OPENBLAS_DIR)
     SHARED_BLAS = -L$(OPENBLAS_DIR) -lopenblas
else
ifeq ($(BLAS), VECLIB)
   ifeq ($(UNAME), Darwin)
     BLAS_DEF = -DVECLIB
     BLAS_INC = /System/Library/Frameworks/Accelerate.framework/Versions/Current/Frameworks/vecLib.framework/Headers
     SHARED_COMPILER = -framework Accelerate -lc++ -lm
   endif
else	    
   ifeq ($(BLAS), GENERIC)
     BLAS_DEF = -DBLAS_GENERIC
     BLAS_INC = ..
     SHARED_BLAS =
     SHARED_COMPILER = -lc++ -lm 
else
   $(error BLAS should be specified as one of MKL, VECLIB, OpenBLAS, GENERIC)
endif
endif
endif
endif
endif
#
CCFLAGS = $(DEBUGFLAG) -I$(SCOTCH_INC) $(BLAS_DEF) -I$(BLAS_INC) -I$(PROJ_SRC)
# to use pthread.h in case of C++03
CCFLAGS += -DPOSIX_THREADS
# -DNO_TO_STRING for C++ compiler without to_string() defined in C++11
ifeq ($(COMPILERBASE), INTEL)
  CCFLAGS += -DNO_TO_STRING
endif
ifeq ($(DISSECTION), NO_METIS)
  CCFLAGS += -DNO_METIS
  LIB_METIS = 
else
  CCFLAGS += -I$(METIS_INC)
  ifeq ($(UNAME), Linux)
     LIB_METIS = -Xlinker -rpath=$(LIB_DIR_METIS) -L$(LIB_DIR_METIS) -lmetis
  else
     LIB_METIS = -L$(LIB_DIR_METIS) -lmetis
  endif
  endif
#
ifeq ($(QUAD), DD_REAL)
  CCFLAGS += -D$(QUAD) -I$(QD_INC)
  ifeq ($(UNAME), Linux)
     LIB_QD = -Xlinker -rpath=$(LIB_DIR_QD) -L$(LIB_DIR_QD) -lqd
  else
     LIB_QD = -L$(LIB_DIR_QD) -lqd
  endif
endif
ifeq ($(QUAD), LONG_DOUBLE)
  CCFLAGS += -D$(QUAD) -DNO_OCTRUPLE
endif
ifeq ($(QUAD), FLOAT128)
  CCFLAGS += -DNO_OCTRUPLE
  LIB_QD = -lquadmath
endif
#
#
ifeq ($(UNAME), Darwin)
   CCFLAGS      += #-DGETRUSAGE
   LIB_SCOTCH = -L$(LIB_DIR_SCOTCH) -lscotch -lscotcherr
endif
ifeq ($(UNAME), Linux)
   CCFLAGS      += -DCLOCK_GETTIME
   LIB_SCOTCH = -Xlinker -rpath=$(LIB_DIR_SCOTCH) -L$(LIB_DIR_SCOTCH) -lscotch -lscotcherr
endif
