// RUN: %clang_cc1 -verify -fopenmp-simd -std=c++11 -fopenmp-late-outline \
// RUN:  -ast-print %s | FileCheck %s

// RUN: %clang_cc1 -fopenmp-simd -x c++ -std=c++11 -fopenmp-late-outline \
// RUN:  -emit-pch -o %t %s

// RUN: %clang_cc1 -fopenmp-simd -std=c++11 -fopenmp-late-outline \
// RUN:  -include-pch %t -fsyntax-only -verify %s -ast-print | FileCheck %s

// RUN: %clang_cc1 -verify -fopenmp-simd -fopenmp-late-outline \
// RUN:  -std=c++11 -ast-print %s | FileCheck %s

// RUN: %clang_cc1 -fopenmp-simd -x c++ -std=c++11 -fopenmp-late-outline \
// RUN:  -fopenmp-version=51 -emit-pch -o %t %s

// RUN: %clang_cc1 -fopenmp-simd -std=c++11 -fopenmp-version=51 \
// RUN:  -fopenmp-late-outline \
// RUN:  -include-pch %t -fsyntax-only -verify %s -ast-print | FileCheck %s

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

//CHECK: #pragma omp declare simd ompx_processor(skylake) ompx_processor(broadwell)
//CHECK-NEXT: int foo(int *ip, int c)
#pragma omp declare simd ompx_processor(skylake) ompx_processor(broadwell)
int foo(int *ip, int c) {
  return *ip+c;
}

//CHECK: #pragma omp declare simd ompx_processor(skylake) ompx_processor(broadwell)
//CHECK-NEXT: template <typename T> int bar(T *ip, int c)
#pragma omp declare simd ompx_processor(skylake) ompx_processor(broadwell)
template <typename T>
int bar(T *ip, int c) {
  return  int(*ip)+c;
}

//CHECK: #pragma omp declare simd ompx_processor(skylake) ompx_processor(broadwell)
//CHECK-NEXT: template<> int bar<unsigned long>(unsigned long *ip, int c)

int baz()
{
  unsigned long j = 3;
  return bar(&j, 5);
}
#endif
