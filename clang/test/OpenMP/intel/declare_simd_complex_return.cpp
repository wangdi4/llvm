// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

//CHECK-NOT: _ZGVbN1v__Z3fooCd
//CHECK-NOT: _ZGVcN2v__Z3fooCd
//CHECK-NOT: _ZGVdN2v__Z3fooCd
//CHECK-NOT: _ZGVeN4v__Z3fooCd
//CHECK-NOT: _ZGVbM1v__Z3fooCd
//CHECK-NOT: _ZGVcM2v__Z3fooCd
//CHECK-NOT: _ZGVdM2v__Z3fooCd
//CHECK-NOT: _ZGVeM4v__Z3fooCd

#pragma omp declare simd
_Complex double foo(_Complex double x) // expected-warning {{Vectorization of a functions with a struct or complex return type is not supported}}

{ return x*x; }
