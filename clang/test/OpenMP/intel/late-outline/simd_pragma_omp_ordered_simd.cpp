// INTEL_COLLAB

// Tests whether we can compile an simd region with multiple ordered regions
// without triggering any assertion.

// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -Wuninitialized %s | FileCheck %s

void foo() {
  #define N 1024
  int t, A[N], B[N], C[N] = {0};
// CHECK: "DIR.OMP.SIMD"()
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
    A[i] = B[i] + C[i];
// CHECK: "DIR.OMP.ORDERED"()
#pragma omp ordered simd
    t = A[i];
// CHECK: "DIR.OMP.ORDERED"()
#pragma omp ordered simd
    B[i] = t;
// CHECK: "DIR.OMP.END.SIMD"()
  }
}
// end INTEL_COLLAB
