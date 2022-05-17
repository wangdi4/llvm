// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline         \
// RUN:  -fopenmp-targets=spir64 -emit-llvm %s -o -          \
// RUN:   | FileCheck %s --check-prefix HOST

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc

// RUN: %clang_cc1 -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s --check-prefix TARG

// expected-no-diagnostics

#pragma omp declare target
//HOST: @naa_hidden = internal target_declare global i32 0
//TARG: @naa_hidden = internal target_declare addrspace(1) global i32 0

static int naa_hidden;
#pragma omp end declare target

//HOST: define {{.*}}bar
//TARG: define {{.*}}bar
int bar() { if(naa_hidden == 42) return 0; return 1; }

//HOST: define {{.*}}main
int main()
{
  int i;
  naa_hidden = 42;
  //HOST: DIR.OMP.TARGET.UPDATE{{.+}}QUAL.OMP.MAP.TO{{.*}}naa_hidden
  #pragma omp target update to(naa_hidden)

  #pragma omp target
  i = bar();

  return i;
}
//HOST: !omp_offload.info = !{!0, !1}
//HOST: !1 = !{i32 1, {{.*}}naa_hidden
