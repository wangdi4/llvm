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
// CHECK: void foo
void foo()
{
  int k = 0;
  double a, b, c;

// CHECK: #pragma omp parallel
#pragma omp parallel
  for (int i = 0; i < 1024; i++) {
// CHECK: #pragma omp simd simdlen(1) ompx_assert
#pragma omp simd simdlen(1) ompx_assert
    for (int l = 0; l < 16; l++) {
      k++;
    }
  }
// CHECK: #pragma omp for simd ompx_assert simdlen(1) safelen(1)
#pragma omp for simd ompx_assert simdlen(1) safelen(1)
  for (int i = 0; i < 1024; i++) {
      k++;
  }
// CHECK: #pragma omp parallel for simd ompx_assert simdlen(1) safelen(1)
#pragma omp parallel for simd ompx_assert simdlen(1) safelen(1)
  for (int i = 0; i < 1024; i++) {
      k++;
  }
// CHECK: #pragma omp taskloop simd ompx_assert simdlen(1) safelen(1)
#pragma omp taskloop simd ompx_assert simdlen(1) safelen(1)
  for (int i = 0; i < 1024; i++) {
      k++;
  }
// CHECK: #pragma omp master taskloop simd ompx_assert simdlen(1) safelen(1)
#pragma omp master taskloop simd ompx_assert simdlen(1) safelen(1)
  for (int i = 0; i < 1024; i++) {
      k++;
  }
// CHECK: #pragma omp masked taskloop simd ompx_assert simdlen(1) safelen(1)
#pragma omp masked taskloop simd ompx_assert simdlen(1) safelen(1)
  for (int i = 0; i < 1024; i++) {
      k++;
  }
// CHECK: #pragma omp simd ompx_assert simdlen(1) safelen(1)
#pragma omp simd ompx_assert simdlen(1) safelen(1)
  for (int i = 0; i < 1024; i++) {
      k++;
  }
// CHECK: #pragma omp target teams
#pragma omp target teams
// CHECK: #pragma omp distribute simd ompx_assert
#pragma omp distribute simd ompx_assert
  for(int i = 0 ; i < 1024; i++) {
    k++;
  }
// CHECK: #pragma omp target teams
#pragma omp target teams
// CHECK: #pragma omp distribute parallel for simd ompx_assert
#pragma omp distribute parallel for simd ompx_assert
  for(int i = 0 ; i < 1024; i++) {
    k++;
  }
}

struct S {
  int a;
  S();
  ~S();
  // CHECK: template <int t> void apply()
  template <int t>
  void apply() {
  // CHECK: #pragma omp simd ompx_assert safelen(1) simdlen(1)
#pragma omp simd ompx_assert safelen(1) simdlen(1)
    for (int i = 0; i < 1024; i++) {
      a++;
    }
  }
};
template<typename T>
void foo(T x) {
#pragma omp simd ompx_assert safelen(1) simdlen(1)
    for (int i = 0; i < 1024; i++) {
      x++;
  }
}
void use_template() {
  S obj;
  // CHECK: template<> void apply<10>()
  // CHECK: #pragma omp simd ompx_assert safelen(1) simdlen(1)
  obj.apply<10>();
  // CHECK: template <typename T> void foo(T x)
  // CHECK: #pragma omp simd ompx_assert safelen(1) simdlen(1)
  // CHECK: template<> void foo<int>(int x)
  // CHECK: #pragma omp simd ompx_assert safelen(1) simdlen(1)
  foo(10);
}
#endif
