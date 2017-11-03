OpenMP runtime lib and include files were copied from ICC 18.0 from the R drive.

libiomp5*.so  cp from /rdrive/crunlib/18_0_20170404/efi2/linux_test/lib_lin
libiomp5*.dll cp from /rdrive/crunlib/18_0_20170404/{efi2,ia32}/nt_test/lib_win
omp.h         cp from /rdrive/crunlib/18_0_20170404/efi2/linux_test/include


OMPTBB: OpenMP runtime subset implemented with TBB:

    Source files from Alexei Katranov, modified by Ernesto to remove stubs and
    rename routines from __kmpc_* to __tbb_omp_*

      - omptbb/src/bindings.cpp
      - omptbb/src/bindings.h

    Ernesto's debug version (prints internal data structs for debugging)
      - omptbb/src_debug/bindings.cpp
      - omptbb/src_debug/bindings.h

    Original Makefile and tests from Alexei Katranov
      - omptbb/Makefile
      - omptbb/test/test_taskloop.cpp
      - omptbb/test/test_taskloop_reduction.cpp

    Binaries
      - bin/Lin64_Release/libomptbb.so:
          omptbb/src/bindings.cpp built with
          clang++ -o build/bindings.o -c -std=c++11 -fPIC -I ../../tbb/include src/bindings.cpp
          clang++ -L ../../tbb/bin/Lin64_Release -shared -o build/libomptbb.so build/bindings.o -ltbb

      - bin/Lin64_Debug/libomptbb.so:
          omptbb/src/bindings.cpp built with
          clang++ -O0 -g -o build/bindings.o -c -std=c++11 -fPIC -I ../../tbb/include src/bindings.cpp
          clang++ -O0 -g -L ../../tbb/bin/Lin64_Debug -shared -o build/libomptbb.so build/bindings.o -ltbb_debug

      - bin/Lin64_Debug/libomptbb_debug.so
          omptbb/src_debug/bindings.cpp built with
          clang++ -O0 -g -o build/bindings.o -c -std=c++11 -fPIC -I ../../tbb/include src_debug/bindings.cpp
          clang++ -O0 -g -L ../../tbb/bin/Lin64_Debug -shared -o build/libomptbb_debug.so build/bindings.o -ltbb_debug
