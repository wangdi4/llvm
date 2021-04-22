///
/// Perform several driver tests for OpenMP offloading involving static libs
///
// REQUIRES: clang-driver
// REQUIRES: x86-registered-target
// UNSUPPORTED: windows

/// ###########################################################################

/// test behaviors of passing a fat static lib
// Build a fat static lib that will be used for all tests
// RUN: echo "void foo(void) {}" > %t1.cpp
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 %t1.cpp -c -o %t1_bundle.o
// RUN: llvm-ar cr %t.a %t1_bundle.o
// RUN: llvm-ar cr %t.lo %t1_bundle.o
// RUN: llvm-ar cr %t_2.a %t1_bundle.o
//
// RUN: touch %t.a
// RUN: touch %t.lo
// RUN: touch %t.o
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fno-openmp-device-lib=all -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.a -### %t.o 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB -DINPUTA=%t.a -DINPUTO=%t.o
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fno-openmp-device-lib=all -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.lo -### %t.o 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB -DINPUTA=%t.lo -DINPUTO=%t.o
// STATIC_LIB: clang-offload-bundler{{.*}} "-type=o" {{.*}} "-inputs=[[INPUTO]]" "-outputs=[[HOSTOBJ:.+\.o]],{{.+\.o}}"
// STATIC_LIB: clang-offload-bundler{{.*}} "-type=a" {{.*}} "-inputs=[[INPUTA]]" "-outputs=[[OUTLIB:.+\.a]]"
// STATIC_LIB: llvm-link{{.*}} "[[OUTLIB]]"
// STATIC_LIB: ld{{.*}} "[[INPUTA]]" "[[HOSTOBJ]]"

/// ###########################################################################

/// test behaviors of fat static lib with multiple objects
// RUN: touch %t.a
// RUN: touch %t-1.o
// RUN: touch %t-2.o
// RUN: touch %t-3.o
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 %t.a -### %t-1.o %t-2.o %t-3.o 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_MULTI_O
// STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=o" {{.*}} "-inputs={{.+}}-1.o"
// STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=o" {{.*}} "-inputs={{.+}}-2.o"
// STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=o" {{.*}} "-inputs={{.+}}-3.o"
// STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=a"
// STATIC_LIB_MULTI_O: llvm-link{{.*}}

/// ###########################################################################

/// test behaviors of fat static lib from source
// RUN: touch %t.a
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64  %t.a -### %s 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_SRC
// STATIC_LIB_SRC: clang-offload-bundler{{.*}} "-type=a"
// STATIC_LIB_SRC: llvm-link{{.*}}

/// ###########################################################################

/// test behaviors of -Wl,--whole-archive staticlib.a -Wl,--no-whole-archive
/// also test behaviors of -Wl,@arg with the above arguments
// RUN: touch %t.a
// RUN: touch %t_2.a
// RUN: touch %t.o
// RUN: echo "--whole-archive %t.a %t_2.a --no-whole-archive" > %t.arg
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.o -Wl,--whole-archive %t.a %t_2.a -Wl,--no-whole-archive -fno-openmp-device-lib=all -### 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=WHOLE_STATIC_LIB,WHOLE_STATIC_LIB_1
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.o -Wl,@%t.arg -fno-openmp-device-lib=all -### 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=WHOLE_STATIC_LIB
// WHOLE_STATIC_LIB: clang-offload-bundler{{.*}} "-type=o" {{.*}}
// WHOLE_STATIC_LIB: clang-offload-bundler{{.*}} "-type=a" {{.*}} "-inputs=[[INPUTA:.+\.a]]" "-outputs={{.*}}"
// WHOLE_STATIC_LIB: clang-offload-bundler{{.*}} "-type=a" {{.*}} "-inputs=[[INPUTB:.+\.a]]" "-outputs={{.*}}"
// WHOLE_STATIC_LIB: llvm-link{{.*}}
// WHOLE_STATIC_LIB: llvm-spirv{{.*}}
// WHOLE_STATIC_LIB: clang-offload-wrapper{{.*}}
// WHOLE_STATIC_LIB: clang{{.*}}
// WHOLE_STATIC_LIB_1: ld{{.*}} "--whole-archive" "[[INPUTA]]" "[[INPUTB]]" "--no-whole-archive"

/// ###########################################################################

/// test behaviors of static lib with no source/object
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.a -fno-openmp-device-lib=all -### 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_NOSRC -DINPUTLIB=%t.a
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.lo -fno-openmp-device-lib=all -### 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_NOSRC -DINPUTLIB=%t.lo
// STATIC_LIB_NOSRC: clang-offload-bundler{{.*}} "-type=ao" "-targets=host-x86_64-unknown-linux-gnu" "-inputs=[[INPUTLIB]]" "-check-section"
// STATIC_LIB_NOSRC: clang-offload-bundler{{.*}} "-type=a" "-targets=openmp-spir64" "-inputs=[[INPUTLIB]]" "-outputs=[[OUTLIB:.+\.a]]" "-unbundle"
// STATIC_LIB_NOSRC: llvm-link{{.*}} "[[OUTLIB]]" "-o" "[[OUTFILE:.+\.bc]]"
// STATIC_LIB_NOSRC: sycl-post-link{{.*}} "-o" "[[BCFILE:.+\.bc]]" "[[OUTFILE]]"
// STATIC_LIB_NOSRC: llvm-spirv{{.*}} "-o" "[[OUTFILE2:.+\.spv]]" {{.*}} "[[BCFILE]]"
// STATIC_LIB_NOSRC: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[BCFILE2:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OUTFILE2]]"
// STATIC_LIB_NOSRC: clang{{.*}} "-o" "[[FINALOBJ:.+\.o]]" {{.*}} "[[BCFILE2]]"
// STATIC_LIB_NOSRC: ld{{.*}} "-L/dummy/dir" {{.*}} "[[INPUTLIB]]"{{.*}}"[[FINALOBJ]]"
