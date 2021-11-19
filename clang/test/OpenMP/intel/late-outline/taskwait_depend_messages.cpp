// INTEL_COLLAB
//
// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline \
// RUN: -fopenmp-version=51 -DLATEOUT %s -verify
//
// RUN: %clang_cc1 -fopenmp -fopenmp-version=51 %s -verify
//
void foo()
{
  int var;

  #pragma omp task
  {
#ifdef LATEOUT
    // expected-error@+1 {{'mutexinoutset' modifier not allowed in 'depend' clause on 'taskwait' directive}}
    #pragma omp taskwait nowait depend(in:var) depend(mutexinoutset:var)
    // expected-error@+1 {{'nowait' clause requires a 'depend' clause when used with 'taskwait' directive}}
    #pragma omp taskwait nowait
    // expected-error@+1 {{directive '#pragma omp taskwait' cannot contain more than one 'nowait' clause}}
    #pragma omp taskwait nowait depend(out:var) nowait
#else
    // expected-error@+1 {{unexpected OpenMP clause 'nowait' in directive '#pragma omp taskwait'}}
    #pragma omp taskwait nowait depend(in:var)
#endif
  }
}
// end INTEL_COLLAB
