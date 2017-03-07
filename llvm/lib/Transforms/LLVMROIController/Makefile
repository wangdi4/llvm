LLVMSRC=/nfs/mmdc/disks/tpi4/proj/ThreadPoints/DesignForward2/CSA-LPU/Leap-FPGA-Advisor/llvm-3.6.1.src/
BOOST_ROOT=/usr/intel/pkgs/boost/1.59.0/
all: ROIanalysis ROIhandler
ROIanalysis: ROIanalysis.C ROIanalysis.H 
	clang -g -I $(LLVMSRC)/include -I $(BOOST_ROOT)/include -emit-llvm ROIanalysis.C	-c -o ROIanalysis.bc
ROIhandler: ROIhandler.C
	clang -g -emit-llvm ROIhandler.C -c -o ROIhandler.bc
clean: 
	rm *.bc *.o
