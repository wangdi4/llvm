// INTEL_COLLAB

// RUN: %clang_cc1 -O0 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN: -fintel-compatibility -fopenmp-late-outline \
// RUN: -fopenmp-targets=spir64 -no-opaque-pointers -emit-llvm %s -o - \
// RUN: | FileCheck %s --check-prefix HOST
//
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:   -no-opaque-pointers -fintel-compatibility -fopenmp-late-outline \
// RUN:   -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc

// RUN: %clang_cc1 -verify -triple spir64 -fopenmp \
// RUN:   -fintel-compatibility -fopenmp-late-outline \
// RUN:   -fopenmp-targets=spir64 -fopenmp-is-device -no-opaque-pointers \
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

// TARG-DAG: @_ZL3Str = internal target_declare addrspace(1) global i8 addrspace(4)* addrspacecast (i8 addrspace(1)* getelementptr inbounds ([5 x i8], [5 x i8] addrspace(1)* @.str, i32 0, i32 0) to i8 addrspace(4)*)
// TARG-DAG: @_ZL1S = internal target_declare addrspace(1) global i8 addrspace(4)* addrspacecast (i8 addrspace(1)* getelementptr inbounds ([5 x i8], [5 x i8] addrspace(1)* @.str, i32 0, i32 0) to i8 addrspace(4)*)
// TARG-LABEL: @_Z6getStrv
// TARG-LABEL: @_Z3zoov
// TARG-LABLE: @_Z1xv

// end  INTEL_COLLAB
