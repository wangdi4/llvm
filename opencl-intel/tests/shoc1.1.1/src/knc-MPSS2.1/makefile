# Basics
CC		= icc
CPP     = icc
LD		= icc
CXX     = icc

SHOC_COMMON=./common

MICROOT=./bin

BINDIR=./bin

VPATH=$(SHOC_COMMON)

COMMON_OBJS = main.o \
			 Option.o \
			 OptionParser.o \
			 Timer.o \
			 ResultDatabase.o \
                         ProgressBar.cpp

BENCH_OBJS = BusSpeedReadback.o \
                         BusSpeedDownload.o \
			 MD.o \
			 Reduction.o \
			 Scan.o \
                         Triad.o \
                         Spmv.o\
			 Sort.o\
			 FFT.o\
              S3D.o\
              DeviceMemory.o\
              MaxFlops.o
			

BENCHMARKPROG = $(patsubst %.o,$(BINDIR)/%,$(BENCH_OBJS))

# MKL paths
MKLROOT=/opt/intel/composerxe/mkl
KNF_MKL_LIB_PATH=/opt/intel/composerxe/mkl/lib/mic

# Flags to include KNF MKL library
KNF_MKL_LIBS=-lmkl_core

CFLAGS          = -vec-report3 -parallel -g -O3 -offload-build -openmp -intel-extensions -I/opt/intel/composerxe/include/mic -I./common -I$(SHOC_COMMON) -I$(MKLROOT)/include -opt-report-phase:offload -openmp-report

LDFLAGS = -vec-report3 -parallel -offload-build -offload-ldopts:"-openmp -L$(MKLROOT)/lib/mic -Wl,--start-group -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -Wl,--end-group" \
                       -L$(MKLROOT)/lib/intel64 -Wl,--start-group  -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -Wl,--end-group

#LDFLAGS = -vec-report3 -parallel -offload-build -offload-ldopts:"-openmp -L$(MKLROOT)/lib/mic -Wl,--start-group -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -Wl,--end-group" \
 

%.o: %.cpp
	$(CC) -c $< $(CFLAGS) $(CXXFLAGS)

$(BINDIR)/%: %.o 
	$(CC) -o $@ $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) $(COMMON_OBJS) $< $(LIBS) 

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

$(BINDIR)/Sort : $(COMMON_OBJS)

sort: $(BINDIR)/Sort

$(BINDIR)/S3D : $(COMMON_OBJS)

s3d : $(BINDIR)/S3D

maxflops : $(BINDIR)/MaxFlops
