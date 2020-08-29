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
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.a -### %t.o 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB -DINPUTA=%t.a -DINPUTO=%t.o
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.lo -### %t.o 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB -DINPUTA=%t.lo -DINPUTO=%t.o
// STATIC_LIB: ld{{(.exe)?}}" "-r" "-o" "[[INPUTLD:[^ ]+\.o]]" {{.*}} "-L/dummy/dir"{{.*}} "[[INPUTO]]" "[[INPUTA]]"
// STATIC_LIB: clang-offload-bundler{{.*}} "-type=oo" {{.*}} "-inputs=[[INPUTLD]]" "-outputs=[[LISTFILE:.+\.txt]]"
// STATIC_LIB: llvm-link{{.*}} "@[[LISTFILE]]"
// STATIC_LIB: ld{{.*}} "[[INPUTA]]" "[[INPUTO]]"

/// ###########################################################################

/// test behaviors of fat static lib with multiple objects
// RUN: touch %t.a
// RUN: touch %t-1.o
// RUN: touch %t-2.o
// RUN: touch %t-3.o
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 %t.a -### %t-1.o %t-2.o %t-3.o 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_MULTI_O
// STATIC_LIB_MULTI_O: ld{{(.exe)?}}" "-r" "-o" {{.*}} "[[INPUT:.+\-1.o]]" "[[INPUT:.+\-2.o]]" "[[INPUT:.+\-3.o]]" "[[INPUT:.+\.a]]"
// STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=oo"
// STATIC_LIB_MULTI_O: llvm-link{{.*}} "@{{.*}}"

/// ###########################################################################

/// test behaviors of fat static lib from source
// RUN: touch %t.a
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64  %t.a -### %s 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_SRC
// STATIC_LIB_SRC: ld{{(.exe)?}}" "-r" "-o" {{.*}} "[[INPUT:.+\.a]]"
// STATIC_LIB_SRC: clang-offload-bundler{{.*}} "-type=oo"
// STATIC_LIB_SRC: llvm-link{{.*}} "@{{.*}}"

/// ###########################################################################

/// test behaviors of -Wl,--whole-archive staticlib.a -Wl,--no-whole-archive
/// also test behaviors of -Wl,@arg with the above arguments
// RUN: touch %t.a
// RUN: touch %t_2.a
// RUN: touch %t.o
// RUN: echo "--whole-archive %t.a %t_2.a --no-whole-archive" > %t.arg
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.o -Wl,--whole-archive %t.a %t_2.a -Wl,--no-whole-archive -### 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=WHOLE_STATIC_LIB,WHOLE_STATIC_LIB_1
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.o -Wl,@%t.arg -### 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=WHOLE_STATIC_LIB,WHOLE_STATIC_LIB_2 -DARGFILE=%t.arg
// WHOLE_STATIC_LIB: ld{{(.exe)?}}" "-r" "-o" "[[INPUT:.+\.o]]" "{{.*}}crt1.o" "{{.*}}crti.o" "-L/dummy/dir" {{.*}} "[[INPUTO:.+\.o]]" "--whole-archive" "[[INPUTA:.+\.a]]" "[[INPUTB:.+\.a]]" "--no-whole-archive" "{{.*}}crtn.o"
// WHOLE_STATIC_LIB: clang-offload-bundler{{.*}} "-type=oo" {{.*}} "-inputs=[[INPUT]]"
// WHOLE_STATIC_LIB: llvm-link{{.*}} "@{{.*}}"
// WHOLE_STATIC_LIB: llvm-spirv{{.*}}
// WHOLE_STATIC_LIB: clang-offload-wrapper{{.*}}
// WHOLE_STATIC_LIB: clang{{.*}}
// WHOLE_STATIC_LIB_1: ld{{.*}} "[[INPUTO]]" "--whole-archive" "[[INPUTA]]" "[[INPUTB]]" "--no-whole-archive"
// WHOLE_STATIC_LIB_2: ld{{.*}} "[[INPUTO]]" "@[[ARGFILE]]"

/// ###########################################################################

/// test behaviors of static lib with no source/object
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.a -### 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_NOSRC -DINPUTLIB=%t.a
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.lo -### 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_NOSRC -DINPUTLIB=%t.lo
// STATIC_LIB_NOSRC: clang-offload-bundler{{.*}} "-type=ao" "-targets=host-x86_64-unknown-linux-gnu" "-inputs=[[INPUTLIB]]" "-check-section"
// STATIC_LIB_NOSRC: ld{{.*}} "-r" "-o" "[[PARTIALOBJ:.+\.o]]" "{{.*}}crt1.o" {{.*}} "-L/dummy/dir" {{.*}} "[[INPUTLIB]]"
// STATIC_LIB_NOSRC: clang-offload-bundler{{.*}} "-type=oo" "-targets=openmp-spir64" "-inputs=[[PARTIALOBJ]]" "-outputs=[[DEVICELIST:.+\.txt]]" "-unbundle"
// STATIC_LIB_NOSRC: llvm-link{{.*}} "@[[DEVICELIST]]" "-o" "[[OUTFILE:.+\.out]]"
// STATIC_LIB_NOSRC: sycl-post-link{{.*}} "-o" "[[BCFILE:.+\.bc]]" "[[OUTFILE]]"
// STATIC_LIB_NOSRC: llvm-spirv{{.*}} "-o" "[[OUTFILE2:.+\.out]]" {{.*}} "[[BCFILE]]"
// STATIC_LIB_NOSRC: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[BCFILE2:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OUTFILE2]]"
// STATIC_LIB_NOSRC: clang{{.*}} "-o" "[[FINALOBJ:.+\.o]]" {{.*}} "[[BCFILE2]]"
// STATIC_LIB_NOSRC: ld{{.*}} "-L/dummy/dir" {{.*}} "[[INPUTLIB]]" "[[FINALOBJ]]"
