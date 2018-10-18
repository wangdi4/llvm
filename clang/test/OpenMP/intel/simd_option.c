// RUN: %clang_cc1 -emit-llvm -o - -DONE -fopenmp-simd -Werror \
// RUN:  -Wsource-uses-openmp -fintel-compatibility -fintel-openmp-region \
// RUN:  -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s -check-prefix=CHECK-ONE

// RUN: %clang_cc1 -emit-llvm -o - -DTWO -fopenmp-simd -Wsource-uses-openmp \
// RUN:  -fintel-compatibility -fintel-openmp-region \
// RUN:  -triple x86_64-unknown-linux-gnu -verify %s

// RUN: %clang_cc1 -emit-llvm -o - -DTHREE -fopenmp -fno-openmp-simd \
// RUN:  -Wsource-uses-openmp -fintel-compatibility -fintel-openmp-region \
// RUN:  -triple x86_64-unknown-linux-gnu -verify %s \
// RUN:  | FileCheck %s -check-prefix=CHECK-THREE

#ifdef ONE
// Full OpenMP not enabled, only simd
// CHECK-ONE-LABEL: foo1
void foo1()
{
  int arr[10];
  int i;
// CHECK-ONE: DIR.OMP.SIMD
  #pragma omp simd
  for (i=0;i<10;++i) {
    arr[i] = i;
  }
}
#endif

#ifdef TWO
// Full OpenMP not enabled, only simd
// CHECK-TWO-LABEL: foo2
void foo2()
{
  int arr[10];
  int i;
  #pragma omp parallel for // expected-warning {{unexpected '#pragma omp ...' in program}}
  for (i=0;i<10;++i) {
    arr[i] = i;
  }
}
#endif

#ifdef THREE
// Full OpenMP enabled, but simd disabled
// CHECK-THREE-LABEL: foo3
void foo3()
{
  int arr[10];
  int i;
// CHECK-THREE: DIR.OMP.PARALLEL.LOOP
  #pragma omp parallel for
  for (i=0;i<10;++i) {
    arr[i] = i;
  }
  #pragma omp simd // expected-warning {{unexpected '#pragma omp ...' in program}}
  for (i=0;i<10;++i) {
    arr[i] = i;
  }
}
#endif
