
// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc

// RUN: %clang_cc1 -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

extern double log(double __x) throw();
#pragma omp declare target
double log_smallx = log(2.5);
#pragma omp end declare target

int main() { }
// CHECK: @log_smallx = target_declare addrspace(1) global double

// CHECK-LABEL: @{{.*}}_log_smallx_{{.*}}_ctor()
// CHECK: entry:
// CHECK-NEXT %call = call {{.*}} @_Z3logd
// CHECK-NEXT store double %call, double addrspace(4)* addrspacecast (double addrspace(1)* @log_smallx to double addrspace(4)*)
