// INTEL_COLLAB
//
// RUN: %clang_cc1 -fopenmp -fopenmp-targets=spir64 -triple spir64 \
// RUN:  -fopenmp-late-outline -verify %s
//
// RAN: %clang_cc1 -fopenmp -fopenmp-targets=x86_64 \
// RAN:  -fopenmp-late-outline -verify -DNOERROR %s
//
// Verify 'ordered' clause in target region with spir64 generates an error.
// Otherwise, it should compile cleanly.

#ifdef NOERROR
// expected-no-diagnostics
#endif

double S[(16*32)];
int main() {

  #pragma omp target map(tofrom:S[0:16*32])
  #pragma omp teams distribute 
  for (int idx = 0; idx < 16*32; idx++) {
    S[idx] = 0;
#ifndef NOERROR
    // expected-error@+2 {{'ordered' is unsupported in target region}}
#endif
    #pragma omp parallel for ordered
    for (int i = 0; i < (1024*3); i++) {
#ifndef NOERROR
      // expected-error@+2 {{region cannot be closely nested inside 'parallel for' region; perhaps you forget to enclose 'omp ordered' directive into a for or a parallel for region with 'ordered' clause?}}
#endif
      #pragma omp ordered
      S[idx] += 1;
    }
  }
}
// end INTEL_COLLAB
