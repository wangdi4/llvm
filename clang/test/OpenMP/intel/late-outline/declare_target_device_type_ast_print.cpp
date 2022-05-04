// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-late-outline -ast-print \
// RUN:   %s | FileCheck %s
//
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-late-outline -I %S/Inputs \
// RUN:   -emit-pch -o %t %s
//
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-late-outline -include-pch %t \
// RUN:   -fsyntax-only -I %S/Inputs -verify %s -ast-print | FileCheck %s

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

#pragma omp declare target device_type(nohost)
static const char *Str = "test";
const char *getStr() {
  return Str;
}
#pragma omp end declare target

// CHECK: #pragma omp declare target device_type(nohost)
// CHECK: static const char *Str = "test";
// CHECK: #pragma omp end declare target
// CHECK: #pragma omp declare target device_type(nohost)
// CHECK: const char *getStr() {
// CHECK:    return Str;
// CHECK: }

#pragma omp begin declare target device_type(nohost)
static const char *S = "test";
#pragma omp end declare target
#pragma omp declare target to(S)
const char *getS() {
  return S;
}

// CHECK: #pragma omp declare target device_type(nohost)
// CHECK: static const char *S = "test";
// CHECK: #pragma omp end declare target
// CHECK: const char *getS() {
// CHECK:     return S;
// CHECK: }

#pragma omp begin declare target device_type(nohost)
void zoo() {}
void x();
#pragma omp end declare target
#pragma omp declare target to(x)
// CHECK: #pragma omp declare target device_type(nohost)
// CHECK: void zoo() {
// CHECK: }
// CHECK: #pragma omp end declare target
// CHECK: #pragma omp declare target device_type(nohost)
// CHECK: void x();
// CHECK: #pragma omp end declare target

#endif
// end  INTEL_COLLAB
