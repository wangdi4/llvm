///
/// Perform several driver tests for OpenMP offloading involving static libs
///
// REQUIRES: clang-driver
// REQUIRES: x86-registered-target
// UNSUPPORTED: linux

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
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.lo -### %t.o 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB
// STATIC_LIB: clang-offload-bundler{{.*}} "-type=a" {{.*}} "-inputs={{.*}}" "-outputs=[[LIBDEVICE:.+\.a]]"
// STATIC_LIB: llvm-link{{.*}} "[[LIBDEVICE]]"
// STATIC_LIB: ld{{.*}}

/// ###########################################################################

/// test behaviors of fat static lib with multiple objects
// RUN: touch %t.a
// RUN: touch %t-1.o
// RUN: touch %t-2.o
// RUN: touch %t-3.o
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 %t.a -### %t-1.o %t-2.o %t-3.o 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_MULTI_O
// STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=o" {{.*}} "-outputs=[[OBJHOST1:.+\.o]],[[OBJDEVICE1:.+\.o]]"
// STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=o" {{.*}} "-outputs=[[OBJHOST2:.+\.o]],[[OBJDEVICE2:.+\.o]]"
// STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=o" {{.*}} "-outputs=[[OBJHOST3:.+\.o]],[[OBJDEVICE3:.+\.o]]"
// STATIC_LIB_MULTI_O: ld{{.*}} "[[OBJHOST1]]" "[[OBJHOST2]]" "[[OBJHOST3]]"
// STATIC_LIB_MULTI_O: clang-offload-deps{{.*}}
// STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=a" {{.*}} "-outputs=[[LIBDEVICE1:.+\.a]]"
// STATIC_LIB_MULTI_O: llvm-link{{.*}} "[[OBJDEVICE1]]" "[[OBJDEVICE2]]" "[[OBJDEVICE3]]"{{.*}} "-o" "[[LINKDEVICEOBJ:.+\.bc]]"
// STATIC_LIB_MULTI_O: llvm-link{{.*}} "[[LINKDEVICEOBJ]]" "[[LIBDEVICE1]]"

/// ###########################################################################

/// test behaviors of fat static lib from source
// RUN: touch %t.a
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64  %t.a -### %s 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_SRC
// STATIC_LIB_SRC: clang-offload-bundler{{.*}} "-type=a" {{.*}} "-outputs=[[LIBDEVICE:.+\.a]]"
// STATIC_LIB_SRC: llvm-link{{.*}} "[[LIBDEVICE]]"

/// ###########################################################################

/// test behaviors of static lib with no source/object
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.a -### 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_NOSRC
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -L/dummy/dir %t.lo -### 2>&1 \
// RUN:   | FileCheck %s -check-prefix=STATIC_LIB_NOSRC
// STATIC_LIB_NOSRC: clang-offload-bundler{{.*}} "-type=ao" "-targets=host-x86_64-unknown-linux-gnu" "-inputs={{.*}}" "-check-section"
// STATIC_LIB_NOSRC: clang-offload-bundler{{.*}} "-type=a" "-targets=openmp-spir64" "-inputs={{.*}}" "-outputs=[[DEVICELIB:.+\.a]]" "-unbundle"
// STATIC_LIB_NOSRC: llvm-link{{.*}} "[[DEVICELIB]]" "-o" "[[OUTFILE:.+\.bc]]"
// STATIC_LIB_NOSRC: sycl-post-link{{.*}} "-o" "[[BCFILE:.+\.bc]]" "[[OUTFILE]]"
// STATIC_LIB_NOSRC: llvm-spirv{{.*}} "-o" "[[OUTFILE2:.+\.spv]]" {{.*}} "[[BCFILE]]"
// STATIC_LIB_NOSRC: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[BCFILE2:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OUTFILE2]]"
// STATIC_LIB_NOSRC: clang{{.*}} "-o" "[[FINALOBJ:.+\.o]]" {{.*}} "[[BCFILE2]]"
// STATIC_LIB_NOSRC: ld{{.*}} "-L/dummy/dir" {{.*}} "[[FINALOBJ]]"


/// ###########################################################################
// Build a fat static lib
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -Qiopenmp -Qopenmp-targets=spir64 %t1.cpp -c -Fo%t1_bundle.obj
// RUN: mkdir -p %t_dir
// RUN: lib -out:%t_dir/liboffload.lib %t1_bundle.obj
//
/// Test LIB env variable library search
// RUN: env LIB=%t_dir \
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -Qiopenmp -Qopenmp-targets=spir64 %s liboffload.lib -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=STATIC_LIB_WIN_LIBENV
// STATIC_LIB_WIN_LIBENV: clang-offload-bundler{{.*}} "-type=a" "-targets=openmp-spir64" "-inputs={{.*}}liboffload.lib" "-outputs=[[DEVICELIB:.+\.a]]" "-unbundle"
// STATIC_LIB_WIN_LIBENV: llvm-link{{.*}} "[[DEVICELIB]]" "-o" "[[LINKOUT:.+\.bc]]"
// STATIC_LIB_WIN_LIBENV: sycl-post-link{{.*}} "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-ir-output-only" "-O2" "-spec-const=rt" "-o" "[[POSTLINKBC:.+\.bc]]" "[[LINKOUT]]"
// STATIC_LIB_WIN_LIBENV: llvm-spirv{{.*}} "-o" "[[SPIRVOUT:.+\.spv]]" "-spirv-ext={{.*}}" "[[POSTLINKBC]]"
// STATIC_LIB_WIN_LIBENV: clang-offload-wrapper{{.*}} "-host" "x86_64-pc-windows-msvc{{.*}}" "-o" "[[WRAPPEROUT:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[SPIRVOUT]]"
// STATIC_LIB_WIN_LIBENV: clang{{.*}} "-o" "[[WRAPOBJ:.+\.obj]]" "-x" "ir" "[[WRAPPEROUT]]"
// STATIC_LIB_WIN_LIBENV: link{{.*}} "liboffload.lib"{{.*}}"[[WRAPOBJ]]"
