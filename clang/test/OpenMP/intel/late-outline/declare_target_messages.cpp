// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=52 -x c++ %s
// RUN: %clang_cc1 -verify -fopenmp %s

#if _OPENMP == 202111
void f1(); // expected-note {{defined here}}
int c1, c2;
#pragma omp declare target local(c1) local(c2), local(f1) // expected-error {{function name is not allowed in 'local' clause}}

struct SSSt {
  void f2(); // expected-note {{defined here}}
  static int a;
#pragma omp declare target local(a, f2) // expected-error {{function name is not allowed in 'local' clause}}
  int b;
};
void f3(); // expected-note {{defined here}}
#pragma omp declare target
int inner_local;
#pragma omp declare target local(inner_local, f3) // expected-error {{function name is not allowed in 'local' clause}}
#pragma omp end declare target
#pragma omp begin declare target local(inner_local) // expected-error {{unexpected 'local' clause, only 'device_type', 'indirect' clauses expected}}
#pragma omp end declare target
#else
int c1, c2;
#pragma omp declare target local(c1) local(c2) // expected-error {{unexpected 'local' clause}}
#endif
// end INTEL_COLLAB
