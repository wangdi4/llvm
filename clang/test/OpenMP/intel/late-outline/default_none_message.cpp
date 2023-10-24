// INTEL_COLLAB
//
// RUN: %clang_cc1 -fopenmp -fopenmp-targets=spir64 -triple spir64 \
// RUN:  -fopenmp-late-outline -verify %s
//

void foo(int j) {
  // expected-note@+2 {{explicit data sharing attribute requested here}}
  // expected-error@+3 {{variable 'j' must have explicitly specified data sharing attributes}}
  #pragma omp target teams distribute parallel for default(none)
  for (int i = 0; i < 10; i++)
    j = i;
  // expected-note@+2 {{explicit data sharing attribute requested here}}
  // expected-error@+3 {{variable 'j' must have explicitly specified data sharing attributes}}
  #pragma omp target teams loop default(none)
  for (int i = 0; i < 10; i++)
    j = i;
}

void bar(int *j) {
  // expected-note@+2 {{explicit data sharing attribute requested here}}
  // expected-error@+3 {{variable 'j' must have explicitly specified data sharing attributes}}
  #pragma omp target teams distribute parallel for default(none)
  for (int i = 0; i < 10; i++)
    *j = i;
  // expected-note@+2 {{explicit data sharing attribute requested here}}
  // expected-error@+3 {{variable 'j' must have explicitly specified data sharing attributes}}
  #pragma omp target teams loop default(none)
  for (int i = 0; i < 10; i++)
    *j = i;
}
// end INTEL_COLLAB
