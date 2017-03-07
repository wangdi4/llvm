LLVMSRC=/nfs/mmdc/disks/tpi4/proj/ThreadPoints/DesignForward2/CSA-LPU/Leap-FPGA-Advisor/llvm-3.6.1.src/
BOOST_ROOT=/usr/intel/pkgs/boost/1.59.0/
all: bbanalysis bbprofile
bbanalysis: bbanalysis.C bbanalysis.H bbprofile.cpp
	clang -g -I $(LLVMSRC)/include -I $(BOOST_ROOT)/include -emit-llvm bbanalysis.C	-c -o bbanalysis.bc
	clang -g -c bbanalysis.bc
bbprofile: bbprofile.cpp
	clang++ -g -I $(LLVMSRC)/include -I $(BOOST_ROOT)/include -emit-llvm bbprofile.cpp	-c -o bbprofile.bc
	clang -g -c bbprofile.bc
clean: 
	rm *.bc *.o
