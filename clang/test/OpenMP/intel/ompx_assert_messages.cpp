// RUN: %clang_cc1 -verify -fsyntax-only -std=c++14 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline %s -DMESSAGES
// RUN: %clang_cc1 -std=c++14 -DTEST -verify \
// RUN:  -fopenmp -ferror-limit 200 %s -Wuninitialized

void bar(int) noexcept;

#define LOOP                   \
  for (i = 0; i < 1024; i++) {  \
    bar(v_ptr[i]);        \
  }

const int M = 64;
void ompx_assert_test(int v_ptr[M])
{
  int i, j;
#ifdef MESSAGES
  //expected-error@+1 {{unexpected OpenMP clause 'ompx_assert' in directive}}
  #pragma omp parallel for ompx_assert
  LOOP

  //expected-error@+1 {{cannot contain more than one 'ompx_assert' clause}}
  #pragma omp simd ompx_assert ompx_assert safelen(1) simdlen(1)
  LOOP
#endif // MESSAGES

#ifdef TEST
  // expected-warning@+1 {{extra tokens at the end of '#pragma omp simd' are ignored}}
#pragma omp simd ompx_assert simdlen(1)
  LOOP
#endif // TEST
}
