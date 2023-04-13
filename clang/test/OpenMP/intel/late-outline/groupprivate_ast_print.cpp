// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -triple x86_64-apple-darwin10.6.0 -Wno-openmp-groupprivate -ast-print %s | FileCheck %s
// RUN: %clang_cc1 -fopenmp -triple x86_64-apple-darwin10.6.0 -Wno-openmp-groupprivate -x c++ -std=c++11 -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp -triple x86_64-apple-darwin10.6.0 -Wno-openmp-groupprivate -std=c++11 -include-pch %t -fsyntax-only -verify %s -ast-print
// RUN: %clang_cc1 -verify -fopenmp -triple x86_64-unknown-linux-gnu -Wno-openmp-groupprivate -ast-print %s | FileCheck %s
// RUN: %clang_cc1 -fopenmp -fnoopenmp-use-tls -triple x86_64-unknown-linux-gnu -Wno-openmp-groupprivate -x c++ -std=c++11 -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp -fnoopenmp-use-tls -triple x86_64-unknown-linux-gnu -Wno-openmp-groupprivate -std=c++11 -include-pch %t -fsyntax-only -verify %s -ast-print

// RUN: %clang_cc1 -verify -fopenmp-simd -triple x86_64-apple-darwin10.6.0 -Wno-openmp-groupprivate -ast-print %s | FileCheck %s
// RUN: %clang_cc1 -fopenmp-simd -triple x86_64-apple-darwin10.6.0 -x c++ -Wno-openmp-groupprivate -std=c++11 -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -triple x86_64-apple-darwin10.6.0 -Wno-openmp-groupprivate -std=c++11 -include-pch %t -fsyntax-only -verify %s -ast-print
// RUN: %clang_cc1 -verify -fopenmp-simd -triple x86_64-unknown-linux-gnu -Wno-openmp-groupprivate -ast-print %s | FileCheck %s
// RUN: %clang_cc1 -fopenmp-simd -fnoopenmp-use-tls -triple x86_64-unknown-linux-gnu -Wno-openmp-groupprivate -x c++ -std=c++11 -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -fnoopenmp-use-tls -triple x86_64-unknown-linux-gnu -Wno-openmp-groupprivate -std=c++11 -include-pch %t -fsyntax-only -verify %s -ast-print
// expected-no-diagnostics

#ifndef HEADER
#define HEADER

struct St{
 int a;
};

struct St1{
 int a;
 static int b;
// CHECK: static int b;
#pragma omp groupprivate(b) device_type(nohost)
// CHECK-NEXT: #pragma omp groupprivate(St1::b) device_type(nohost)
};

int a, b;
// CHECK: int a;
// CHECK: int b;
#pragma omp groupprivate(a) device_type(nohost)
// CHECK-NEXT: #pragma omp groupprivate(a) device_type(nohost)
#pragma omp groupprivate(b) device_type(nohost)
// CHECK-NEXT: #pragma omp groupprivate(b) device_type(nohost)
#pragma omp begin declare target
template <class T>
struct ST {
  static T m;
  #pragma omp groupprivate(m) device_type(nohost)
};
//CHECK:   template <class T> struct ST {
//CHECK-NEXT: #pragma omp declare target
//CHECK-NEXT: static T m;
//CHECK-NEXT: #pragma omp end declare target
//CHECK-NEXT: #pragma omp groupprivate(ST::m) device_type(nohost)
//CHECK-NEXT: };
//CHECK:   template<> struct ST<int> {
//CHECK-NEXT: #pragma omp declare target
//CHECK-NEXT: static int m;
//CHECK-NEXT: #pragma omp end declare target
//CHECK-NEXT: #pragma omp groupprivate(ST<int>::m) device_type(nohost)
//CHECK-NEXT: };
template <class T> T foo() {
  static T v;
  #pragma omp groupprivate(v) device_type(nohost)
  v = ST<T>::m;
  return v;
}
#pragma omp end declare target
//CHECK: template <class T> T foo() {
//CHECK-NEXT: #pragma omp declare target
//CHECK-NEXT: static T v;
//CHECK-NEXT: #pragma omp groupprivate(v) device_type(nohost)
//CHECK-NEXT: v = ST<T>::m;
//CHECK-NEXT: return v;
//CHECK-NEXT: }
//CHECK-NEXT: #pragma omp end declare target
//CHECK: template<> int foo<int>() {
//CHECK-NEXT: #pragma omp declare target
//CHECK-NEXT: static int v;
//CHECK-NEXT: #pragma omp groupprivate(v) device_type(nohost)

namespace ns{
  int a;
}
// CHECK: namespace ns {
// CHECK-NEXT: int a;
// CHECK-NEXT: }
#pragma omp groupprivate(ns::a) device_type(nohost)
// CHECK-NEXT: #pragma omp groupprivate(ns::a) device_type(nohost)

int main () {
  static int a;
// CHECK: static int a;
#pragma omp groupprivate(a)
// CHECK-NEXT: #pragma omp groupprivate(a)
#pragma omp target
  a=2;
  return (foo<int>());
}

extern template int ST<int>::m;
#endif

// end INTEL_COLLAB
