// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | \
// RUN:  FileCheck %s

// CHECK: atomicrmw fadd ptr
// CHECK: atomicrmw fsub ptr

int bar();

#pragma omp declare target
void foo()
{
  float counter_teams{};
  #pragma omp atomic update
  counter_teams = counter_teams + float { float{ 1. } / (  bar() ) };
  #pragma omp atomic update
  counter_teams = counter_teams - float { float{ 1. } / (  bar() ) };
}
#pragma omp end declare target
// end INTEL_COLLAB
