// RUN: %clang_cc1 -verify -fopenmp-simd -std=c++11 -fopenmp-late-outline %s
// RUN: %clang_cc1 -verify -fopenmp -std=c++11 -fopenmp-late-outline %s
// RUN: %clang_cc1 -verify -fopenmp-simd -std=c++11 -DNOEXT %s

#ifdef NOEXT
//expected-warning@+1 {{extra tokens at the end of '#pragma omp declare simd' are ignored}}
#pragma omp declare simd ompx_processor(skylake)
int foo(int *ip, int c);
#else // NOEXT
//expected-error@+2 {{invalid cpu name 'sklake' in 'ompx_processor' clause}}
//expected-error@+1 {{invalid cpu name 'foobar' in 'ompx_processor' clause}}
#pragma omp declare simd ompx_processor(sklake) ompx_processor(skylake) ompx_processor(foobar)
int foo(int *ip, int c);

//expected-error@+3 {{invalid cpu name 'sklake' in 'ompx_processor' clause}}
//expected-error@+2 {{expected ')'}}
//expected-note@+1 {{to match this '('}}
#pragma omp declare simd ompx_processor(sklake
int foo2(int *ip, int c);

//expected-error@+1 {{expected identifier in 'ompx_processor' clause}}
#pragma omp declare simd ompx_processor()
int foo3(int *ip, int c);

//expected-error@+3 {{expected ')'}}
//expected-note@+2 {{to match this '('}}
//expected-error@+1 {{expected identifier in 'ompx_processor' clause}}
#pragma omp declare simd ompx_processor(
int foo4(int *ip, int c);

//expected-error@+1 {{expected identifier in 'ompx_processor' clause}}
#pragma omp declare simd ompx_processor(3.4)
int foo5(int *ip, int c);

//expected-error@+2 {{expected '(' after 'ompx_processor'}}
//expected-error@+1 {{expected identifier in 'ompx_processor' clause}}
#pragma omp declare simd ompx_processor
int foo5(int *ip, int c);

//expected-error@+1 {{invalid cpu name 'foo' in 'ompx_processor' clause}}
#pragma omp declare simd ompx_processor(foo)
template <typename T>
int foo6(T *ip, int c);
#endif // NOEXT
