// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -DCOMBINED %s | FileCheck %s
//
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -DCOMBINED_TARGET %s | FileCheck %s
//
// Verify variants of combined directives involving omp target, teams,
// distribute, and simd compile without error.

bool almost_equal(double x, double gold, float tol) {
  return gold * (1-tol) <= x && x <= gold * (1 + tol);
}

void test_target__teams_distribute_simd() {
  const int L = 262144;
  double counter{};

//CHECK: "DIR.OMP.TARGET"
//CHECK-SAME: "QUAL.OMP.OFFLOAD.ENTRY.IDX"
//CHECK-SAME: QUAL.OMP.MAP.TOFROM"
//CHECK: "DIR.OMP.TEAMS"
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"
//CHECK: "DIR.OMP.DISTRIBUTE"
//CHECK: "DIR.OMP.SIMD"
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"
//CHECK: "DIR.OMP.END.SIMD"
//CHECK: "DIR.OMP.END.DISTRIBUTE"
//CHECK: "DIR.OMP.END.TEAMS"
//CHECK: "DIR.OMP.END.TARGET"
#ifndef COMBINED_TARGET
#pragma omp target map(tofrom:counter)
#endif
  {
#if defined(COMBINED)
#pragma omp teams distribute simd reduction(+: counter)
#elif defined(COMBINED_TARGET)
#pragma omp target teams distribute simd reduction(+: counter)
#else
#pragma omp teams reduction(+: counter)
#pragma omp distribute simd  reduction(+: counter)
#endif
    for (int i = 0 ; i < L ; i++ ) {
      counter += double { 1.0f };
    }
  }
  if (!almost_equal(counter,double { L }, 0.1))
    return;
}
// end INTEL_COLLAB
