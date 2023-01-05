// RUN: %clang_cc1 -x c++ -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -verify -triple x86_64-unknown-linux-gnu %s | FileCheck %s
// RUN: %clang_cc1 -x c -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -verify -triple x86_64-unknown-linux-gnu %s | FileCheck %s

#ifdef __cplusplus
struct A { float a; int b; double c; long d; };
#else
typedef struct A { float a; int b; double c; long d; } A;
#endif

#pragma omp declare simd simdlen(4)
int foo(A a); // expected-warning {{vectorization of a function with a struct or complex parameter type is not supported}}

#pragma omp declare simd simdlen(4)
int bar(int i, _Complex float a); // expected-warning {{vectorization of a function with a struct or complex parameter type is not supported}}

// CHECK: define {{.*}}func
// CHECK-NOT: vector-variants
void func(A* Aarr, _Complex float* Carr, int N, int *out) {
  #pragma omp simd simdlen(4)
  for (int i = 0; i < N; i++)
    out[i] = foo(Aarr[i]);
  #pragma omp simd simdlen(4)
  for (int i = 0; i < N; i++)
    out[i] = bar(i, Carr[i]);
}

