// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

//CHECK-NOT: _ZGVbM4vv_vec_sum
//CHECK-NOT: _ZGVbN4vv_vec_sum
//CHECK-NOT: _ZGVcM8vv_vec_sum
//CHECK-NOT: _ZGVcN8vv_vec_sum
//CHECK-NOT: _ZGVdM8vv_vec_sum
//CHECK-NOT: _ZGVdN8vv_vec_sum
//CHECK-NOT: _ZGVeM16vv_vec_sum
//CHECK-NOT: _ZGVeN16vv_vec_sum

#pragma omp declare simd
_Complex double foo(_Complex double x)

{ return x*x; }
