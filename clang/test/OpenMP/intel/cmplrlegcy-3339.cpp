//RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fintel-compatibility \
//RUN:   -fopenmp-late-outline -fno-intel-openmp-offload \
//RUN:   -fopenmp-targets=x86_64 -triple x86_64 %s \
//RUN:   | FileCheck %s

// CHECK: @.omp_offloading.entry
// CHECK: @.omp_offloading.entry

#pragma omp declare target
int x1;
int x2;
#pragma omp end declare target
