// INTEL_COLLAB
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple x86_64-pc-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

#pragma omp declare target
//CHECK: @glob = dso_local target_declare{{.*}}
int glob;

//CHECK: define {{.*}}barv() #[[HAS_BOTH:[0-9]+]]
int bar() {
#pragma omp target map(glob)
  {
    glob = 10;
  }
  return glob;
}
#pragma omp end declare target

//CHECK: define {{.*}}funcv() #[[CONTAINS_TARGET:[0-9]+]]
void func() {
#pragma omp target map(glob)
  {
    glob += bar();
  }
}
//CHECK: attributes #[[HAS_BOTH]] = {{.*}}"contains-openmp-target"="true"{{.*}}"openmp-target-declare"="true"
//CHECK: attributes #[[CONTAINS_TARGET]] = {{.*}}"contains-openmp-target"="true"

// end INTEL_COLLAB
