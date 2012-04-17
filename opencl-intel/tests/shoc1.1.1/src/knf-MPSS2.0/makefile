# Basics
CC		= icc 
CPP     = icc 
LD		= icc
CXX     = icc

SHOC_COMMON=./common

MICROOT=../../bin/Serial/knf-MPSS2.0

BINDIR=../../bin/Serial/knf-MPSS2.0

VPATH=$(SHOC_COMMON)

COMMON_OBJS = main.o \
			 Option.o \
			 OptionParser.o \
			 Timer.o \
			 ResultDatabase.o \
                         ProgressBar.o 

BENCH_OBJS = BusSpeedReadback.o \
                         BusSpeedDownload.o \
                         MaxFlops.o \
                         DeviceMemory.o \
			 GEMM.o \
			 MD.o \
			 Reduction.o \
			 Scan.o \
                         Triad.o \
                         Spmv.o\
			 Sort.o\
			 FFT.o\
			 S3D.o


BENCHMARKPROG = $(patsubst %.o,$(BINDIR)/%,$(BENCH_OBJS))

# MKL paths
MKLROOT=/opt/intel/composerxe_mic/mkl
KNF_MKL_LIB_PATH=/opt/intel/composerxe_mic/mkl/lib/mic

# Flags to include KNF MKL library
KNF_MKL_LIBS=-lmkl_core

CFLAGS          = -vec-report3 -parallel -g -O3 -offload-build -openmp -intel-extensions -pthread -I$(SHOC_COMMON) -I$(MKLROOT)/include -opt-report-phase:offload -openmp-report 

LDFLAGS = -g -vec-report3 -parallel -offload-build -offload-ldopts:"-L$(KNF_MKL_LIB_PATH) $(KNF_MKL_LIBS) -openmp -Wl,--no-undefined" \
	      -L$(MKLROOT)/lib/intel64 -Wl,--start-group -openmp\
          -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -pthread -Wl,--end-group 

%.o: %.cpp
	$(CC) -c $< $(CFLAGS) $(CXXFLAGS)

$(BINDIR)/%: %.o 
	$(CC) -o $@ $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) $(COMMON_OBJS)  $< $(LIBS)  

all : $(BENCHMARKPROG)

$(BENCHMARKPROG) : $(COMMON_OBJS)

clean :
	rm *.o *.out *.xnlog
	rm ./bin/*.o
	rm ./bin/*.out

# individual targets
fft : $(BINDIR)/FFT

$(BINDIR)/FFT : $(COMMON_OBJS)

gemm : $(BINDIR)/GEMM

$(BINDIR)/GEMM : $(COMMON_OBJS)

md : $(BINDIR)/MD

$(BINDIR)/MD : $(COMMON_OBJS)

reduction : $(BINDIR)/Reduction

$(BINDIR)/Reduction : $(COMMON_OBJS)

scan: $(BINDIR)/Scan

$(BINDIR)/Scan : $(COMMON_OBJS)

spmv: $(BINDIR)/Spmv

$(BINDIR)/Sort : $(COMMON_OBJS) Sort.h sortKernel.h

sort: $(BINDIR)/Sort

$(BINDIR)/S3D : $(COMMON_OBJS)

s3d : $(BINDIR)/S3D
