// INTEL UNSUPPORTED: intel_opencl && i686-pc-windows
/// Covers the Intel performance library options for IPP, MKL, TBB, DAAL

// IPP tests
// RUN: env IPPROOT=/dummy/ipp \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -ipp -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP,CHECK-IPP-LIN,CHECK-IPP-LIN-COMMON %s
// RUN: env IPPROOT=/dummy/ipp \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qipp -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP,CHECK-IPP-LIN,CHECK-IPP-LIN-COMMON %s
// RUN: env IPPROOT=/dummy/ipp \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qipp=common -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP,CHECK-IPP-LIN,CHECK-IPP-LIN-COMMON %s
// RUN: env IPPROOT=/dummy/ipp \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qipp=nonpic -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP,CHECK-IPP-LIN,CHECK-IPP-LIN-NONPIC %s
// RUN: env IPPCRYPTOROOT=/dummy/ippcp \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qipp=crypto -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP-CRYPTO,CHECK-IPP-LIN-CRYPTO %s
// RUN: env IPPCRYPTOROOT=/dummy/ippcp \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qipp=nonpic_crypto -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP-CRYPTO,CHECK-IPP-LIN-NONPIC-CRYPTO %s
// RUN: env IPPROOT=/dummy/ipp \
// RUN: %clang_cl -Qipp -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP,CHECK-IPP-WIN %s
// RUN: env IPPROOT=/dummy/ipp \
// RUN: %clang_cl -Qipp:common -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP,CHECK-IPP-WIN %s
// RUN: env IPPCRYPTOROOT=/dummy/ippcp \
// RUN: %clang_cl -Qipp:crypto -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP-CRYPTO,CHECK-IPP-WIN-CRYPTO %s
// CHECK-IPP-WIN: clang{{.*}} "--dependent-lib=ippcv" "--dependent-lib=ippch" "--dependent-lib=ippcc" "--dependent-lib=ippdc" "--dependent-lib=ippe" "--dependent-lib=ippi" "--dependent-lib=ipps" "--dependent-lib=ippvm" "--dependent-lib=ippcore"
// CHECK-IPP-WIN-CRYPTO: clang{{.*}} "--dependent-lib=ippcp"
// CHECK-IPP: "-internal-isystem" "{{.*}}ipp{{/|\\\\}}include"
// CHECK-IPP-CRYPTO: "-internal-isystem" "{{.*}}ippcp{{/|\\\\}}include"
// CHECK-IPP-LIN-COMMON: ld{{.*}} "-L{{.*}}/ipp{{/|\\\\}}lib{{/|\\\\}}intel64"
// CHECK-IPP-LIN-CRYPTO: ld{{.*}} "-L{{.*}}/ippcp{{/|\\\\}}lib{{/|\\\\}}intel64"
// CHECK-IPP-LIN-NONPIC: ld{{.*}} "-L{{.*}}/ipp{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}nonpic"
// CHECK-IPP-LIN-NONPIC-CRYPTO: ld{{.*}} "-L{{.*}}/ippcp{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}nonpic"
// CHECK-IPP-LIN: "-Bstatic" "-lippcv" "-lippch" "-lippcc" "-lippdc" "-lippe" "-lippi" "-lipps" "-lippvm" "-lippcore" "-Bdynamic"
// CHECK-IPP-LIN-CRYPTO: "-Bstatic" "-lippcp" "-Bdynamic"
// CHECK-IPP-LIN-NONPIC-CRYPTO: "-Bstatic" "-lippcp" "-Bdynamic"
// CHECK-IPP-WIN: link{{.*}} "-libpath:{{.*}}ipp{{/|\\\\}}lib{{/|\\\\}}intel64"
// CHECK-IPP-WIN-CRYPTO: link{{.*}} "-libpath:{{.*}}ippcp{{/|\\\\}}lib{{/|\\\\}}intel64"

// MKL tests
// For SYCL testing, we need to create a dummy fat library as it is needed
// to trigger the offloading step.
// RUN: echo "void foo();" > %t_mkl.cpp
// RUN: %clang -target x86_64-unknown-linux-gnu -fsycl -c %t_mkl.cpp -o %t_mkl.o
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fsycl -c %t_mkl.cpp -o %t_mkl.obj
// RUN: mkdir -p %t_dir/mkl/lib/intel64
// RUN: llvm-ar cr %t_dir/mkl/lib/intel64/libmkl_sycl.a %t_mkl.o
// RUN: llvm-ar cr %t_dir/mkl/lib/intel64/mkl_sycl.lib %t_mkl.obj
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qmkl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-LIN,CHECK-MKL-LIN-PARALLEL-OMP %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qmkl -qtbb -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-LIN,CHECK-MKL-LIN-TBB %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qmkl -fsycl -static -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-LIN,CHECK-MKL-LIN-PARALLEL-OMP,CHECK-MKL-SYCL,CHECK-MKL-LIN-SYCL %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qmkl -fsycl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-LIN,CHECK-MKL-SYCL,CHECK-MKL-LIN-SYCL-DEFAULT %s
// RUN: env MKLROOT=%t_dir/mkl TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qmkl -fsycl --dpcpp -static -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-LIN,CHECK-MKL-LIN-TBB,CHECK-MKL-SYCL,CHECK-MKL-LIN-SYCL,CHECK-MKL-LIN-DPCPP %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qmkl=parallel -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-LIN,CHECK-MKL-LIN-PARALLEL-OMP %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fopenmp -qmkl=parallel -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-LIN,CHECK-MKL-LIN-PARALLEL-OMP %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qmkl=sequential -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-LIN,CHECK-MKL-LIN-SEQUENTIAL %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qmkl=cluster -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-LIN,CHECK-MKL-LIN-CLUSTER %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clang_cl -Qmkl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-WIN,CHECK-MKL-WIN-PARALLEL-OMP %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clang_cl -Qmkl -Qtbb -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-WIN,CHECK-MKL-WIN-TBB %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clang_cl -Qmkl:parallel -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-WIN,CHECK-MKL-WIN-PARALLEL-OMP %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clang_cl -Qmkl:parallel -openmp -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-WIN,CHECK-MKL-WIN-PARALLEL-OMP %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clang_cl -Qmkl -fsycl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-WIN,CHECK-MKL-WIN-PARALLEL-OMP,CHECK-MKL-SYCL,CHECK-MKL-WIN-SYCL %s
// RUN: env MKLROOT=%t_dir/mkl env TBBROOT=/dummy/tbb \
// RUN: %clang_cl -Qmkl -fsycl --dpcpp -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-WIN,CHECK-MKL-WIN-TBB,CHECK-MKL-SYCL,CHECK-MKL-WIN-SYCL,CHECK-MKL-WIN-DPCPP %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clang_cl -Qmkl -Qtbb -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-WIN,CHECK-MKL-WIN-TBB %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clang_cl -Qmkl:sequential -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-WIN,CHECK-MKL-WIN-SEQUENTIAL %s
// RUN: env MKLROOT=%t_dir/mkl \
// RUN: %clang_cl -Qmkl:cluster -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL-WIN,CHECK-MKL-WIN-CLUSTER %s
// CHECK-MKL-WIN-PARALLEL: clang{{.*}} "--dependent-lib=mkl_intel_ilp64" "--dependent-lib=mkl_core" {{.*}} "-internal-isystem" "{{.*}}mkl{{/|\\\\}}include{{/|\\\\}}intel64{{/|\\\\}}lp64"
// CHECK-MKL-WIN-PARALLEL-OMP: clang{{.*}} "--dependent-lib=mkl_intel_ilp64" "--dependent-lib=mkl_intel_thread" "--dependent-lib=mkl_core" {{.*}} "-internal-isystem" "{{.*}}mkl{{/|\\\\}}include{{/|\\\\}}intel64{{/|\\\\}}lp64"
// CHECK-MKL-WIN-SEQUENTIAL: clang{{.*}} "--dependent-lib=mkl_intel_ilp64" "--dependent-lib=mkl_sequential" "--dependent-lib=mkl_core" {{.*}} "-internal-isystem" "{{.*}}mkl{{/|\\\\}}include{{/|\\\\}}intel64{{/|\\\\}}lp64"
// CHECK-MKL-WIN-TBB: clang{{.*}} "--dependent-lib=mkl_intel_ilp64" "--dependent-lib=mkl_tbb_thread" "--dependent-lib=mkl_core" {{.*}} "-internal-isystem" "{{.*}}mkl{{/|\\\\}}include{{/|\\\\}}intel64{{/|\\\\}}lp64"
// CHECK-MKL-WIN-CLUSTER: clang{{.*}} "--dependent-lib=mkl_intel_ilp64" "--dependent-lib=mkl_cdft_core" "--dependent-lib=mkl_scalapack_ilp64" "--dependent-lib=mkl_blacs_intelmpi_ilp64" "--dependent-lib=mkl_sequential" "--dependent-lib=mkl_core" {{.*}} "-internal-isystem" "{{.*}}mkl{{/|\\\\}}include{{/|\\\\}}intel64{{/|\\\\}}lp64"
// CHECK-MKL-WIN-SYCL-NOT: clang-offload-bundler{{.*}} "-type=o" {{.*}} "-inputs=libmkl_sycl"
// CHECK-MKL-WIN-SYCL: clang-offload-bundler{{.*}} "-inputs={{.*}}lib{{/|\\\\}}intel64{{/|\\\\}}mkl_sycl.lib" "-outputs=[[LISTWIN:.+\.a]]" "-unbundle"
// CHECK-MKL-WIN-SYCL: llvm-link{{.*}} "[[LISTWIN]]"
// CHECK-MKL-LIN-SYCL: clang-offload-bundler{{.*}} "-type=a" "-targets=sycl-spir64-unknown-unknown-sycldevice" "-inputs={{.*}}mkl{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}libmkl_sycl.a" "-outputs=[[LISTA:.+\.a]]" "-unbundle"
// CHECK-MKL-LIN-SYCL: llvm-link{{.*}} "[[LISTA]]"
// CHECK-MKL-SYCL: llvm-spirv{{.*}}
// CHECK-MKL-SYCL: clang-offload-wrapper{{.*}}
// CHECK-MKL-SYCL: llc{{.*}}
// CHECK-MKL-LIN: ld{{.*}} "-L{{.*}}mkl{{/|\\\\}}lib{{/|\\\\}}intel64"
// CHECK-MKL-LIN-DPCPP: "-L{{.*}}tbb{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}gcc4.8"
// CHECK-MKL-LIN-SYCL-DEFAULT: "--start-group" "-lmkl_sycl" "-lmkl_intel_ilp64" "-lmkl_intel_thread" "-lmkl_core" "--end-group"
// CHECK-MKL-LIN-PARALLEL: "--start-group" "-lmkl_intel_ilp64" "-lmkl_core" "--end-group" {{.*}} "-liomp5"
// CHECK-MKL-LIN-PARALLEL-OMP: "--start-group" "-lmkl_intel_ilp64" "-lmkl_intel_thread" "-lmkl_core" "--end-group"
// CHECK-MKL-LIN-TBB: "--start-group" "-lmkl_intel_ilp64" "-lmkl_tbb_thread" "-lmkl_core" "--end-group"
// CHECK-MKL-LIN-SEQUENTIAL: "--start-group" "-lmkl_intel_ilp64" "-lmkl_sequential" "-lmkl_core" "--end-group"
// CHECK-MKL-LIN-CLUSTER: "--start-group" "-lmkl_intel_ilp64" "-lmkl_cdft_core" "-lmkl_scalapack_ilp64" "-lmkl_blacs_intelmpi_ilp64" "-lmkl_sequential" "-lmkl_core" "--end-group"
// CHECK-MKL-WIN-SYCL: link{{.*}} "-defaultlib:{{[^ ]+}}mkl{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}mkl_sycl.lib"
// CHECK-MKL-WIN: "-libpath:{{[^ ]+}}mkl{{/|\\\\}}lib{{/|\\\\}}intel64"
// CHECK-MKL-WIN-DPCPP: "-libpath:{{.*}}tbb{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}vc14"

// TBB tests
// RUN: env TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -tbb -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-TBB,CHECK-TBB-LIN %s
// RUN: env TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qtbb -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-TBB,CHECK-TBB-LIN %s
// RUN: env TBBROOT=/dummy/tbb \
// RUN: %clang_cl -Qtbb -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-TBB,CHECK-TBB-WIN %s
// CHECK-TBB-LIN: "-DPSTL_USE_PARALLEL_POLICIES=0" "-D_GLIBCXX_USE_TBB_PAR_BACKEND=0"
// CHECK-TBB: "-internal-isystem" "{{.*}}tbb{{/|\\\\}}include"
// CHECK-TBB-LIN: ld{{.*}} "-L{{.*}}tbb{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}gcc4.8" {{.*}} "-ltbb"
// CHECK-TBB-WIN: link{{.*}} "-libpath:{{.*}}tbb{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}vc14"

// DAAL tests
// Copy the libs created above from MKL
// RUN: mkdir -p %t_dir/dal/lib/intel64
// RUN: cp %t_dir/mkl/lib/intel64/libmkl_sycl.a %t_dir/dal/lib/intel64/libonedal_sycl.a
// RUN: cp %t_dir/mkl/lib/intel64/mkl_sycl.lib %t_dir/dal/lib/intel64/onedal_sycl.lib
// RUN: env DAALROOT=%t_dir/dal TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -daal -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-LIN,CHECK-DAAL-LIN-PARALLEL %s
// RUN: env DAALROOT=%t_dir/dal TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qdaal -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-LIN,CHECK-DAAL-LIN-PARALLEL %s
// RUN: env DAALROOT=%t_dir/dal TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qdaal -fsycl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-LIN,CHECK-DAAL-LIN-PARALLEL,CHECK-DAAL-SYCL,CHECK-DAAL-LIN-SYCL %s
// RUN: env DAALROOT=%t_dir/dal TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qdaal=parallel -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-LIN,CHECK-DAAL-LIN-PARALLEL %s
// RUN: env DAALROOT=%t_dir/dal TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qdaal=sequential -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-LIN,CHECK-DAAL-LIN-SEQUENTIAL %s
// RUN: env DAALROOT=%t_dir/dal TBBROOT=/dummy/tbb \
// RUN: %clang_cl -Qdaal -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-WIN-PARALLEL %s
// RUN: env DAALROOT=%t_dir/dal TBBROOT=/dummy/tbb \
// RUN: %clang_cl -Qdaal -fsycl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-WIN-PARALLEL,CHECK-DAAL-SYCL,CHECK-DAAL-WIN-SYCL %s
// RUN: env DAALROOT=%t_dir/dal TBBROOT=/dummy/tbb \
// RUN: %clang_cl -Qdaal=parallel -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-WIN-PARALLEL %s
// RUN: env DAALROOT=%t_dir/dal TBBROOT=/dummy/tbb \
// RUN: %clang_cl -Qdaal=sequential -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-WIN-SEQUENTIAL %s
// CHECK-DAAL-WIN-PARALLEL: clang{{.*}} "--dependent-lib=onedal_core" "--dependent-lib=onedal_thread"
// CHECK-DAAL-WIN-SEQUENTIAL: clang{{.*}} "--dependent-lib=onedal_core" "--dependent-lib=onedal_sequential"
// CHECK-DAAL: "-internal-isystem" "{{.*}}tbb{{/|\\\\}}include" "-internal-isystem" "{{.*}}dal{{/|\\\\}}include"
// CHECK-DAAL-WIN-SYCL: clang-offload-bundler{{.*}} "-inputs={{.*}}lib{{/|\\\\}}intel64{{/|\\\\}}onedal_sycl.lib" "-outputs=[[WINLIB:.+\.a]]" "-unbundle"
// CHECK-DAAL-WIN-SYCL: llvm-link{{.*}} "[[WINLIB]]"
// CHECK-DAAL-LIN-SYCL: clang-offload-bundler{{.*}} "-type=a" "-targets=sycl-spir64-unknown-unknown-sycldevice" "-inputs={{.*}}dal{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}libonedal_sycl.a" "-outputs=[[LINLIB:.+\.a]]" "-unbundle"
// CHECK-DAAL-LIN-SYCL: llvm-link{{.*}} "[[LINLIB]]"
// CHECK-DAAL-SYCL: llvm-spirv{{.*}}
// CHECK-DAAL-SYCL: clang-offload-wrapper{{.*}}
// CHECK-DAAL-SYCL: llc{{.*}}
// CHECK-DAAL-LIN: ld{{.*}} "-L{{.*}}tbb{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}gcc4.8" "-L{{.*}}dal{{/|\\\\}}lib{{/|\\\\}}intel64"
// CHECK-DAAL-LIN-PARALLEL: "--start-group" "-lonedal_core" "-lonedal_thread" "--end-group" "-ltbb"
// CHECK-DAAL-LIN-SEQUENTIAL: "--start-group" "-lonedal_core" "-lonedal_sequential" "--end-group" "-ltbb"
// CHECK-DAAL-WIN-SYCL: link{{.*}} "-defaultlib:{{.*}}dal{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}onedal_sycl.lib"
// CHECK-DAAL-WIN: "-libpath:{{.*}}tbb{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}vc14" "-libpath:{{.*}}dal{{/|\\\\}}lib{{/|\\\\}}intel64"

// Check phases for -qmkl and objects
// RUN: touch %t.o
// RUN: %clang -target x86_64-unknown-linux-gnu -fsycl -fno-sycl-device-lib=all -qmkl %t.o -### -ccc-print-phases 2>&1 \
// RUN:  | FileCheck -check-prefix=MKL-SHARED-OBJ-PHASES %s
// MKL-SHARED-OBJ-PHASES: 0: input, "{{.*}}", object, (host-sycl)
// MKL-SHARED-OBJ-PHASES: 1: clang-offload-unbundler, {0}, object, (host-sycl)
// MKL-SHARED-OBJ-PHASES: 2: linker, {1}, image, (host-sycl)
// MKL-SHARED-OBJ-PHASES: 3: linker, {1}, ir, (device-sycl)
// MKL-SHARED-OBJ-PHASES: 4: sycl-post-link, {3}, tempfiletable, (device-sycl)
// MKL-SHARED-OBJ-PHASES: 5: file-table-tform, {4}, tempfilelist, (device-sycl)
// MKL-SHARED-OBJ-PHASES: 6: llvm-spirv, {5}, tempfilelist, (device-sycl)
// MKL-SHARED-OBJ-PHASES: 7: file-table-tform, {4, 6}, tempfiletable, (device-sycl)
// MKL-SHARED-OBJ-PHASES: 8: clang-offload-wrapper, {7}, object, (device-sycl)
// MKL-SHARED-OBJ-PHASES: 9: offload, "host-sycl (x86_64-unknown-linux-gnu)" {2}, "device-sycl (spir64-unknown-unknown-sycldevice)" {8}, image

// AC Types tests (-qactypes)
// RUN: env INTELFPGAOCLSDKROOT=/dummy/actypes \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -qactypes -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-ACTYPES,CHECK-ACTYPES-LIN %s
// RUN: env INTELFPGAOCLSDKROOT=/dummy/actypes \
// RUN: %clang_cl -Qactypes -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-ACTYPES,CHECK-ACTYPES-WIN %s
// CHECK-ACTYPES-WIN: "--dependent-lib=dspba_mpir" "--dependent-lib=dspba_mpfr" "--dependent-lib=hls_fixed_point_math_x86" "--dependent-lib=hls_vpfp_library"
// CHECK-ACTYPES: "-internal-isystem" "{{.*}}actypes{{/|\\\\}}include"
// CHECK-ACTYPES-LIN: ld{{.*}} "-L{{.*}}actypes{{/|\\\\}}host{{/|\\\\}}linux64{{/|\\\\}}lib" {{.*}} "-ldspba_mpir" "-ldspba_mpfr" "-lhls_fixed_point_math_x86" "-lhls_vpfp_library"
// CHECK-ACTYPES-WIN: link{{.*}} "-libpath:{{.*}}actypes{{/|\\\\}}host{{/|\\\\}}windows64{{/|\\\\}}lib"
