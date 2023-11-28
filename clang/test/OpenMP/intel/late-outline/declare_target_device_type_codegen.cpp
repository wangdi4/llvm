// INTEL_COLLAB

// RUN: %clang_cc1 -O0 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN: -fintel-compatibility -fopenmp-late-outline \
// RUN: -fopenmp-targets=spir64 -emit-llvm %s -o - \
// RUN: | FileCheck %s --check-prefix HOST
//
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:   -fintel-compatibility -fopenmp-late-outline \
// RUN:   -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc

// RUN: %clang_cc1 -verify -triple spir64 -fopenmp \
// RUN:   -fintel-compatibility -fopenmp-late-outline \
// RUN:   -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:   -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:   | FileCheck %s --check-prefix TARG

// expected-no-diagnostics


#pragma omp declare target device_type(nohost)
static const char *Str = "test";
const char *getStr() {
  return Str;
}
#pragma omp end declare target

#pragma omp declare target device_type(nohost)
static const char *S = "test";
#pragma omp end declare target
#pragma omp declare target to(S)

#pragma omp begin declare target device_type(nohost)
void zoo() {}
void x();
#pragma omp end declare target
#pragma omp declare target to(x)
void x() {
}
void foo()
{
}

// HOST-NOT: @_ZL1S = external target_declare global
// HOST-NOT: @_ZL3Str = external target_declare global
// HOST-LABEL: @_Z3foov

// TARG-DAG: @_ZL3Str = internal target_declare addrspace(1) global ptr addrspace(4) addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), align 8
// TARG-DAG: @_ZL1S = internal target_declare addrspace(1) global ptr addrspace(4) addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4))
// TARG-LABEL: @_Z6getStrv
// TARG-LABEL: @_Z3zoov
// TARG-LABLE: @_Z1xv

// end  INTEL_COLLAB
