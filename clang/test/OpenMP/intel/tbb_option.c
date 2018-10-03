// RUN: %clang_cc1 -emit-llvm -o - -DONE -fopenmp-tbb -Werror \
// RUN:  -Wsource-uses-openmp -fintel-compatibility -fintel-openmp-region \
// RUN:  -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s -check-prefix=CHECK-ONE

// RUN: %clang_cc1 -emit-llvm -o - -DTWO -fopenmp-tbb -Wsource-uses-openmp \
// RUN:  -fintel-compatibility -fintel-openmp-region \
// RUN:  -triple x86_64-unknown-linux-gnu -verify %s

// RUN: %clang_cc1 -emit-llvm -o - -DTHREE -fopenmp -fnoopenmp-tbb \
// RUN:  -Wsource-uses-openmp -fintel-compatibility -fintel-openmp-region \
// RUN:  -triple x86_64-unknown-linux-gnu -verify %s \
// RUN:  | FileCheck %s -check-prefix=CHECK-THREE

#ifdef ONE
#pragma omp declare simd notinbranch aligned(a : 32)
int baz(int v, float *a) { return 0; }

// Full OpenMP not enabled, only tbb subset
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
// CHECK-ONE: DIR.OMP.TASKLOOP
  #pragma omp taskloop
  for (i=0;i<10;++i) {
    arr[i] = i;
  }
}
// CHECK-ONE: attributes{{.*}}vva32_baz
#endif

#ifdef TWO
// Full OpenMP not enabled, only tbb subset
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
// Full OpenMP enabled, but tbb subset disabled
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
  #pragma omp taskloop // expected-warning {{unexpected '#pragma omp ...' in program}}
  for (i=0;i<10;++i) {
    arr[i] = i;
  }
}
#endif
