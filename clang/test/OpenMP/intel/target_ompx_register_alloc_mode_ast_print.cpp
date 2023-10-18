// RUN: %clang_cc1 -verify -fopenmp  -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -ast-print %s -o - | FileCheck %s --check-prefix=CHECK
// RUN: %clang_cc1 -fopenmp  -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp  -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -std=c++11 -include-pch %t -verify %s -ast-print -o - | FileCheck %s --check-prefix=CHECK
// RUN: %clang_cc1 -verify -fopenmp  -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -ast-print %s -o - | FileCheck %s --check-prefix=CHECK
// RUN: %clang_cc1 -fopenmp -x c++ -std=c++11 -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp  -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -std=c++11 -include-pch %t -verify %s -ast-print -o - | FileCheck %s --check-prefix=CHECK

// RUN: %clang_cc1 -verify -fopenmp-simd  -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -ast-print %s -o - | FileCheck %s
// RUN: %clang_cc1 -fopenmp-simd  -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp-simd  -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -std=c++11 -include-pch %t -verify %s -ast-print -o - | FileCheck %s
// RUN: %clang_cc1 -verify -fopenmp-simd  -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -ast-print %s -o - | FileCheck %s
// RUN: %clang_cc1 -fopenmp-simd  -x c++ -std=c++11 -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -std=c++11 -include-pch %t -verify %s -ast-print -o - | FileCheck %s

// expected-no-diagnostics
#ifndef HEADER
#define HEADER

template<typename tx>
tx ftemplate(int n) {
  tx a = 0;

  #pragma omp target teams ompx_register_alloc_mode(small)
  {
  }

  short b = 1;
  #pragma omp target teams num_teams(b) ompx_register_alloc_mode(default)
  {
    a += b;
  }

  return a;
}

static
int fstatic(int n) {

  #pragma omp target teams distribute parallel for simd num_teams(n) ompx_register_alloc_mode(auto)
  for (int i = 0; i < n ; ++i) {
  }

  #pragma omp target teams ompx_register_alloc_mode(large) nowait
  {
  }

  return n+1;
}

struct S1 {
  double a;

  int r1(int n){
    int b = 1;

    #pragma omp target teams ompx_register_alloc_mode(auto)
    {
      this->a = (double)b + 1.5;
    }

    #pragma omp target ompx_register_alloc_mode(default)
    {
      this->a = 2.5;
    }

    return (int)a;
  }
};

int bar(int n){
  int a = 0;

  S1 S;
  a += S.r1(n);

  a += fstatic(n);

  a += ftemplate<int>(n);

  return a;
}

#endif

// CHECK: template <typename tx> tx ftemplate(int n) {
// CHECK:    tx a = 0;
// CHECK:    #pragma omp target teams ompx_register_alloc_mode(small)
// CHECK:        {
// CHECK:        }
// CHECK:    short b = 1;
// CHECK:    #pragma omp target teams num_teams(b) ompx_register_alloc_mode(default)
// CHECK:        {
// CHECK:            a += b;
// CHECK:        }
// CHECK:    return a;
// CHECK:}
// CHECK:template<> int ftemplate<int>(int n) {
// CHECK:    int a = 0;
// CHECK:    #pragma omp target teams ompx_register_alloc_mode(small)
// CHECK:        {
// CHECK:        }
// CHECK:   short b = 1;
// CHECK:   #pragma omp target teams num_teams(b) ompx_register_alloc_mode(default)
// CHECK:        {
// CHECK:           a += b;
// CHECK:       }
// CHECK:    return a;
// CHECK:}
// CHECK:static int fstatic(int n) {
// CHECK:    #pragma omp target teams distribute parallel for simd num_teams(n) ompx_register_alloc_mode(auto)
// CHECK:        for (int i = 0; i < n; ++i) {
// CHECK:        }
// CHECK:    #pragma omp target teams ompx_register_alloc_mode(large) nowait
// CHECK:        {
// CHECK:        }
// CHECK:    return n + 1;
// CHECK:}
// CHECK:struct S1 {
// CHECK:    double a;
// CHECK:    int r1(int n) {
// CHECK:        int b = 1;
// CHECK:        #pragma omp target teams ompx_register_alloc_mode(auto)
// CHECK:            {
// CHECK:                this->a = (double)b + 1.5;
// CHECK:            }
// CHECK:        #pragma omp target ompx_register_alloc_mode(default)
// CHECK:            {
// CHECK:                this->a = 2.5;
// CHECK:            }
// CHECK:        return (int)this->a;
// CHECK:    }
// CHECK:};
