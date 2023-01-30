// INTEL_COLLAB
//
// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline -fopenmp-version=51 %s -verify
//
void foo()
{
  int var;

  #pragma omp task
  {
    // expected-error@+1 {{'mutexinoutset' modifier not allowed in 'depend' clause on 'taskwait' directive}}
    #pragma omp taskwait nowait depend(in:var) depend(mutexinoutset:var)
  }
}
// end INTEL_COLLAB
