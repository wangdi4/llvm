// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=52 -I %S/Inputs -ast-print %s | FileCheck %s
// RUN: %clang_cc1 -fopenmp -fopenmp-version=52 -x c++ -std=c++11 -I %S/Inputs -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp -fopenmp-version=52 -std=c++11 -include-pch %t -fsyntax-only -I %S/Inputs -verify %s -ast-print | FileCheck %s

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

int c1, c2, c3;
#pragma omp declare target local(c1) local(c2), local(c3)
// CHECK: #pragma omp declare target local{{$}}
// CHECK: int c1;
// CHECK: #pragma omp end declare target{{$}}
// CHECK: #pragma omp declare target local{{$}}
// CHECK: int c2;
// CHECK: #pragma omp end declare target{{$}}
// CHECK: #pragma omp declare target local{{$}}
// CHECK: int c3;
// CHECK: #pragma omp end declare target{{$}}

struct SSSt {
  static int a;
#pragma omp declare target local(a)
  int b;
};
// CHECK: #pragma omp declare target local{{$}}
// CHECK: static int a;
// CHECK: #pragma omp end declare target

#pragma omp declare target
int inner_local;
#pragma omp declare target local(inner_local)
#pragma omp end declare target

// CHECK: #pragma omp declare target
// CHECK-NEXT: #pragma omp declare target local
// CHECK-NEXT: int inner_local;
// CHECK-NEXT: #pragma omp end declare target

// Do not expect anything here since the region is empty.
#pragma omp declare target
#pragma omp end declare target

#endif
// end INTEL_COLLAB
