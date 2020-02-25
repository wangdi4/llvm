// INTEL_COLLAB
// Verify aligned clause accepts array/reference to array.
//
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void foo()
{
#define N 10

  int i, a[N] = {0};
  int (&ra)[N] = a;

  // simd aligned(<array>)
  // CHECK: DIR.OMP.SIMD
  // CHECK-SAME: QUAL.OMP.ALIGNED
  // CHECK: DIR.OMP.END.SIMD
#pragma omp simd aligned(a)
  for (i=0; i<N; i++) {
    a[i] = 1;
  }

  // simd aligned(<reference to array>)
  // CHECK: DIR.OMP.SIMD
  // CHECK-SAME: QUAL.OMP.ALIGNED
  // CHECK: DIR.OMP.END.SIMD
#pragma omp simd aligned(ra)
  for (i=0; i<N; i++) {
    ra[i] = i;
  }
}
// end INTEL_COLLAB
