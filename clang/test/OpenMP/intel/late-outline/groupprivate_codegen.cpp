// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp  \
// RUN:  -fopenmp-late-outline -verify -triple x86_64-unknown-linux-gnu  \
// RUN:  -Wno-openmp-groupprivate %s | FileCheck %s -check-prefix HOST

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -Wno-openmp-groupprivate \
// RUN:  -emit-llvm-bc -disable-llvm-passes -fopenmp -fopenmp-targets=spir64 \
// RUN:  -fopenmp-late-outline -Werror -Wsource-uses-openmp -o \
// RUN:  %t_host.bc %s

// RUN: %clang_cc1 -triple spir64 \
// RUN:  -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -emit-llvm -disable-llvm-passes \
// RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline\
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
// RUN:  -Wsource-uses-openmp -Wno-openmp-groupprivate -o - %s \
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

int x_gp;
#pragma omp groupprivate (x_gp)

int main() {
#pragma omp target
  test1();
#pragma omp target
  test2();
#pragma omp target teams num_teams(4)
  x_gp++;
}
//HOST: @y = target_declare global i32 0, align 4
//HOST: @x_gp = global i32 0, align 4
//HOST: @_ZZL5test1vE1x = internal global i32 0, align 4
//TARG: @y = {{.*}}target_declare addrspace(3) global i32 0, align 4
//TARG: @x_gp = external addrspace(3) global i32, align 4
//TARG: @_ZZL5test1vE1x = internal addrspace(3) global i32 undef, align 4
//TARG-NOT: {i32 1, !"_Z1y", i32 0, i32 0, ptr addrspace(3) @y}

// end INTEL_COLLAB
