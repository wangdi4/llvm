BOOST_ROOT=/usr/intel/pkgs/boost/1.59.0/

all: ROIanalysis ROIhandler ROImarkeranalysis

ROIanalysis: ROIanalysis.C ROIanalysis.H 
	csa-clang -target x86_64-unknown-linux-gnu -g -I $(CSA_TOP)/tools/src/llvm/include -I $(BOOST_ROOT)/include -emit-llvm ROIanalysis.C	-c -o ROIanalysis.bc

ROIhandler: ROIhandler.C
	csa-clang -target x86_64-unknown-linux-gnu -g -emit-llvm ROIhandler.C -c -o ROIhandler.bc

ROImarker: ROImarkeranalysis.C ROImarkerhandler.C
	csa-clang -target x86_64-unknown-linux-gnu -g -I $(BOOST_ROOT)/include -emit-llvm ROImarkeranalysis.C -c -o ROImarkeranalysis.bc
	csa-clang -target x86_64-unknown-linux-gnu -g -I $(BOOST_ROOT)/include -emit-llvm ROImarkerhandler.C -c -o ROImarkerhandler.bc

clean: 
	rm -f *.bc *.o
