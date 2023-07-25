// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -fvecabi-cmdtarget -triple x86_64-unknown-linux-gnu %s | FileCheck %s

float *a;

//#pragma omp declare simd linear(i:1) inbranch
#pragma omp declare simd notinbranch ompx_processor(core_2nd_gen_avx) linear(b)
int set_a(int* b, float p)
{
  return *b + p;
}

#pragma omp declare simd notinbranch ompx_processor(core_2nd_gen_avx) linear(b)
float set_b(float* b, float p)
{
  return *b + p;
}

#pragma omp declare simd notinbranch ompx_processor(core_2nd_gen_avx) linear(b)
double set_c(float* b, float p)
{
  return *b + p;
}

// core_2nd_gen_avx, Integer width:16, Floating width:32

// CHECK-DAG: _ZGVyN4l4v_set_a
// CHECK-DAG: _ZGVyN8l4v_set_b
// CHECK-DAG: _ZGVyN4l4v_set_c
// CHECK-DAG: core_2nd_gen_avx
