// INTEL_COLLAB

// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline %s -Wuninitialized -verify

void foo(int thread)
{
  #pragma omp masked
  {
  }

  // expected-warning@+1 {{'filter' clause is not supported yet and will be ignored}}
  #pragma omp masked filter(thread)
  {
  }
}

// end INTEL_COLLAB
