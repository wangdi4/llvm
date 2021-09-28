// INTEL_COLLAB
// RUN: %clang_cc1 -fopenmp %s -verify

void test() {
  int foo;

  // expected-error@+1 {{unexpected OpenMP clause 'in_reduction' in directive '#pragma omp target'}}
  #pragma omp target in_reduction(+:foo)
  { foo++; }
}
