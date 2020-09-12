// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline -DUSEHDR \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -isystem %S/late-outline/Inputs -emit-llvm -o - %s | FileCheck %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline -DUSEHDR \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -isystem %S/late-outline/Inputs -emit-llvm-bc -o %t-host.bc %s
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline -DUSEHDR \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc \
// RUN:  -isystem %S/late-outline/Inputs -emit-llvm -o - %s | \
// RUN:  FileCheck %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -isystem %S/late-outline/Inputs -emit-llvm -o - %s | FileCheck %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -isystem %S/late-outline/Inputs -emit-llvm-bc -o %t-host.bc %s
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc \
// RUN:  -isystem %S/late-outline/Inputs -emit-llvm -o - %s | \
// RUN:  FileCheck %s

void one(void) {}
void two(void) {}

#if USEHDR
// Without the builtin, just an external call.
#include <omp.h>
#else
// Excepted future contents of the header.
#pragma omp begin declare variant match(device={kind(host)})
static int omp_is_initial_device(void) { return 1; }
#pragma omp end declare variant
#pragma omp begin declare variant match(device={kind(nohost)})
static int omp_is_initial_device(void) { return 0; }
#pragma omp end declare variant
#endif

//CHECK-LABEL: foo
void foo(void) {
  #pragma omp target
  //CHECK: is_initial_device
  if (omp_is_initial_device())
    //CHECK: call {{.*}}one
    one();
  else
    //CHECK: call {{.*}}two
    two();
}
