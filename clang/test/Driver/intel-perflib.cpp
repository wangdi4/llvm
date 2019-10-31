/// Covers the Intel performance library options for IPP, MKL, TBB, DAAL

// IPP tests
// RUN: env IPPROOT=/dummy/ipp \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -ipp -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP,CHECK-IPP-LIN,CHK-IPP-LIN-COMMON %s
// RUN: env IPPROOT=/dummy/ipp \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -ipp=common -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP,CHECK-IPP-LIN,CHK-IPP-LIN-COMMON %s
// RUN: env IPPROOT=/dummy/ipp \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -ipp=nonpic -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP,CHECK-IPP-LIN,CHECK-IPP-LIN-NONPIC %s
// RUN: env IPPCRYPTOROOT=/dummy/ippcp \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -ipp=crypto -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-IPP-CRYPTO,CHECK-IPP-LIN-CRYPTO %s
// RUN: env IPPCRYPTOROOT=/dummy/ippcp \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -ipp=nonpic_crypto -### %s 2>&1 \
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
// CHECK-IPP-WIN: clang{{.*}} "--dependent-lib=ippcv" "--dependent-lib=ippch" "--dependent-lib=ippcc" "--dependent-lib=ippdc" "--dependent-lib=ippi" "--dependent-lib=ipps" "--dependent-lib=ippvm" "--dependent-lib=ippcore"
// CHECK-IPP-WIN-CRYPTO: clang{{.*}} "--dependent-lib=ippcp"
// CHECK-IPP: "-internal-isystem" "{{.*}}ipp{{/|\\\\}}include"
// CHECK-IPP-CRYPTO: "-internal-isystem" "{{.*}}ippcp{{/|\\\\}}include"
// CHECK-IPP-LIN-COMMON: ld{{.*}} "-L{{.*}}/ipp{{/|\\\\}}lib{{/|\\\\}}intel64"
// CHECK-IPP-LIN-CRYPTO: ld{{.*}} "-L{{.*}}/ippcp{{/|\\\\}}lib{{/|\\\\}}intel64"
// CHECK-IPP-LIN-NONPIC: ld{{.*}} "-L{{.*}}/ipp{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}nonpic"
// CHECK-IPP-LIN-NONPIC-CRYPTO: ld{{.*}} "-L{{.*}}/ippcp{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}nonpic"
// CHECK-IPP-LIN: "-Bstatic" "-lippcv" "-lippch" "-lippcc" "-lippdc" "-lippi" "-lipps" "-lippvm" "-lippcore" "-Bdynamic"
// CHECK-IPP-LIN-CRYPTO: "-Bstatic" "-lippcp" "-Bdynamic"
// CHECK-IPP-LIN-NONPIC-CRYPTO: "-Bstatic" "-lippcp" "-Bdynamic"
// CHECK-IPP-WIN: link{{.*}} "-libpath:{{.*}}ipp{{/|\\\\}}lib{{/|\\\\}}intel64"
// CHECK-IPP-WIN-CRYPTO: link{{.*}} "-libpath:{{.*}}ippcp{{/|\\\\}}lib{{/|\\\\}}intel64"

// MKL tests
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -mkl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-LIN,CHK-MKL-LIN-PARALLEL %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -mkl -tbb -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-LIN,CHK-MKL-LIN-TBB %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -mkl -fsycl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-LIN,CHK-MKL-LIN-PARALLEL,CHK-MKL-SYCL,CHK-MKL-LIN-SYCL %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -mkl=parallel -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-LIN,CHK-MKL-LIN-PARALLEL %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fopenmp -mkl=parallel -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-LIN,CHK-MKL-LIN-PARALLEL-OMP %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -mkl=sequential -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-LIN,CHECK-MKL-LIN-SEQUENTIAL %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -mkl=cluster -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-LIN,CHECK-MKL-LIN-CLUSTER %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clang_cl -Qmkl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-WIN,CHK-MKL-WIN-PARALLEL %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clang_cl -Qmkl -Qtbb -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-WIN,CHK-MKL-WIN-TBB %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clang_cl -Qmkl:parallel -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-WIN,CHK-MKL-WIN-PARALLEL %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clang_cl -Qmkl:parallel -openmp -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-WIN,CHK-MKL-WIN-PARALLEL-OMP %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clang_cl -Qmkl -fsycl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-WIN,CHK-MKL-WIN-PARALLEL,CHK-MKL-SYCL,CHK-MKL-WIN-SYCL %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clang_cl -Qmkl -Qtbb -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-WIN,CHK-MKL-WIN-TBB %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clang_cl -Qmkl:sequential -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-WIN,CHK-MKL-WIN-PARALLEL %s
// RUN: env MKLROOT=/dummy/mkl \
// RUN: %clang_cl -Qmkl:cluster -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-MKL,CHECK-MKL-WIN,CHK-MKL-WIN-CLUSTER %s
// CHECK-MKL-WIN-SYCL: clang-offload-bundler{{.*}} "-inputs={{.*}}lib{{/|\\\\}}intel64{{/|\\\\}}libmkl_sycl{{.*}}" {{.*}} "-unbundle"
// CHECK-MKL-SYCL: llvm-link{{.*}}
// CHECK-MKL-SYCL: llvm-spirv{{.*}}
// CHECK-MKL-SYCL: clang-offload-wrapper{{.*}}
// CHECK-MKL-SYCL: llc{{.*}}
// CHECK-MKL: clang{{.*}} "-internal-isystem" "{{.*}}mkl{{/|\\\\}}include{{/|\\\\}}intel64{{/|\\\\}}lp64"
// CHECK-MKL-LIN: ld{{.*}} "-L{{.*}}mkl{{/|\\\\}}lib{{/|\\\\}}intel64"
// CHECK-MKL-WIN-PARALLEL: clang{{.*}} "--dependent-lib=mkl_intel_lp64" "--dependent-lib=mkl_core"
// CHECK-MKL-WIN-PARALLEL-OMP: clang{{.*}} "--dependent-lib=mkl_intel_lp64" "--dependent-lib=mkl_intel_thread" "--dependent-lib=mkl_core"
// CHECK-MKL-WIN-TBB: clang{{.*}} "--dependent-lib=mkl_intel_lp64" "--dependent-lib=mkl_tbb_thread" "--dependent-lib=mkl_core"
// CHECK-MKL-WIN-CLUSTER: clang{{.*}} "--dependent-lib=mkl_intel_lp64" "--dependent-lib=mkl_cdft_core" "--dependent-lib=mkl_scalapack_lp64" "--dependent-lib=mkl_blacs_intelmpi_lp64" "--dependent-lib=mkl_sequential" "--dependent-lib=mkl_core"
// CHECK-MKL-LIN-PARALLEL: "-Bstatic" "--start-group" "-lmkl_intel_lp64" "-lmkl_core" "--end-group" "-Bdynamic"
// CHECK-MKL-LIN-PARALLEL-OMP: "-Bstatic" "--start-group" "-lmkl_intel_lp64" "-lmkl_intel_thread" "-lmkl_core" "--end-group" "-Bdynamic"
// CHECK-MKL-LIN-TBB: "-Bstatic" "--start-group" "-lmkl_intel_lp64" "-lmkl_core" "--end-group" "-Bdynamic"
// CHECK-MKL-LIN-SEQUENTIAL: "-Bstatic" "--start-group" "-lmkl_intel_lp64" "-lmkl_sequential" "-lmkl_core" "--end-group" "-Bdynamic"
// CHECK-MKL-LIN-CLUSTER: "-Bstatic" "--start-group" "-lmkl_intel_lp64" "-lmkl_cdft_core" "-lmkl_scalapack_lp64" "-lmkl_blacs_intelmpi_lp64" "-lmkl_sequential" "-lmkl_core" "--end-group" "-Bdynamic"
// CHECK-MKL-WIN-SYCL: link{{.*}} "-defaultlib:{{.*}}mkli{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}libmkl_sycl.lib"
// CHECK-MKL-WIN: "-libpath:{{.*}}mkl{{/|\\\\}}lib{{/|\\\\}}intel64"

// TBB tests
// RUN: env TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -tbb -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-TBB,CHECK-TBB-LIN %s
// RUN: env TBBROOT=/dummy/tbb \
// RUN: %clang_cl -Qtbb -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-TBB,CHECK-TBB-WIN %s
// CHECK-TBB: clang{{.*}} "-internal-isystem" "{{.*}}tbb{{/|\\\\}}include"
// CHECK-TBB-LIN: ld{{.*}} "-L{{.*}}tbb{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}gcc4.8" {{.*}} "-Bstatic" "-ltbb" "-Bdynamic"
// CHECK-TBB-WIN: link{{.*}} "-libpath:{{.*}}tbb{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}vc14"

// DAAL tests
// RUN: env DAALROOT=/dummy/daal TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -daal -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-LIN,CHECK-DAAL-LIN-PARALLEL %s
// RUN: env DAALROOT=/dummy/daal TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -daal -fsycl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-LIN,CHECK-DAAL-LIN-PARALLEL,CHECK-DAAL-SYCL,CHECK-DAAL-LIN-SYCL %s
// RUN: env DAALROOT=/dummy/daal TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -daal=parallel -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-LIN,CHECK-DAAL-LIN-PARALLEL %s
// RUN: env DAALROOT=/dummy/daal TBBROOT=/dummy/tbb \
// RUN: %clangxx -target x86_64-unknown-linux-gnu -daal=sequential -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-LIN,CHECK-DAAL-LIN-SEQUENTIAL %s
// RUN: env DAALROOT=/dummy/daal TBBROOT=/dummy/tbb \
// RUN: %clang_cl -Qdaal -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-WIN-PARALLEL %s
// RUN: env DAALROOT=/dummy/daal TBBROOT=/dummy/tbb \
// RUN: %clang_cl -Qdaal -fsycl -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-WIN-PARALLEL,CHECK-DAAL-SYCL,CHECK-DAAL-WIN-SYCL %s
// RUN: env DAALROOT=/dummy/daal TBBROOT=/dummy/tbb \
// RUN: %clang_cl -Qdaal=parallel -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-WIN-PARALLEL %s
// RUN: env DAALROOT=/dummy/daal TBBROOT=/dummy/tbb \
// RUN: %clang_cl -Qdaal=sequential -### %s 2>&1 \
// RUN: | FileCheck -check-prefixes=CHECK-DAAL,CHECK-DAAL-WIN-SEQUENTIAL %s
// CHECK-DAAL-WIN-PARALLEL: clang{{.*}} "--dependent-lib=daal_core" "--dependent-lib=daal_thread"
// CHECK-DAAL-WIN-SEQUENTIAL: clang{{.*}} "--dependent-lib=daal_core" "--dependent-lib=daal_sequential"
// CHECK-DAAL: "-internal-isystem" "{{.*}}tbb{{/|\\\\}}include" "-internal-isystem" "{{.*}}daal{{/|\\\\}}include"
// CHECK-DAAL-WIN-SYCL: clang-offload-bundler{{.*}} "-inputs={{.*}}lib{{/|\\\\}}intel64{{/|\\\\}}libdaal_sycl.lib" {{.*}} "-unbundle"
// CHECK-DAAL-LIN-SYCL: ld{{.*}} "-r" {{.*}} "{{.*}}daal{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}libdaal_sycl.a"
// CHECK-DAAL-SYCL: llvm-link{{.*}}
// CHECK-DAAL-SYCL: llvm-spirv{{.*}}
// CHECK-DAAL-SYCL: clang-offload-wrapper{{.*}}
// CHECK-DAAL-SYCL: llc{{.*}}
// CHECK-DAAL-LIN: ld{{.*}} "-L{{.*}}tbb{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}gcc4.8" "-L{{.*}}daal{{/|\\\\}}lib{{/|\\\\}}intel64"
// CHECK-DAAL-LIN-PARALLEL: "-Bstatic" "--start-group" "-ldaal_core" "-ldaal_thread" "--end-group" "-Bdynamic" "-Bstatic" "-ltbb" "-Bdynamic"
// CHECK-DAAL-LIN-SEQUENTIAL: "-Bstatic" "--start-group" "-ldaal_core" "-ldaal_sequential" "--end-group" "-Bdynamic" "-Bstatic" "-ltbb" "-Bdynamic"
// CHECK-DAAL-WIN-SYCL: link{{.*}} "-defaultlib:{{.*}}daal{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}libdaal_sycl.lib"
// CHECK-DAAL-WIN: "-libpath:{{.*}}tbb{{/|\\\\}}lib{{/|\\\\}}intel64{{/|\\\\}}vc14" "-libpath:{{.*}}daal{{/|\\\\}}lib{{/|\\\\}}intel64"
