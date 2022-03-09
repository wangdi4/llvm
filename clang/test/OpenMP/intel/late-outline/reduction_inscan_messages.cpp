// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-late-outline\
// RUN: -Wsource-uses-openmp %s
//
// Verify both reduction modifier 'inscan' *and* omp scan directive generate
// warnings and are ignored.

#define N 16
int scan_a;
int main(){
  int a[N], simd_scan[N];
  scan_a = 0;
  // expected-warning@+1 {{unsupported modifier 'inscan' - ignored}}
  #pragma omp for simd reduction(inscan, +:scan_a)
  for(int i = 0; i < N; i++){
    scan_a += a[i];
    // expected-warning@+1 {{OpenMP directive 'scan' unimplemented - ignored}}
    #pragma omp scan inclusive(scan_a)
    simd_scan[i] = scan_a;
  }
  return 0;
}
// end INTEL_COLLAB
