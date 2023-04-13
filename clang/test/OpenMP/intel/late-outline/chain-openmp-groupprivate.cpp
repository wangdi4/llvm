// INTEL_COLLAB
// no PCH
// RUN: %clang_cc1 -fopenmp -ast-print -include %s -include %s %s -o - | FileCheck %s
// with PCH
// RUN: %clang_cc1 -fopenmp  -ast-print -chain-include %s -chain-include %s %s -o - | FileCheck %s
// no PCH

#if !defined(PASS1)
#define PASS1

extern "C" int* malloc (int size);
int *a;

#elif !defined(PASS2)
#define PASS2

#pragma omp groupprivate(a) device_type(nohost)
// CHECK: #pragma omp groupprivate(a) device_type(nohost)
#else


// CHECK-LABEL: foo
int foo() {
  return *a;
}

#endif
// end INTEL_COLLAB
