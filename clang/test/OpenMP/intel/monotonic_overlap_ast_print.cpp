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
typedef int(*fptr)();
int bar();
void foo()
{
  int k = 0;
  double a, b, c;

#pragma omp parallel
  for (int i = 0; i < 1024; i++) {
#pragma omp simd simdlen(1)
    for (int l = 0; l < 16; l++) {
      k++;
#pragma omp ordered simd ompx_monotonic(i : 10)
      {
        k += l;
      }
    }
  }
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_monotonic(i,k)
    k -= 0;
  }
  fptr fp = &bar;
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_overlap(fp)
    k -= 0;
  }
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_overlap(i+k)
    k -= 0;
  }
}
// CHECK: void foo
// CHECK: #pragma omp parallel
// CHECK: #pragma omp simd simdlen(1)
// CHECK: #pragma omp ordered simd ompx_monotonic(i: 10)
// CHECK: #pragma omp simd
// CHECK: #pragma omp ordered simd ompx_monotonic(i,k)
// CHECK: #pragma omp simd
// CHECK: #pragma omp ordered simd ompx_overlap(fp)
// CHECK: #pragma omp simd
// CHECK: #pragma omp ordered simd ompx_overlap(i + k)
//
struct S {
  int a;
  S();
  ~S();
  template <int t>
  void apply() {
#pragma omp simd
    for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_monotonic(a:t)
      a++;
    }
#pragma omp simd
    for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_overlap(a++)
      a++;
    }
#pragma omp simd
    for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_overlap(t)
      a++;
    }
  }
  // CHECK: template <int t> void apply()
  // CHECK: #pragma omp simd
  // CHECK: #pragma omp ordered simd ompx_monotonic(this->a: t)
  // CHECK: #pragma omp simd
  // CHECK: #pragma omp ordered simd ompx_overlap(this->a++)
  // CHECK: #pragma omp simd
  // CHECK: #pragma omp ordered simd ompx_overlap(t)
  // CHECK: template<> void apply<10>()
  // CHECK: #pragma omp simd
  // CHECK: #pragma omp ordered simd ompx_monotonic(this->a: 10)
  // CHECK: #pragma omp simd
  // CHECK: #pragma omp ordered simd ompx_overlap(this->a++)
  // CHECK: #pragma omp simd
  // CHECK: #pragma omp ordered simd ompx_overlap(10)
};
template<typename T>
void foo(T x) {
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_monotonic(x:10)
    x++;
  }
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_overlap(x)
    x++;
  }
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_overlap(10)
    x++;
  }
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_overlap(i+x)
    x++;
  }
}
// CHECK: template <typename T> void foo(T x)
// CHECK: #pragma omp simd
// CHECK: #pragma omp ordered simd ompx_monotonic(x: 10)
// CHECK: template<> void foo<int>(int x)
// CHECK: #pragma omp simd
// CHECK: #pragma omp ordered simd ompx_monotonic(x: 10)
// CHECK: #pragma omp simd
// CHECK: #pragma omp ordered simd ompx_overlap(x)
// CHECK: #pragma omp simd
// CHECK: #pragma omp ordered simd ompx_overlap(10)
// CHECK: #pragma omp simd
// CHECK: #pragma omp ordered simd ompx_overlap(i + x)
void use_template() {
   S obj;
   obj.apply<10>();
   foo(10);
}
#endif
