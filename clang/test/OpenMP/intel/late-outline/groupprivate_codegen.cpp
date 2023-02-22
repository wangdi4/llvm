// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp  \
// RUN:  -fopenmp-late-outline -verify -triple x86_64-unknown-linux-gnu  \
// RUN:  %s | FileCheck %s -check-prefix HOST

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
// RUN:  -emit-llvm-bc -disable-llvm-passes -fopenmp -fopenmp-targets=spir64 \
// RUN:  -fopenmp-late-outline -Werror -Wsource-uses-openmp -o \
// RUN:  %t_host.bc %s

// RUN: %clang_cc1 -opaque-pointers -triple spir64 \
// RUN:  -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -emit-llvm -disable-llvm-passes \
// RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline\
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
// RUN:  -Wsource-uses-openmp -o - %s \
// RUN:  | FileCheck %s -check-prefix TARG

// expected-no-diagnostics

#pragma omp begin declare target
static void test1() {
  static int x;
  #pragma omp groupprivate(x)
  x = 0;
}
#pragma omp end declare target

#pragma omp begin declare target
int y;
#pragma omp groupprivate (y)
int test2() {
  y = y + 1;
  return y;
}
#pragma omp end declare target

int main() {
#pragma omp target
  test1();
#pragma omp target
  test2();
}
//HOST: @y = target_declare global i32 0, align 4
//HOST: @_ZZL5test1vE1x = internal global i32 0, align 4
//TARG: @y = target_declare addrspace(3) global i32 0, align 4
//TARG: @_ZZL5test1vE1x = internal addrspace(3) global i32 undef, align 4

// end INTEL_COLLAB
