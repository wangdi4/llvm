// RUN: %clang_cc1 -std=c++11 -DTEST -verify \
// RUN:  -fopenmp -ferror-limit 200 %s -Wuninitialized

// RUN: %clang_cc1 -std=c++11 -DTEST1 -verify -fopenmp-late-outline \
// RUN:  -fopenmp-simd -ferror-limit 200 %s -Wuninitialized
#ifdef TEST
void zoo()
{
  int k = 0;
#pragma omp simd simdlen(1)
  for (int l = 0; l < 16; l++) {
    k++;
  // expected-warning@+1 {{extra tokens at the end of '#pragma omp ordered' are ignored}}
  #pragma omp ordered simd ompx_monotonic(k:10)
    {
      k -= l;
    }
  }
#pragma omp simd simdlen(1)
  for (int l = 0; l < 16; l++) {
    k++;
  // expected-warning@+1 {{extra tokens at the end of '#pragma omp ordered' are ignored}}
  #pragma omp ordered simd ompx_overlap(k)
    {
      k -= l;
    }
  }
}
#endif

#ifdef TEST1
void foo()
{
     int k;
#pragma omp simd simdlen(1)
  for (int l = 0; l < 16; l++) {
    double a;
    k++;
  // expected-error@+3 {{argument to 'ompx_monotonic' clause must be a strictly positive integer value}}
  // expected-error@+2 {{OpenMP constructs may not be nested inside a simd region}}
  // expected-error@+1 {{'#pragma omp ordered' with 'ompx_monotonic' clause can only be used with 'ordered simd'}}
  #pragma omp ordered ompx_monotonic(k:-1)
    {
      k -= 1;
    }
  }
  // expected-error@+1 {{'#pragma omp ordered' with 'ompx_overlap' clause can only be used with 'ordered simd'}}
  #pragma omp ordered ompx_overlap(k)
    {
      k -= 1;
    }
}

void zoo()
{
  int k;
#pragma omp simd simdlen(1)
  for (int l = 0; l < 16; l++) {
    double a; // expected-note {{'a' defined here}}
    k++;
  //expected-error@+2 {{integral constant expression must have integral or unscoped enumeration type, not 'double'}}
  //expected-error@+1 {{argument of ompx_monotonic clause should be of integral or pointer type, not 'double'}}
  #pragma omp ordered simd ompx_monotonic(a,k:a)
    {
      k -= l;
    }
  }
  // expected-error@+1 {{expected expression}}
  #pragma omp ordered simd ompx_monotonic()
    {
      k -= 0;
    }
  double a;
  //expected-error@+1 {{argument of ompx_overlap clause should be of integral or pointer type, not 'double'}}
  #pragma omp ordered simd ompx_overlap(a+k)
    {
      k -= 1;
    }
}
#endif
