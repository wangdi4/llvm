// INTEL_COLLAB

// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s \
// RUN:  -o %t-x86_64-host.bc
//
// RUN: %clang_cc1 -verify -triple x86_64-pc-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-x86_64-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s \
// RUN:  -o %t-spir64-host.bc
//
// RUN: %clang_cc1 -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-spir64-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s --check-prefix SPIRV
//
// expected-no-diagnostics

// Verifies that varargs are not compiled on targets that do not support
// it (SPIR-V) but work okay on those that do.

int MY_printf(char const* const _Format, ...) {
  __builtin_va_list _ArgList;
  __builtin_va_start(_ArgList, _Format);
  __builtin_va_end(_ArgList);
  return 0;
}

//CHECK: define {{.*}}foo{{.*}}#[[CONTAINS_TARGET:[0-9]+]]
//SPIRV: define {{.*}}foo{{.*}}#[[CONTAINS_TARGET:[0-9]+]]
void foo()
{
  int i = 9;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK: call{{.*}}MY_printf
  //CHECK: "DIR.OMP.END.TARGET"()
  #pragma omp target
  MY_printf("%d\n", i);
}

#pragma omp declare target
int ANOTHER_printf(char const* const _Format, ...) {
  __builtin_va_list _ArgList;
  __builtin_va_start(_ArgList, _Format);
  __builtin_va_end(_ArgList);
  return 0;
}
#pragma omp end declare target

//SPIRV-NOT: define {{.*}}printf
//CHECK: define {{.*}}MY_printf{{.*}}#[[TARGET_DECLARE:[0-9]+]]
//CHECK: define {{.*}}ANOTHER_printf{{.*}}#[[TARGET_DECLARE:[0-9]+]]
//SPIRV-NOT: define {{.*}}printf

//CHECK: attributes #[[CONTAINS_TARGET]] = {{.*}}"contains-openmp-target"="true"
//SPIRV: attributes #[[CONTAINS_TARGET]] = {{.*}}"contains-openmp-target"="true"
//CHECK: attributes #[[TARGET_DECLARE]] = {{.*}}"openmp-target-declare"="true"

// end INTEL_COLLAB
