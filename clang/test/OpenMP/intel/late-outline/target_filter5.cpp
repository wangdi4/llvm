// INTEL_COLLAB

// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple x86_64-pc-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

// Verifies that routines seen after the first time are also marked
// target-declare on the deferred list.

template <int i>
void bar()
{
}

void foo()
{
  bar<1>();
  #pragma omp target
  {
    bar<1>();
    bar<2>();
  }
}

//CHECK: define {{.*}}foo{{.*}}#[[CONTAINS_TARGET:[0-9]+]]
//CHECK: define {{.*}}bar{{.*}}#[[TARGET_DECLARE:[0-9]+]]
//CHECK: define {{.*}}bar{{.*}}#[[TARGET_DECLARE]]

//CHECK: attributes #[[CONTAINS_TARGET]] = {{.*}}"contains-openmp-target"="true"
//CHECK: attributes #[[TARGET_DECLARE]] = {{.*}}"openmp-target-declare"="true"

// end INTEL_COLLAB
