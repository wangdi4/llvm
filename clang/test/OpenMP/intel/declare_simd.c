// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fintel-openmp-region \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

//CHECK-DAG: _ZGVbM4vv_vec_sum
//CHECK-DAG: _ZGVbN4vv_vec_sum
//CHECK-DAG: _ZGVcM8vv_vec_sum
//CHECK-DAG: _ZGVcN8vv_vec_sum
//CHECK-DAG: _ZGVdM8vv_vec_sum
//CHECK-DAG: _ZGVdN8vv_vec_sum
//CHECK-DAG: _ZGVeM16vv_vec_sum
//CHECK-DAG: _ZGVeN16vv_vec_sum

#pragma omp declare simd
int vec_sum(int i, int j) {
  return i + j;
}

