// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:   -triple x86_64-unknown-linux-gnu %s | FileCheck %s

float rc = 0.0;
void foo()
{
  int j(32);

  // ordered simd
  //
  // CHECK: DIR.OMP.SIMD
  // CHECK: DIR.OMP.ORDERED
  // CHECK-SAME: "QUAL.OMP.ORDERED.SIMD"
  // CHECK: DIR.OMP.END.ORDERED
  // CHECK: DIR.OMP.END.SIMD
  #pragma omp simd
  for (int i = 0; i < j; i++) {
    int inter = i * i;
    #pragma omp ordered simd
    {
      rc = inter + rc;
    }
  }

  // ordered threads
  //
  // CHECK: DIR.OMP.LOOP
  // CHECK-SAME: QUAL.OMP.ORDERED
  // CHECK: DIR.OMP.ORDERED
  // CHECK-SAME: "QUAL.OMP.ORDERED.THREADS"
  // CHECK: DIR.OMP.END.ORDERED
  // CHECK: DIR.OMP.END.LOOP
  #pragma omp for ordered
  for (int i = 0; i < j; i++) {
    int inter = i * i;
    #pragma omp ordered threads
    {
      rc = inter + rc;
    }
  }

  // ordered threads simd
  //
  // CHECK: DIR.OMP.SIMD
  // CHECK: DIR.OMP.ORDERED
  // CHECK-SAME: "QUAL.OMP.ORDERED.THREADS"
  // CHECK-SAME: "QUAL.OMP.ORDERED.SIMD"
  // CHECK: DIR.OMP.END.ORDERED
  // CHECK: DIR.OMP.END.SIMD
  #pragma omp simd
  for (int i = 0; i < j; i++) {
    int inter = i * i;
    #pragma omp ordered threads simd
    {
      rc = inter + rc;
    }
  }

  // ordered simd threads
  //
  // CHECK: DIR.OMP.SIMD
  // CHECK: DIR.OMP.ORDERED
  // CHECK-SAME: "QUAL.OMP.ORDERED.SIMD"
  // CHECK-SAME: "QUAL.OMP.ORDERED.THREADS"
  // CHECK: DIR.OMP.END.ORDERED
  // CHECK: DIR.OMP.END.SIMD
  #pragma omp simd
  for (int i = 0; i < j; i++) {
    int inter = i * i;
    #pragma omp ordered simd threads
    {
      rc = inter + rc;
    }
  }
}
// end INTEL_COLLAB
