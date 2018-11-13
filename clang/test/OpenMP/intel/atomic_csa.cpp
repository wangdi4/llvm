// INTEL_FEATURE_CSA
// This code was moved from atomic.cpp.
// TODO (vzakhari 11/14/2018): this is actually a hack that we do not use
//       #if guard above.  We have to live with this, until driver
//       starts defining the proper INTEL_FEATURE macros.
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility \
// RUN:   -fintel-openmp-region -triple csa \
// RUN:   | FileCheck %s
// REQUIRES: csa-registered-target

int i;
__int128 j;
//CHECK: define{{.*}}foo
void foo()
{
  #pragma omp target
  {
    //CHECK: atomicrmw add{{.*}}monotonic
    #pragma omp atomic
    i++;

    //CHECK-NOT: "DIR.OMP.ATOMIC"()
    #pragma omp atomic
    j++;
  }
}
// end INTEL_FEATURE_CSA
