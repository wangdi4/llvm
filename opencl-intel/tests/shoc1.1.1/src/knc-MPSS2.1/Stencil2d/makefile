# Basics
CC		= icpc
CPP     = icc
LD		= icc
CXX     = icc
SHOCROOT=..

BINDIR=$(SHOCROOT)/bin

COMMON_DIR=$(SHOCROOT)/src/common

KNF_COMMON=$(SHOCROOT)/src/knc-MPSS2.1/common

VPATH=$(KNF_COMMON):$(COMMON_DIR)

COMMON_SHOC_OBJS = main.o \
			 Option.o \
			 OptionParser.o \
			 Timer.o \
			 ResultDatabase.o \
                         ProgressBar.o

COMMON_OBJS=	         InvalidArgValue.o\
			 CommonKNFStencilFactory.o\
			 KNFStencilKernel.o\
			 KNFStencilFactory.o\
			 KNFStencil.o

BENCH_OBJS = Stencil2Dmain.o

BENCHMARKPROG = $(patsubst %.o,$(BINDIR)/%,$(BENCH_OBJS))


# MKL paths
MKLROOT=/opt/intel/composerxe_mic_12_19_11/mkl
KNF_MKL_LIB_PATH=/opt/intel/composerxe_mic_12_19_11/mkl/lib/mic
 
# Flags to include KNF MKL library
KNF_MKL_LIBS=-lmkl_core
 
CFLAGS          = -vec-report3 -parallel -g -O3 -openmp -intel-extensions -I/opt/intel/composerxe_mic_12_19_11/compiler/include/mic -I../common -I../../common -I../../../config -I$(MKLROOT)/include -opt-report-phase:offload -openmp-report -DLROWS=16 -DLCOLS=16
 
LDFLAGS = -vec-report3 -parallel -offload-ldopts:"-openmp -L$(MKLROOT)/lib/mic -Wl,--start-group -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -Wl,--end-group" \
                       -L$(MKLROOT)/lib/intel64 -Wl,--start-group  -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -Wl,--end-group

%.o: %.cpp
	$(CC) -c $< $(CFLAGS) $(CXXFLAGS)

$(BINDIR)/%: %.o 
	$(CC) -o $@ $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) $(COMMON_SHOC_OBJS) $(COMMON_OBJS) $< $(LIBS) 

all : $(BENCHMARKPROG)

$(BENCHMARKPROG) : $(COMMON_OBJS) $(COMMON_SHOC_OBJS)

clean :
	rm *.o *.out *.xnlog
	rm ./bin/*.o
	rm ./bin/*.out

# individual targets

stencil2d : $(BINDIR)/Stencil2D

$(BINDIR)/Stencil2D : $(COMMON_OBJS)

