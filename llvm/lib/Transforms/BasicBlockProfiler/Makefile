BOOST_ROOT=/usr/intel/pkgs/boost/1.59.0/

all: bbanalysis bbprofile

bbanalysis: bbanalysis.C bbanalysis.H bbprofile.cpp bbinscountanalysis.C
	csa-clang -target x86_64-unknown-linux-gnu -g -I $(BOOST_ROOT)/include -emit-llvm bbanalysis.C -c -o bbanalysis.bc
	csa-clang -target x86_64-unknown-linux-gnu -g -I $(BOOST_ROOT)/include -emit-llvm bbinscountanalysis.C -c -o bbinscountanalysis.bc
	csa-clang -target x86_64-unknown-linux-gnu -g -c bbanalysis.bc

bbprofile: bbprofile.cpp
	csa-clang++ -target x86_64-unknown-linux-gnu -g -I $(BOOST_ROOT)/include -emit-llvm bbprofile.cpp -c -o bbprofile.bc
	csa-clang -target x86_64-unknown-linux-gnu -g -c bbprofile.bc

clean: 
	rm *.bc *.o
