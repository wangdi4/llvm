// INTEL_COLLAB
// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline \
// RUN:    -fopenmp-version=51 -emit-llvm -o - %s | FileCheck %s

void foo1() {

  //CHECK: "QUAL.OMP.ORDER.CONCURRENT"()
  #pragma omp parallel loop order(concurrent)
  for (int i=0; i<1000; ++i) {}

  //CHECK: "QUAL.OMP.ORDER.CONCURRENT:REPRODUCIBLE"()
  #pragma omp parallel loop order(reproducible:concurrent)
  for (int i=0; i<1000; ++i) {}


  //CHECK: "QUAL.OMP.ORDER.CONCURRENT:UNCONSTRAINED"()
  #pragma omp parallel loop order(unconstrained:concurrent)
  for (int i=0; i<1000; ++i) {}

}
// end INTEL_COLLAB
